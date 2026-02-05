import 'package:flutter/material.dart';
import 'package:board/constants.dart';

class MobileScaffold extends StatefulWidget {
  const MobileScaffold({super.key});

  @override
  State<MobileScaffold> createState() => _MobileScaffoldState();
}

class _MobileScaffoldState extends State<MobileScaffold> {
  String currentMode = "Training"; // ✅ store mode here (parent)

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: myDefaultBackground,
      body: SafeArea(
        child: SingleChildScrollView(
          padding: const EdgeInsets.all(14),
          child: Column(
            children: [
              DashboardHeader(
                title: "Dog's Dashboard",
                subtitle: "Live monitoring & control",
              ),
              const SizedBox(height: 14),
              TopStatsRow(currentMode: currentMode),
              const SizedBox(height: 14),

              // GPS
              const SizedBox(height: 500, child: GpsCard()),
              const SizedBox(height: 14),

              // LED
              const LedControlCard(),
              const SizedBox(height: 14),

              // Movement (stacked)
              const SizedBox(height: 500, child: MovementMonitorCard()),
              const SizedBox(height: 14),

              // Footage (slightly longer)
              const SizedBox(height: 500, child: FootageViewerCard()),
              const SizedBox(height: 14),

              // Comms + Vibration + Recent
              // CommsPanel(),
              ModePanel(
                initialMode: currentMode,
                onModeChanged: (mode) {
                  setState(() => currentMode = mode); // ✅ update parent
                },
              ),
              SizedBox(height: 14),
              VibrationPanel(),
              SizedBox(height: 14),
              RecentCommandsPanel(),
              SizedBox(height: 14),
            ],
          ),
        ),
      ),
    );
  }
}
