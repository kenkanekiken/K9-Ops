import 'package:flutter/material.dart';
import 'package:firebase_database/firebase_database.dart';
import 'dart:math' as math;

/* -------------------- COLORS -------------------- */
const bg = Color(0xFF0B1220);
const card = Color(0xFF121C2D);
const cardBorder = Color(0xFF22314A);
const softBg = Color(0xFF0E1727);

const accentBlue = Color(0xFF5BC0FF);
const accentGreen = Color(0xFF2FE57A);
const mutedText = Color(0xFF9FB0C8);

final Color myDefaultBackground = bg;

/* -------------------- TEXT STYLES -------------------- */
TextStyle labelStyle() => const TextStyle(
      color: mutedText,
      fontSize: 12,
      height: 1.2,
    );

TextStyle valueStyle() => const TextStyle(
      color: Colors.white,
      fontSize: 18,
      fontWeight: FontWeight.w600,
      height: 1.1,
    );

TextStyle titleStyle() => const TextStyle(
      color: Colors.white,
      fontSize: 14,
      fontWeight: FontWeight.w600,
    );

/* -------------------- DASHBOARD HEADER (LARGE) -------------------- */
class DashboardHeader extends StatelessWidget {
  final String title;
  final String subtitle;

  const DashboardHeader({
    super.key,
    required this.title,
    required this.subtitle,
  });

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 20, vertical: 18),
      decoration: BoxDecoration(
        color: card,
        borderRadius: BorderRadius.circular(20),
        border: Border.all(color: cardBorder),
        boxShadow: const [
          BoxShadow(
            blurRadius: 20,
            offset: Offset(0, 10),
            color: Color(0x33000000),
          )
        ],
      ),
      child: Row(
        children: [
          // Big icon block
          Container(
            height: 64,
            width: 64,
            decoration: BoxDecoration(
              color: softBg,
              borderRadius: BorderRadius.circular(18),
              border: Border.all(color: cardBorder),
            ),
            child: const Icon(
              Icons.pets_rounded,
              color: accentBlue,
              size: 34,
            ),
          ),
          const SizedBox(width: 18),

          // Title + subtitle
          Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Text(
                title,
                style: const TextStyle(
                  color: Colors.white,
                  fontSize: 28,          // 👈 BIG
                  fontWeight: FontWeight.w800,
                  letterSpacing: 0.4,
                ),
              ),
              const SizedBox(height: 6),
              Text(
                subtitle,
                style: const TextStyle(
                  color: mutedText,
                  fontSize: 14,
                  fontWeight: FontWeight.w500,
                ),
              ),
            ],
          ),
        ],
      ),
    );
  }
}

/* -------------------- BASE CARD -------------------- */
class GlassCard extends StatelessWidget {
  final Widget child;
  final EdgeInsets padding;
  const GlassCard({super.key, required this.child, this.padding = const EdgeInsets.all(16)});

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: padding,
      decoration: BoxDecoration(
        color: card,
        borderRadius: BorderRadius.circular(18),
        border: Border.all(color: cardBorder),
        boxShadow: const [
          BoxShadow(
            blurRadius: 18,
            offset: Offset(0, 8),
            color: Color(0x33000000),
          )
        ],
      ),
      child: child,
    );
  }
}

/* -------------------- TOP STATS -------------------- */
class StatPill extends StatelessWidget {
  final IconData icon;
  final String title;
  final String value;
  final Color iconColor;

  const StatPill({
    super.key,
    required this.icon,
    required this.title,
    required this.value,
    this.iconColor = accentBlue,
  });

  @override
  Widget build(BuildContext context) {
    return GlassCard(
      padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 14),
      child: Row(
        children: [
          Container(
            height: 40,
            width: 40,
            decoration: BoxDecoration(
              color: softBg,
              borderRadius: BorderRadius.circular(12),
              border: Border.all(color: cardBorder),
            ),
            child: Icon(icon, color: iconColor),
          ),
          const SizedBox(width: 12),
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(title, style: labelStyle()),
                const SizedBox(height: 4),
                Text(value, style: valueStyle()),
              ],
            ),
          ),
        ],
      ),
    );
  }
}

class TemperatureStatPill extends StatelessWidget {
  const TemperatureStatPill({super.key});

