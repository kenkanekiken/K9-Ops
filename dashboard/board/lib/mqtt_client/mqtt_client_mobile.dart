import 'package:mqtt_client/mqtt_client.dart';
import 'package:mqtt_client/mqtt_server_client.dart';

MqttClient createPlatformClient({
  required String host,
  required int port,
  required String clientId,
  String? wsUrl,
}) {
  final c = MqttServerClient(host, clientId);
  c.port = port; // 1883 / 8883
  return c;
}
