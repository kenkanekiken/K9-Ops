import 'package:flutter/material.dart';
import 'package:board/constants.dart';

class DesktopScaffold extends StatefulWidget {
  const DesktopScaffold({super.key});

  @override
  State<DesktopScaffold> createState() => _DesktopScaffoldState();
}

class _DesktopScaffoldState extends State<DesktopScaffold> {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: myDefaultBackground,
      body: SafeArea(
        child: SingleChildScrollView(
          padding: const EdgeInsets.all(18),
          child: Column(
            children: [
              const DashboardHeader(
                title: "Dog's Dashboard",
                subtitle: "Live monitoring & control",
              ),
              const SizedBox(height: 18),
              Row(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  // LEFT column
                  Expanded(
                    flex: 3,
                    child: Column(
                      children: const [
                        TopStatsRow(),
                        SizedBox(height: 18),

                        // Give GPS a fixed height so it doesn't fight layout
                        SizedBox(height: 420, child: GpsCard()),
                        SizedBox(height: 18),

                        LedControlCard(),
                      ],
                    ),
                  ),

                  const SizedBox(width: 18),

                  // RIGHT column
                  Expanded(
                    flex: 2,
                    child: Column(
                      children: const [
                        CommsPanel(),
                        SizedBox(height: 18),
                        VibrationPanel(),
                        SizedBox(height: 18),
                        RecentCommandsPanel(),
                      ],
                    ),
                  ),
                ],
              ),

              const SizedBox(height: 18),

              // ===== BOTTOM =====
              Row(
                children: const [
                  Expanded(
                    child: SizedBox(height: 600, child: MovementMonitorCard()),
                  ),
                  SizedBox(width: 18),
                  Expanded(
                    child: SizedBox(height: 600, child: FootageViewerCard()),
                  ),
                ],
              ),
            ],
          ),
        ),
      ),
    );
  }
}