  @override
  Widget build(BuildContext context) {
    final ref =
        FirebaseDatabase.instance.ref('devices/teddy/temperature');

    return StreamBuilder<DatabaseEvent>(
      stream: ref.onValue,
      builder: (context, snapshot) {
        String display = "-- °C";

        if (snapshot.hasData) {
          final value = snapshot.data!.snapshot.value;
          if (value is num) {
            display = "${value.toStringAsFixed(1)}°C";
          } else if (value != null) {
            final parsed = double.tryParse(value.toString());
            if (parsed != null) {
              display = "${parsed.toStringAsFixed(1)}°C";
            }
          }
        }

        return StatPill(
          icon: Icons.thermostat_outlined,
          title: "Temperature",
          value: display,
        );
      },
    );
  }
}

class TopStatsRow extends StatelessWidget {
  const TopStatsRow({super.key});

  @override
  Widget build(BuildContext context) {
    return LayoutBuilder(
      builder: (context, c) {
        final isMobile = c.maxWidth < 700;

        final items = const [
          StatPill(icon: Icons.favorite_border, title: "Heat Risk", value: "Low"),
          // StatPill(icon: Icons.thermostat_outlined, title: "Temperature", value: "38.5°C"),
          TemperatureStatPill(), // 🔥 LIVE FROM FIREBASE
          StatPill(icon: Icons.show_chart, title: "Activity", value: "active"),
          StatPill(icon: Icons.battery_5_bar_outlined, title: "Battery", value: "87%"),
        ];

        if (isMobile) {
          // 2x2 grid on mobile
          return GridView.count(
            crossAxisCount: 2,
            mainAxisSpacing: 12,
            crossAxisSpacing: 12,
            shrinkWrap: true,
            physics: const NeverScrollableScrollPhysics(),
            childAspectRatio: 2.4, // tweak: higher = flatter cards
            children: items,
          );
        }

        // 1 row on tablet/desktop
        return Row(
          children: [
            Expanded(child: items[0]),
            const SizedBox(width: 12),
            Expanded(child: items[1]),
            const SizedBox(width: 12),
            Expanded(child: items[2]),
            const SizedBox(width: 12),
            Expanded(child: items[3]),
          ],
        );
      },
    );
  }
}

// Radar Pulse GPS
class BlueWavePulse extends StatefulWidget {
  final Widget child;        // your icon/avatar (stays still)
  final double minRadius;    // starting radius
  final double maxRadius;    // ending radius
  final Color color;
  final Duration duration;

  const BlueWavePulse({
    super.key,
    required this.child,
    this.minRadius = 34,
    this.maxRadius = 82,
    this.color = accentBlue,
    this.duration = const Duration(milliseconds: 1500),
  });

  @override
  State<BlueWavePulse> createState() => _BlueWavePulseState();
}

class _BlueWavePulseState extends State<BlueWavePulse>
    with SingleTickerProviderStateMixin {
  late final AnimationController _c;

  @override
  void initState() {
    super.initState();
    _c = AnimationController(vsync: this, duration: widget.duration)..repeat();
  }

  @override
  void dispose() {
    _c.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return AnimatedBuilder(
      animation: _c,
      builder: (_, __) {
        final t = _c.value; // 0..1
        final radius = widget.minRadius + (widget.maxRadius - widget.minRadius) * t;

        // Starts visible, fades out smoothly
        final opacity = (1.0 - t) * 0.18; // tune 0.18 to match your screenshot

        return Stack(
          alignment: Alignment.center,
          children: [
            // The blue "wave" (NOT affecting the icon)
            Container(
              width: radius * 2,
              height: radius * 2,
              decoration: BoxDecoration(
                shape: BoxShape.circle,
                color: widget.color.withOpacity(opacity),
              ),
            ),

            // Static icon/avatar on top
            widget.child,
          ],
        );
      },
    );
  }
}

/* -------------------- GPS CARD -------------------- */
class GpsCard extends StatelessWidget {
  const GpsCard({super.key});

