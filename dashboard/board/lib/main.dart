import 'package:flutter/material.dart';
// Firebase
import 'package:firebase_core/firebase_core.dart';
import 'firebase_options.dart';
// Responsiveness
import 'responsive/responsive_layout.dart';
import 'responsive/mobile_scaffold.dart';
import 'responsive/tablet_scaffold.dart';
import 'responsive/desktop_scaffold.dart';
import 'live_tracking_page.dart';
// MQTT
import 'package:provider/provider.dart';
import 'package:mqtt_client/mqtt_client.dart';
import 'package:mqtt_client/mqtt_server_client.dart';
import 'mqtt_client/mqtt_provider.dart';

Future<void> main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await Firebase.initializeApp(options: DefaultFirebaseOptions.currentPlatform);

  runApp(
    MultiProvider(
      providers: [
        ChangeNotifierProvider<MqttProvider>(
          create: (_) {
            final mqtt = MqttProvider(
              host: "test.mosquitto.org",
              port: 1883,
              username: "",
              password: "",
              cmdTopic: "k9ops/trainer/cmd",
              telemetryBase: "k9ops/dog",
              wsUrl: "wss://test.mosquitto.org:8081/mqtt",
            );

            // ✅ AUTO CONNECT ON APP START
            mqtt.connect();

            return mqtt;
          },
        ),
      ],
      child: const MyApp(),
    ),
  );
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'K9 Dashboard',
      debugShowCheckedModeBanner: false,
      home: ResponsiveLayout(
        mobileScaffold: const MobileScaffold(),
        tabletScaffold: const TabletScaffold(),
        desktopScaffold: const DesktopScaffold(),
      ),
    );
  }
}
