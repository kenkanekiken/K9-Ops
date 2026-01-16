import 'package:flutter/material.dart';
import 'package:board/constants.dart';

class TabletScaffold extends StatefulWidget {
  const TabletScaffold({super.key});

  @override
  State<TabletScaffold> createState() => _TabletScaffoldState();
}

class _TabletScaffoldState extends State<TabletScaffold> {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: myDefaultBackground,
      body: SafeArea(
        child: SingleChildScrollView(
          padding: const EdgeInsets.all(16),
          child: Column(
            children: [
              const DashboardHeader(
                title: "Dog's Dashboard",
                subtitle: "Live monitoring & control",
              ),
              const SizedBox(height: 16),
              const TopStatsRow(),
              const SizedBox(height: 16),

              // GPS (fixed height so it stays nice)
              const SizedBox(height: 500, child: GpsCard()),
              const SizedBox(height: 16),

              // LED Control
              const LedControlCard(),
              const SizedBox(height: 16),
              // Bottom: Movement + Footage (50/50)
              Row(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: const [
                  Expanded(
                    child: SizedBox(height: 500, child: MovementMonitorCard()),
                  ),
                  SizedBox(width: 16),
                  Expanded(
                    child: SizedBox(
                      height: 500, // 👈 footage slightly longer
                      child: FootageViewerCard(),
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 16),

              // Right-side panels become stacked on tablet
              const CommsPanel(),
              const SizedBox(height: 16),
              const VibrationPanel(),
              const SizedBox(height: 16),
              const RecentCommandsPanel(),
              const SizedBox(height: 16),
            ],
          ),
        ),
      ),
    );
  }
}