  @override
  Widget build(BuildContext context) {
    return GlassCard(
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Row(
            children: [
              const Icon(Icons.location_on_outlined, color: accentBlue),
              const SizedBox(width: 8),
              Text("GPS Tracking", style: titleStyle()),
              const SizedBox(width: 8),
              const Text("📍"),
              const Spacer(),
              Row(
                children: const [
                  Icon(Icons.circle, size: 8, color: accentGreen),
                  SizedBox(width: 6),
                  Text("Live",
                      style: TextStyle(color: accentGreen, fontSize: 12, fontWeight: FontWeight.w600)),
                ],
              ),
            ],
          ),
          const SizedBox(height: 4),
          Text("Real-time location & conditions", style: labelStyle()),
          const SizedBox(height: 14),

          Expanded(
            child: Container(
              decoration: BoxDecoration(
                color: softBg,
                borderRadius: BorderRadius.circular(14),
                border: Border.all(color: cardBorder),
              ),
              child: Stack(
                children: [
                  Positioned.fill(
                    child: Opacity(opacity: 0.22, child: CustomPaint(painter: const GridPainter())),
                  ),
                  const Positioned(
                    left: 12,
                    top: 12,
                    child: _MiniInfoBox(title: "Coordinates", value: "40.758967,  -73.985005"),
                  ),
                  const Positioned(
                    right: 12,
                    top: 12,
                    child: _MiniInfoBox(title: "Accuracy", value: "±5m", valueColor: accentGreen),
                  ),
                  Center(
                    child: BlueWavePulse(
                      child: const CircleAvatar(
                        radius: 26,
                        backgroundColor: Color(0xFF1B4DFF),
                        child: Icon(Icons.navigation, color: Colors.white),
                      ),
                    ),
                  ),
                ],
              ),
            ),
          ),

          const SizedBox(height: 14),
          Row(
            children: [
              const Icon(Icons.info_outline, color: mutedText, size: 16),
              const SizedBox(width: 8),
              const Text("Current Conditions",
                  style: TextStyle(color: Colors.white, fontWeight: FontWeight.w600)),
              const SizedBox(width: 12),
              Expanded(
                child: Row(
                  mainAxisAlignment: MainAxisAlignment.spaceBetween,
                  children: const [
                    _TinyKV(k: "Weather", v: "Clear"),
                    _TinyKV(k: "Temperature", v: "18°C"),
                    _TinyKV(k: "Terrain", v: "Urban Park"),
                  ],
                ),
              ),
            ],
          ),
        ],
      ),
    );
  }
}

class _TinyKV extends StatelessWidget {
  final String k;
  final String v;
  const _TinyKV({required this.k, required this.v});

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(k, style: labelStyle()),
        const SizedBox(height: 2),
        Text(v, style: const TextStyle(color: Colors.white, fontWeight: FontWeight.w600)),
      ],
    );
  }
}

class _MiniInfoBox extends StatelessWidget {
  final String title;
  final String value;
  final Color valueColor;

  const _MiniInfoBox({
    required this.title,
    required this.value,
    this.valueColor = Colors.white,
  });

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 10),
      decoration: BoxDecoration(
        color: bg,
        borderRadius: BorderRadius.circular(12),
        border: Border.all(color: cardBorder),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(title, style: labelStyle()),
          const SizedBox(height: 2),
          Text(value, style: TextStyle(color: valueColor, fontWeight: FontWeight.w700)),
        ],
      ),
    );
  }
}

/* -------------------- LED CONTROL (CLICKABLE) -------------------- */
class LedControlCard extends StatefulWidget {
  const LedControlCard({super.key});

  @override
  State<LedControlCard> createState() => _LedControlCardState();
}

class _LedControlCardState extends State<LedControlCard> {
  int selectedMode = 0; // 0 Off, 1 Steady, 2 Flash, 3 Pulse
  int selectedColor = 0;
  double brightness = 0.75;

