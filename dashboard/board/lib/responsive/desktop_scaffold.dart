import 'package:flutter/material.dart';
import 'package:board/constants.dart';

class DesktopScaffold extends StatefulWidget {
  const DesktopScaffold({super.key});

  @override
  State<DesktopScaffold> createState() => _DesktopScaffoldState();
}

class _DesktopScaffoldState extends State<DesktopScaffold> {
  String currentMode = "Training"; // ✅ store mode here (parent)

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
                      children: [
                        TopStatsRow(currentMode: currentMode),
                        const SizedBox(height: 18),

                        // Give GPS a fixed height so it doesn't fight layout
                        const SizedBox(height: 420, child: GpsCard()),
                        const SizedBox(height: 18),

                        const LedControlCard(),
                      ],
                    ),
                  ),

                  const SizedBox(width: 18),

                  // RIGHT column
                  Expanded(
                    flex: 2,
                    child: Column(
                      children: [
                        // CommsPanel(),
                        ModePanel(
                          initialMode: currentMode,
                          onModeChanged: (mode) {
                            setState(
                              () => currentMode = mode,
                            ); // ✅ update parent
                          },
                        ),
                        const SizedBox(height: 18),
                        const VibrationPanel(),
                        const SizedBox(height: 18),
                        const RecentCommandsPanel(),
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
