import 'package:flutter/material.dart';
import 'package:board/constants.dart';

class MobileScaffold extends StatefulWidget {
  const MobileScaffold({super.key});

  @override
  State<MobileScaffold> createState() => _MobileScaffoldState();
}

class _MobileScaffoldState extends State<MobileScaffold> {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: myDefaultBackground,
      body: SafeArea(
        child: SingleChildScrollView(
          padding: const EdgeInsets.all(14),
          child: Column(
            children: const [
              DashboardHeader(
                title: "Dog's Dashboard",
                subtitle: "Live monitoring & control",
              ),
              SizedBox(height: 14),
              TopStatsRow(),
              SizedBox(height: 14),

              // GPS
              SizedBox(height: 500, child: GpsCard()),
              SizedBox(height: 14),

              // LED
              LedControlCard(),
              SizedBox(height: 14),

              // Movement (stacked)
              SizedBox(height: 500, child: MovementMonitorCard()),
              SizedBox(height: 14),

              // Footage (slightly longer)
              SizedBox(height: 500, child: FootageViewerCard()),
              SizedBox(height: 14),

              // Comms + Vibration + Recent
              CommsPanel(),
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