  @override
  Widget build(BuildContext context) {
    return GlassCard(
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Row(
            children: [
              const Icon(Icons.lightbulb_outline, color: Color(0xFFFFD24A)),
              const SizedBox(width: 8),
              Text("LED Control", style: titleStyle()),
              const SizedBox(width: 6),
              const Text("🔧"),
            ],
          ),
          const SizedBox(height: 4),
          Text("Visibility lighting settings", style: labelStyle()),
          const SizedBox(height: 16),

          Center(
            child: Container(
              height: 92,
              width: 92,
              decoration: BoxDecoration(
                color: softBg,
                shape: BoxShape.circle,
                border: Border.all(color: cardBorder),
              ),
              child: const Icon(Icons.flash_on, color: mutedText, size: 34),
            ),
          ),
          const SizedBox(height: 18),

          const Text("Mode", style: TextStyle(color: mutedText)),
          const SizedBox(height: 10),

          Row(
            children: [
              Expanded(
                child: _ModeButton(
                  label: "Off",
                  icon: Icons.circle,
                  selected: selectedMode == 0,
                  onTap: () => setState(() => selectedMode = 0),
                ),
              ),
              const SizedBox(width: 10),
              Expanded(
                child: _ModeButton(
                  label: "Steady",
                  icon: Icons.circle,
                  selected: selectedMode == 1,
                  onTap: () => setState(() => selectedMode = 1),
                ),
              ),
              const SizedBox(width: 10),
              Expanded(
                child: _ModeButton(
                  label: "Flash",
                  icon: Icons.flash_on,
                  selected: selectedMode == 2,
                  onTap: () => setState(() => selectedMode = 2),
                ),
              ),
              const SizedBox(width: 10),
              Expanded(
                child: _ModeButton(
                  label: "Pulse",
                  icon: Icons.waves,
                  selected: selectedMode == 3,
                  onTap: () => setState(() => selectedMode = 3),
                ),
              ),
            ],
          ),

          const SizedBox(height: 16),
          const Text("Color", style: TextStyle(color: mutedText)),
          const SizedBox(height: 10),

          Row(
            children: [
              _ColorDot(color: const Color(0xFF2DB7FF), selected: selectedColor == 0,
                  onTap: () => setState(() => selectedColor = 0)),
              const SizedBox(width: 10),
              _ColorDot(color: const Color(0xFF2FE57A), selected: selectedColor == 1,
                  onTap: () => setState(() => selectedColor = 1)),
              const SizedBox(width: 10),
              _ColorDot(color: const Color(0xFFFF2D6C), selected: selectedColor == 2,
                  onTap: () => setState(() => selectedColor = 2)),
              const SizedBox(width: 10),
              _ColorDot(color: const Color(0xFFFFA41B), selected: selectedColor == 3,
                  onTap: () => setState(() => selectedColor = 3)),
              const SizedBox(width: 10),
              _ColorDot(color: Colors.white, selected: selectedColor == 4,
                  onTap: () => setState(() => selectedColor = 4)),
            ],
          ),

          const SizedBox(height: 16),
          Text("Brightness: ${(brightness * 100).round()}%", style: const TextStyle(color: mutedText)),
          Slider(
            value: brightness,
            onChanged: (v) => setState(() => brightness = v),
          ),
        ],
      ),
    );
  }
}

class _ModeButton extends StatelessWidget {
  final String label;
  final IconData icon;
  final bool selected;
  final VoidCallback onTap;

  const _ModeButton({
    required this.label,
    required this.icon,
    required this.selected,
    required this.onTap,
  });

  @override
  Widget build(BuildContext context) {
    return Material(
      color: Colors.transparent,
      child: InkWell(
        borderRadius: BorderRadius.circular(14),
        onTap: onTap,
        child: Container(
          height: 54,
          decoration: BoxDecoration(
            color: selected ? const Color(0xFF1B4DFF) : softBg,
            borderRadius: BorderRadius.circular(14),
            border: Border.all(color: selected ? const Color(0xFF1B4DFF) : cardBorder),
          ),
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Icon(icon, color: selected ? Colors.white : mutedText, size: 18),
              const SizedBox(height: 4),
              Text(label, style: TextStyle(color: selected ? Colors.white : mutedText, fontSize: 12)),
            ],
          ),
        ),
      ),
    );
  }
}

class _ColorDot extends StatelessWidget {
  final Color color;
  final bool selected;
  final VoidCallback onTap;

  const _ColorDot({required this.color, required this.selected, required this.onTap});

  @override
  Widget build(BuildContext context) {
    return Material(
      color: Colors.transparent,
      child: InkWell(
        borderRadius: BorderRadius.circular(10),
        onTap: onTap,
        child: Container(
          height: 34,
          width: 34,
          decoration: BoxDecoration(
            color: color,
            borderRadius: BorderRadius.circular(10),
            border: Border.all(
              color: selected ? Colors.white : cardBorder,
              width: selected ? 2 : 1,
            ),
          ),
        ),
      ),
    );
  }
}

