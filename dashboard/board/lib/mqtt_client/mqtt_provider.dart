import 'dart:async';
import 'dart:convert';
import 'package:flutter/foundation.dart';
import 'package:mqtt_client/mqtt_client.dart';

import 'mqtt_client_factory.dart';

enum MqttConnState { disconnected, connecting, connected, error }

class MqttProvider extends ChangeNotifier {
  // ====== CONFIG ======
  final String host;
  final int port; // mobile TCP port (usually 1883)
  final String clientId;
  final String username;
  final String password;

  // ✅ WebSocket URL for Flutter Web (Chrome)
  // Example: "wss://test.mosquitto.org:8081/mqtt"
  final String? wsUrl;

  // Topics
  final String cmdTopic; // "k9ops/trainer/cmd"
  final String telemetryBase; // "k9ops/dog"

  MqttClient? _client;
  StreamSubscription<List<MqttReceivedMessage<MqttMessage>>>? _sub;

  MqttConnState state = MqttConnState.disconnected;
  String? lastError;

  final Map<String, dynamic> latestTelemetry = {};
  String? lastTopic;
  String? lastPayload;

  Timer? _reconnectTimer;
  bool _manualDisconnect = false;
  bool _connecting = false;

  MqttProvider({
    String? clientId,
    this.host = "test.mosquitto.org",
    this.port = 1883,
    this.username = "",
    this.password = "",
    this.wsUrl, // set this in main.dart for web if needed
    this.cmdTopic = "k9ops/trainer/cmd",
    this.telemetryBase = "k9ops/dog",
  }) : clientId =
           clientId ?? "k9ops_flutter_${DateTime.now().millisecondsSinceEpoch}";

  bool get isConnected =>
      _client?.connectionStatus?.state == MqttConnectionState.connected;

  Future<void> connect() async {
    if (_connecting) return;
    _connecting = true;

    _manualDisconnect = false;
    state = MqttConnState.connecting;
    lastError = null;
    notifyListeners();

    // cleanup any old client/sub
    try {
      await _sub?.cancel();
    } catch (_) {}
    _sub = null;

    try {
      _client?.disconnect();
    } catch (_) {}
    _client = null;

    final wsUrl = 'wss://$host:$port/mqtt'; // only used by web
    // ✅ Platform-specific client (web = browser ws, mobile = tcp)
    final c = createPlatformClient(
      host: host,
      port: port,
      clientId: clientId,
      // NOTE: web file supports wsUrl param; mobile ignores extra
      // ignore: undefined_named_parameter
      wsUrl: wsUrl,
    );

    c.onConnected = _onConnected;
    c.onDisconnected = _onDisconnected;
    c.onSubscribed = (t) => debugPrint("Subscribed: $t");

    final connMsg = MqttConnectMessage()
        .withClientIdentifier(clientId)
        .startClean()
        .withWillQos(MqttQos.atLeastOnce);

    if (username.isNotEmpty) {
      connMsg.authenticateAs(username, password);
    }

    c.connectionMessage = connMsg;
    _client = c;

    try {
      await c.connect();

      if (c.connectionStatus?.state != MqttConnectionState.connected) {
        throw Exception('MQTT not connected: ${c.connectionStatus}');
      }
    } catch (e) {
      state = MqttConnState.error;
      lastError = e.toString();
      notifyListeners();
      _connecting = false;
      _scheduleReconnect();
      return;
    }

    // Listen to incoming messages
    _sub = c.updates?.listen(_onMessage);

    // Subscribe
    subscribe('$telemetryBase/+/telemetry', qos: MqttQos.atLeastOnce);

    state = MqttConnState.connected;
    notifyListeners();
    _connecting = false;
  }

  Future<void> disconnect() async {
    _manualDisconnect = true;
    _reconnectTimer?.cancel();
    _reconnectTimer = null;

    try {
      await _sub?.cancel();
    } catch (_) {}
    _sub = null;

    try {
      _client?.disconnect();
    } catch (_) {}
    _client = null;

    state = MqttConnState.disconnected;
    notifyListeners();
  }

  void subscribe(String topic, {MqttQos qos = MqttQos.atLeastOnce}) {
    final c = _client;
    if (c == null || !isConnected) return;
    c.subscribe(topic, qos);
  }

  void sendCommand({
    required String target,
    required String cmd,
    dynamic value,
    String? requestId,
  }) {
    final c = _client;
    if (c == null || !isConnected) {
      debugPrint("MQTT NOT CONNECTED -> cannot publish");
      return;
    }

    final payloadMap = <String, dynamic>{
      "target": target,
      "cmd": cmd,
      "value": value,
      if (requestId != null) "rid": requestId,
      "ts": DateTime.now().millisecondsSinceEpoch,
    };

    final payload = jsonEncode(payloadMap);

    final builder = MqttClientPayloadBuilder()..addString(payload);

    c.publishMessage(cmdTopic, MqttQos.atLeastOnce, builder.payload!);

    lastTopic = cmdTopic;
    lastPayload = payload;
    notifyListeners();
  }

  void _onConnected() {
    _reconnectTimer?.cancel();
    _reconnectTimer = null;
    debugPrint("MQTT connected");
  }

  void _onDisconnected() {
    if (_manualDisconnect) return;
    state = MqttConnState.disconnected;
    notifyListeners();
    _scheduleReconnect();
  }

  void _scheduleReconnect() {
    if (_manualDisconnect) return;
    _reconnectTimer?.cancel();
    _reconnectTimer = Timer.periodic(const Duration(seconds: 3), (_) async {
      if (isConnected || _connecting) return;
      await connect();
    });
  }

  void _onMessage(List<MqttReceivedMessage<MqttMessage>> events) {
    for (final e in events) {
      final topic = e.topic;
      final msg = e.payload as MqttPublishMessage;
      final payload = MqttPublishPayload.bytesToStringAsString(
        msg.payload.message,
      );

      lastTopic = topic;
      lastPayload = payload;

      // telemetry: k9ops/dog/<dogId>/telemetry
      final parts = topic.split('/');
      if (parts.length >= 4 && parts[3] == 'telemetry') {
        final dogId = parts[2];

        dynamic decoded;
        try {
          decoded = jsonDecode(payload);
        } catch (_) {
          decoded = payload;
        }

        latestTelemetry[dogId] = decoded;
      }

      notifyListeners();
    }
  }

  @override
  void dispose() {
    _reconnectTimer?.cancel();
    _sub?.cancel();
    _client?.disconnect();
    super.dispose();
  }
}
