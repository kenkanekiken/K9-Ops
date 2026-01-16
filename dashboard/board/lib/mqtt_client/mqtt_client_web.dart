import 'package:mqtt_client/mqtt_client.dart';
import 'package:mqtt_client/mqtt_browser_client.dart';

MqttClient createPlatformClient({
  required String host,
  required int port, // unused on web (kept for signature)
  required String clientId,
  String? wsUrl, // ✅ provide full ws/wss url
}) {
  // IMPORTANT: web needs WebSocket URL.
  final url = wsUrl ?? 'wss://$host:$port/mqtt'; // change to your broker WS/WSS
  final c = MqttBrowserClient(url, clientId);
  c.port = port;
  return c;
}