/* -------------------- RIGHT PANELS (CLICKABLE COMMANDS) -------------------- */

class CommsPanel extends StatelessWidget {
  const CommsPanel({super.key});

  @override
  Widget build(BuildContext context) {
    return GlassCard(
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Row(
            children: [
              const Icon(Icons.wifi_tethering, color: Color(0xFFB26BFF)),
              const SizedBox(width: 8),
              Text("Communication", style: titleStyle()),
              const SizedBox(width: 6),
              const Text("📡"),
            ],
          ),
          const SizedBox(height: 4),
          Text("Send commands & signals", style: labelStyle()),
          const SizedBox(height: 14),

          Row(
            children: const [
              Icon(Icons.volume_up_outlined, color: accentBlue),
              SizedBox(width: 8),
              Text("Sound Commands", style: TextStyle(color: Colors.white, fontWeight: FontWeight.w600)),
            ],
          ),
          const SizedBox(height: 12),

          GridView.count(
            crossAxisCount: 2,
            shrinkWrap: true,
            mainAxisSpacing: 10,
            crossAxisSpacing: 10,
            physics: const NeverScrollableScrollPhysics(),
            childAspectRatio: 2.2,
            children: const [
              _CommandTile(emoji: "👋", label: "Come"),
              _CommandTile(emoji: "✋", label: "Stay"),
              _CommandTile(emoji: "⬅️", label: "Left"),
              _CommandTile(emoji: "➡️", label: "Right"),
              _CommandTile(emoji: "🪑", label: "Sit"),
              _CommandTile(emoji: "⚠️", label: "Alert"),
            ],
          ),
        ],
      ),
    );
  }
}

class _CommandTile extends StatelessWidget {
  final String emoji;
  final String label;
  const _CommandTile({required this.emoji, required this.label});

  @override
  Widget build(BuildContext context) {
    return Material(
      color: Colors.transparent,
      child: InkWell(
        borderRadius: BorderRadius.circular(14),
        onTap: () => debugPrint("Command tapped: $label"),
        child: Container(
          decoration: BoxDecoration(
            color: softBg,
            borderRadius: BorderRadius.circular(14),
            border: Border.all(color: cardBorder),
          ),
          child: Center(
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Text(emoji, style: const TextStyle(fontSize: 18)),
                const SizedBox(height: 4),
                Text(label, style: const TextStyle(color: Colors.white)),
              ],
            ),
          ),
        ),
      ),
    );
  }
}

class VibrationPanel extends StatelessWidget {
  const VibrationPanel({super.key});

  @override
  Widget build(BuildContext context) {
    return GlassCard(
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: const [
          Row(
            children: [
              Icon(Icons.vibration, color: Color(0xFFB26BFF)),
              SizedBox(width: 8),
              Text("Vibration", style: TextStyle(color: Colors.white, fontWeight: FontWeight.w600)),
            ],
          ),
          SizedBox(height: 12),
          _VibeTile(index: "1", label: "Single Tap"),
          SizedBox(height: 10),
          _VibeTile(index: "2", label: "Double Tap"),
          SizedBox(height: 10),
          _VibeTile(index: "∞", label: "Continuous"),
        ],
      ),
    );
  }
}

class _VibeTile extends StatelessWidget {
  final String index;
  final String label;
  const _VibeTile({required this.index, required this.label});

  @override
  Widget build(BuildContext context) {
    return Material(
      color: Colors.transparent,
      child: InkWell(
        borderRadius: BorderRadius.circular(14),
        onTap: () => debugPrint("Vibration tapped: $label"),
        child: Container(
          height: 54,
          decoration: BoxDecoration(
            color: softBg,
            borderRadius: BorderRadius.circular(14),
            border: Border.all(color: cardBorder),
          ),
          child: Row(
            children: [
              const SizedBox(width: 12),
              Container(
                height: 28,
                width: 28,
                decoration: BoxDecoration(
                  color: const Color(0xFF18263D),
                  borderRadius: BorderRadius.circular(10),
                  border: Border.all(color: cardBorder),
                ),
                alignment: Alignment.center,
                child: Text(index, style: const TextStyle(color: Colors.white)),
              ),
              const SizedBox(width: 12),
              Text(label, style: const TextStyle(color: Colors.white)),
            ],
          ),
        ),
      ),
    );
  }
}

class RecentCommandsPanel extends StatelessWidget {
  const RecentCommandsPanel({super.key});

  @override
  Widget build(BuildContext context) {
    return GlassCard(
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Row(
            children: const [
              Icon(Icons.history, color: accentGreen),
              SizedBox(width: 8),
              Text("Recent Commands",
                  style: TextStyle(color: Colors.white, fontWeight: FontWeight.w600)),
            ],
          ),
          const SizedBox(height: 12),
          Container(
            height: 44,
            decoration: BoxDecoration(
              color: softBg,
              borderRadius: BorderRadius.circular(12),
              border: Border.all(color: cardBorder),
            ),
            alignment: Alignment.centerLeft,
            padding: const EdgeInsets.symmetric(horizontal: 12),
            child: Text("No commands sent yet",
                style: labelStyle().copyWith(fontStyle: FontStyle.italic)),
          ),
        ],
      ),
    );
  }
}
//FakeLine
class _FakeLineChartPainter extends CustomPainter {
  @override
  void paint(Canvas canvas, Size size) {
    _drawWave(canvas, size, const Color(0xFF2DB7FF), 1.0);
    _drawWave(canvas, size, const Color(0xFF2FE57A), 1.6);
    _drawWave(canvas, size, const Color(0xFFFFA41B), 1.3);
  }

  void _drawWave(Canvas canvas, Size size, Color color, double freq) {
    final paint = Paint()
      ..style = PaintingStyle.stroke
      ..strokeWidth = 2.2
      ..color = color.withOpacity(0.95);

    final path = Path();

    for (int i = 0; i <= 60; i++) {
      final t = i / 60.0;
      final x = t * size.width;

      // center line + wave
      final y = size.height * 0.55 +
          math.sin(t * 10 * freq) * size.height * 0.18;

      if (i == 0) {
        path.moveTo(x, y);
      } else {
        path.lineTo(x, y);
      }
    }

    canvas.drawPath(path, paint);
  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) => false;
}
/* -------------------- MOVEMENT MONITOR (LEFT) -------------------- */
class MovementMonitorCard extends StatelessWidget {
  const MovementMonitorCard({super.key});

  @override
  Widget build(BuildContext context) {
    return GlassCard(
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          // Header
          Row(
            children: [
              const Icon(Icons.monitor_heart_outlined, color: accentGreen),
              const SizedBox(width: 10),
              Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text("Movement Monitor", style: titleStyle()),
                  const SizedBox(height: 2),
                  Text("Accelerometer data & activity", style: labelStyle()),
                ],
              ),
            ],
          ),
          const SizedBox(height: 16),

          // Mini stats row
          Row(
            children: const [
              Expanded(child: _MiniStat(title: "Activity", value: "Runnin")),
              SizedBox(width: 12),
              Expanded(child: _MiniStat(title: "Intensity", value: "High", valueColor: accentGreen)),
              SizedBox(width: 12),
              Expanded(child: _MiniStat(title: "Steps", value: "8,432")),
              SizedBox(width: 12),
              Expanded(child: _MiniStat(title: "Distance", value: "6.2 km")),
            ],
          ),
          const SizedBox(height: 16),

          // Chart panel (placeholder)
          Expanded(
            child: Container(
              decoration: BoxDecoration(
                color: softBg,
                borderRadius: BorderRadius.circular(16),
                border: Border.all(color: cardBorder),
              ),
              child: Padding(
                padding: const EdgeInsets.all(14),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Row(
                      children: const [
                        Icon(Icons.show_chart, color: accentBlue, size: 18),
                        SizedBox(width: 8),
                        Text(
                          "Accelerometer (3-axis)",
                          style: TextStyle(color: Colors.white, fontWeight: FontWeight.w600),
                        ),
                      ],
                    ),
                    const SizedBox(height: 12),

                    // Grid + fake lines
                    Expanded(
                      child: ClipRRect(
                        borderRadius: BorderRadius.circular(12),
                        child: Stack(
                          children: [
                            Positioned.fill(
                              child: Opacity(
                                opacity: 0.22,
                                child: CustomPaint(painter: const GridPainter(step: 32)),
                              ),
                            ),
                            Positioned.fill(
                              child: Padding(
                                padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 8),
                                child: CustomPaint(painter: _FakeLineChartPainter()),
                              ),
                            ),
                          ],
                        ),
                      ),
                    ),

                    const SizedBox(height: 10),
                    // Legend
                    Row(
                      mainAxisAlignment: MainAxisAlignment.center,
                      children: const [
                        _LegendDot(color: Color(0xFF2DB7FF), label: "X-axis"),
                        SizedBox(width: 14),
                        _LegendDot(color: Color(0xFF2FE57A), label: "Y-axis"),
                        SizedBox(width: 14),
                        _LegendDot(color: Color(0xFFFFA41B), label: "Z-axis"),
                      ],
                    ),
                  ],
                ),
              ),
            ),
          ),
        ],
      ),
    );
  }
}

class _MiniStat extends StatelessWidget {
  final String title;
  final String value;
  final Color valueColor;

  const _MiniStat({
    required this.title,
    required this.value,
    this.valueColor = Colors.white,
  });

  @override
  Widget build(BuildContext context) {
    return Container(
      height: 68,
      padding: const EdgeInsets.symmetric(horizontal: 14, vertical: 12),
      decoration: BoxDecoration(
        color: softBg,
        borderRadius: BorderRadius.circular(14),
        border: Border.all(color: cardBorder),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(title, style: labelStyle()),
          const SizedBox(height: 4),
          Text(
            value,
            style: TextStyle(color: valueColor, fontWeight: FontWeight.w700, fontSize: 14),
          ),
        ],
      ),
    );
  }
}

class _LegendDot extends StatelessWidget {
  final Color color;
  final String label;
  const _LegendDot({required this.color, required this.label});

  @override
  Widget build(BuildContext context) {
    return Row(
      children: [
        Container(
          width: 8,
          height: 8,
          decoration: BoxDecoration(color: color, shape: BoxShape.circle),
        ),
        const SizedBox(width: 6),
        Text(label, style: const TextStyle(color: mutedText, fontSize: 12)),
      ],
    );
  }
}

/* -------------------- FOOTAGE VIEWER (RIGHT) -------------------- */
class FootageViewerCard extends StatelessWidget {
  const FootageViewerCard({super.key});

  @override
  Widget build(BuildContext context) {
    return GlassCard(
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          // Header
          Row(
            children: [
              const Icon(Icons.photo_camera_outlined, color: Color(0xFFFF5A5A)),
              const SizedBox(width: 10),
              Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text("Footage Viewer", style: titleStyle()),
                  const SizedBox(height: 2),
                  Text("Captured video evidence", style: labelStyle()),
                ],
              ),
            ],
          ),
          const SizedBox(height: 16),

          // Main video preview (placeholder)
          Expanded(
            child: Container(
              decoration: BoxDecoration(
                color: softBg,
                borderRadius: BorderRadius.circular(16),
                border: Border.all(color: cardBorder),
              ),
              child: Stack(
                children: [
                  // fake thumbnail gradient
                  Positioned.fill(
                    child: DecoratedBox(
                      decoration: BoxDecoration(
                        borderRadius: BorderRadius.circular(16),
                        gradient: const LinearGradient(
                          begin: Alignment.topCenter,
                          end: Alignment.bottomCenter,
                          colors: [Color(0xFF2A3448), Color(0xFF0E1727)],
                        ),
                      ),
                    ),
                  ),

                  // play button
                  Center(
                    child: Container(
                      width: 64,
                      height: 64,
                      decoration: BoxDecoration(
                        shape: BoxShape.circle,
                        color: Colors.black.withOpacity(0.25),
                        border: Border.all(color: Colors.white.withOpacity(0.30)),
                      ),
                      child: const Icon(Icons.play_arrow_rounded, color: Colors.white, size: 34),
                    ),
                  ),

                  // bottom left text
                  Positioned(
                    left: 14,
                    bottom: 14,
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: const [
                        Text("Motion Detected",
                            style: TextStyle(color: Colors.white, fontWeight: FontWeight.w700)),
                        SizedBox(height: 3),
                        Text("2025-12-11 14:32", style: TextStyle(color: mutedText, fontSize: 12)),
                      ],
                    ),
                  ),

                  // duration + download (bottom right)
                  Positioned(
                    right: 14,
                    bottom: 14,
                    child: Row(
                      children: [
                        const Text("2:15", style: TextStyle(color: Colors.white, fontSize: 12)),
                        const SizedBox(width: 10),
                        Container(
                          width: 34,
                          height: 34,
                          decoration: BoxDecoration(
                            color: Colors.black.withOpacity(0.25),
                            borderRadius: BorderRadius.circular(10),
                            border: Border.all(color: Colors.white.withOpacity(0.20)),
                          ),
                          child: const Icon(Icons.download_rounded, color: Colors.white, size: 18),
                        ),
                      ],
                    ),
                  ),
                ],
              ),
            ),
          ),

          const SizedBox(height: 14),

          // Recent captures row
          Row(
            children: const [
              Icon(Icons.calendar_month_outlined, color: accentBlue, size: 18),
              SizedBox(width: 8),
              Text("Recent Captures",
                  style: TextStyle(color: Colors.white, fontWeight: FontWeight.w600)),
            ],
          ),
          const SizedBox(height: 10),

          SizedBox(
            height: 74,
            child: ListView(
              scrollDirection: Axis.horizontal,
              children: const [
                _ThumbTile(selected: true, duration: "2:15"),
                SizedBox(width: 10),
                _ThumbTile(duration: "1:45"),
                SizedBox(width: 10),
                _ThumbTile(duration: "3:20"),
                SizedBox(width: 10),
                _ThumbTile(duration: "1:30"),
              ],
            ),
          ),
        ],
      ),
    );
  }
}

class _ThumbTile extends StatelessWidget {
  final bool selected;
  final String duration;

  const _ThumbTile({this.selected = false, required this.duration});

  @override
  Widget build(BuildContext context) {
    return Material(
      color: Colors.transparent,
      child: InkWell(
        borderRadius: BorderRadius.circular(14),
        onTap: () => debugPrint("Open clip $duration"),
        child: Container(
          width: 120,
          decoration: BoxDecoration(
            color: softBg,
            borderRadius: BorderRadius.circular(14),
            border: Border.all(
              color: selected ? const Color(0xFF1B4DFF) : cardBorder,
              width: selected ? 2 : 1,
            ),
          ),
          child: Stack(
            children: [
              // thumbnail placeholder
              Positioned.fill(
                child: ClipRRect(
                  borderRadius: BorderRadius.circular(12),
                  child: DecoratedBox(
                    decoration: BoxDecoration(
                      gradient: LinearGradient(
                        begin: Alignment.topLeft,
                        end: Alignment.bottomRight,
                        colors: [
                          const Color(0xFF2A3448),
                          Colors.black.withOpacity(0.35),
                        ],
                      ),
                    ),
                  ),
                ),
              ),
              // duration badge
              Positioned(
                right: 8,
                bottom: 8,
                child: Container(
                  padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 4),
                  decoration: BoxDecoration(
                    color: Colors.black.withOpacity(0.45),
                    borderRadius: BorderRadius.circular(10),
                    border: Border.all(color: Colors.white.withOpacity(0.10)),
                  ),
                  child: Text(duration, style: const TextStyle(color: Colors.white, fontSize: 11)),
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}

/* -------------------- Grid Painters -------------------- */
class GridPainter extends CustomPainter {
  final double step;          // spacing between lines
  final double opacity;       // line opacity (0..1)
  final double strokeWidth;   // line thickness

  const GridPainter({
    this.step = 28,
    this.opacity = 0.10,
    this.strokeWidth = 1,
  });

  @override
  void paint(Canvas canvas, Size size) {
    final paint = Paint()
      ..color = Colors.white.withOpacity(opacity)
      ..strokeWidth = strokeWidth;

    // vertical lines
    for (double x = 0; x <= size.width; x += step) {
      canvas.drawLine(Offset(x, 0), Offset(x, size.height), paint);
    }

    // horizontal lines
    for (double y = 0; y <= size.height; y += step) {
      canvas.drawLine(Offset(0, y), Offset(size.width, y), paint);
    }
  }

  @override
  bool shouldRepaint(covariant GridPainter oldDelegate) {
    return oldDelegate.step != step ||
        oldDelegate.opacity != opacity ||
        oldDelegate.strokeWidth != strokeWidth;
  }
}