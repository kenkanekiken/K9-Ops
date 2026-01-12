import 'package:flutter/material.dart';
import 'package:firebase_database/firebase_database.dart';
import 'dart:math' as math;
import 'package:google_maps_flutter/google_maps_flutter.dart';
import 'dart:typed_data';
import 'dart:ui' as ui;
import 'package:flutter/services.dart';
import 'package:flutter/foundation.dart';
import 'dart:async';
import 'package:flutter/gestures.dart';

final isWeb = kIsWeb;

/* -------------------- Google API -------------------- */
const String google_api_key = "API_KEY";

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
TextStyle labelStyle() =>
    const TextStyle(color: mutedText, fontSize: 12, height: 1.2);

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
          ),
        ],
      ),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
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
            child: const Icon(Icons.pets_rounded, color: accentBlue, size: 34),
          ),
          const SizedBox(width: 18),

          // Title + subtitle (left) + status (right)
          Expanded(
            child: Row(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                // Left text
                Expanded(
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text(
                        title,
                        style: const TextStyle(
                          color: Colors.white,
                          fontSize: 28,
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
                ),

                const SizedBox(width: 12),

                // Right status (top-right)
                const Padding(
                  padding: EdgeInsets.only(top: 4),
                  child: PowerStatus(), // reads devices/latest/power
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }
}

/* -------------------- GPS Status -------------------- */
class PowerStatus extends StatelessWidget {
  const PowerStatus({super.key});

  @override
  Widget build(BuildContext context) {
    final ref = FirebaseDatabase.instance.ref('devices/latest/power');

    return StreamBuilder<DatabaseEvent>(
      stream: ref.onValue,
      builder: (context, snapshot) {
        bool isLive = false;

        if (snapshot.hasData) {
          final value = snapshot.data!.snapshot.value;

          if (value is bool) {
            isLive = value;
          } else if (value != null) {
            final s = value.toString().toLowerCase();
            isLive = (s == "true" || s == "live" || s == "online");
          }
        }

        final Color c = isLive ? accentGreen : Colors.red;

        return Row(
          children: [
            Icon(Icons.circle, size: 8, color: c),
            const SizedBox(width: 6),
            Text(
              isLive ? "Online" : "Offline",
              style: TextStyle(
                color: c,
                fontSize: 12,
                fontWeight: FontWeight.w600,
              ),
            ),
          ],
        );
      },
    );
  }
}

/* -------------------- BASE CARD -------------------- */
class GlassCard extends StatelessWidget {
  final Widget child;
  final EdgeInsets padding;
  const GlassCard({
    super.key,
    required this.child,
    this.padding = const EdgeInsets.all(16),
  });

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
          ),
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
    final ref = FirebaseDatabase.instance.ref('devices/latest/temperature');

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

class BatteryStatPill extends StatelessWidget {
  const BatteryStatPill({super.key});

  @override
  Widget build(BuildContext context) {
    final ref = FirebaseDatabase.instance.ref('devices/latest/battery');

    return StreamBuilder<DatabaseEvent>(
      stream: ref.onValue,
      builder: (context, snapshot) {
        String display = "-- %";

        if (snapshot.hasData) {
          final value = snapshot.data!.snapshot.value;
          if (value is num) {
            display = "${value.toInt()}%";
          }
        }

        return StatPill(
          icon: Icons.battery_5_bar_outlined,
          title: "Battery",
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
          StatPill(
            icon: Icons.favorite_border,
            title: "Heat Risk",
            value: "Low",
          ),
          TemperatureStatPill(),
          StatPill(icon: Icons.show_chart, title: "Activity", value: "active"),
          BatteryStatPill(),
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

/* -------------------- GPS Status (signal) -------------------- */
class GpsStatus extends StatelessWidget {
  const GpsStatus({super.key});

  @override
  Widget build(BuildContext context) {
    final ref = FirebaseDatabase.instance.ref('devices/latest/gpsStatus');

    return StreamBuilder<DatabaseEvent>(
      stream: ref.onValue,
      builder: (context, snapshot) {
        bool isOnline = false;

        if (snapshot.hasData) {
          final value = snapshot.data!.snapshot.value;

          if (value is bool) {
            isOnline = value;
          } else if (value is num) {
            isOnline = value != 0; // in case you store 1/0
          } else if (value != null) {
            final s = value.toString().toLowerCase();
            isOnline =
                (s == "true" || s == "online" || s == "live" || s == "1");
          }
        }

        final Color c = isOnline ? accentGreen : Colors.red;

        return Row(
          children: [
            Icon(Icons.circle, size: 8, color: c),
            const SizedBox(width: 6),
            Text(
              isOnline ? "Connected" : "No Signal",
              style: TextStyle(
                color: c,
                fontSize: 12,
                fontWeight: FontWeight.w600,
              ),
            ),
          ],
        );
      },
    );
  }
}

/* -------------------- RADAR PULSE GPS -------------------- */
class BlueWavePulse extends StatefulWidget {
  final Widget child; // icon/avatar
  final double minRadius; // starting radius
  final double maxRadius; // ending radius
  final Color color;
  final Duration duration;

  const BlueWavePulse({
    super.key,
    required this.child,
    this.minRadius = 34,
    this.maxRadius = 50,
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
        final radius =
            widget.minRadius + (widget.maxRadius - widget.minRadius) * t;

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

class GpsCard extends StatefulWidget {
  const GpsCard({super.key});

  @override
  State<GpsCard> createState() => _GpsCardState();
}

class _GpsCardState extends State<GpsCard> {
  // Latest position used by the map/marker (smoothed)
  LatLng? _pos;

  // Raw position (no smoothing) for computing bearing
  LatLng? _rawPos;

  // Smoothed state
  LatLng? _smoothPos;

  // Optional telemetry (if you upload these fields)
  double? _hdop;
  int? _sats;
  double? _speedMps;
  double _headingDeg = 0.0;

  // Marker icon
  BitmapDescriptor? navIcon;

  // Map controller + pulse pixel position
  GoogleMapController? _mapCtrl;
  Offset? _pulsePx;

  // Pulse settings (match BlueWavePulse maxRadius)
  static const double _pulseMaxRadius = 50;

  // ✅ No fixed pixel nudge; keep pulse truly locked to marker
  static const Offset _pulseCenterNudge = Offset.zero;

  // Firebase
  late final DatabaseReference _gpsRef;
  StreamSubscription<DatabaseEvent>? _gpsSub;

  // Behavior: recenter until user touches map
  bool _userInteracted = false;
  bool _didCenterOnce = false;

  // ----------------- marker bitmap resize -----------------
  Future<Uint8List> _loadMarkerBytes(String assetPath, int targetWidth) async {
    final data = await rootBundle.load(assetPath);
    final codec = await ui.instantiateImageCodec(
      data.buffer.asUint8List(),
      targetWidth: targetWidth,
    );
    final frame = await codec.getNextFrame();
    final byteData = await frame.image.toByteData(
      format: ui.ImageByteFormat.png,
    );
    return byteData!.buffer.asUint8List();
  }

  int _markerSize() => kIsWeb ? 30 : 80;

  num? _toNum(dynamic v) {
    if (v is num) return v;
    if (v is String) return num.tryParse(v);
    return null;
  }

  // ----------------- geo helpers -----------------
  double _haversineMeters(LatLng a, LatLng b) {
    const r = 6371000.0; // Earth radius (m)
    final dLat = (b.latitude - a.latitude) * (math.pi / 180.0);
    final dLng = (b.longitude - a.longitude) * (math.pi / 180.0);
    final lat1 = a.latitude * (math.pi / 180.0);
    final lat2 = b.latitude * (math.pi / 180.0);

    final h =
        math.sin(dLat / 2) * math.sin(dLat / 2) +
        math.cos(lat1) *
            math.cos(lat2) *
            math.sin(dLng / 2) *
            math.sin(dLng / 2);

    final c = 2 * math.atan2(math.sqrt(h), math.sqrt(1 - h));
    return r * c;
  }

  LatLng _smooth(LatLng prev, LatLng next, double alpha) {
    return LatLng(
      prev.latitude + (next.latitude - prev.latitude) * alpha,
      prev.longitude + (next.longitude - prev.longitude) * alpha,
    );
  }

  double _bearingDeg(LatLng a, LatLng b) {
    final lat1 = a.latitude * (math.pi / 180.0);
    final lat2 = b.latitude * (math.pi / 180.0);
    final dLng = (b.longitude - a.longitude) * (math.pi / 180.0);

    final y = math.sin(dLng) * math.cos(lat2);
    final x =
        math.cos(lat1) * math.sin(lat2) -
        math.sin(lat1) * math.cos(lat2) * math.cos(dLng);

    final brng = math.atan2(y, x) * (180.0 / math.pi);
    return (brng + 360.0) % 360.0;
  }

  // LatLng -> pixel position inside the map widget
  Future<void> _updatePulsePosition(LatLng pos) async {
    final c = _mapCtrl;
    if (c == null) return;

    try {
      final sc = await c.getScreenCoordinate(pos);

      // Default conversion (works for web + android)
      final dpr = MediaQuery.of(context).devicePixelRatio;
      double dx = sc.x.toDouble() / dpr;
      double dy = sc.y.toDouble() / dpr;

      // iOS sometimes reports in a different unit
      if (!kIsWeb && defaultTargetPlatform == TargetPlatform.iOS) {
        dx = sc.x.toDouble();
        dy = sc.y.toDouble();
      }

      if (!mounted) return;
      setState(() => _pulsePx = Offset(dx, dy));
    } catch (_) {
      // ignore
    }
  }

  @override
  void initState() {
    super.initState();

    // Load custom nav icon
    _loadMarkerBytes('assets/navigationIcon.png', _markerSize())
        .then((bytes) {
          if (!mounted) return;
          setState(() => navIcon = BitmapDescriptor.fromBytes(bytes));
        })
        .catchError((e) => debugPrint("Marker icon load/resize failed: $e"));

    // Listen to GPS from Firebase
    _gpsRef = FirebaseDatabase.instance.ref('devices/latest');

    _gpsSub = _gpsRef.onValue.listen((event) async {
      final data = event.snapshot.value;
      if (data is! Map) return;

      final lat = _toNum(data['latitude']);
      final lon = _toNum(data['longitude']);
      if (lat == null || lon == null) return;

      final newRaw = LatLng(lat.toDouble(), lon.toDouble());

      // Optional fields (only work if you upload them)
      _hdop = _toNum(data['hdop'])?.toDouble();
      _sats = _toNum(data['sats'])?.toInt();
      final course = _toNum(data['course'])?.toDouble(); // 0..360

      // ---- Quality gating (tune these) ----
      // If you DON'T upload hdop/sats, these are null and won't block.
      if (_sats != null && _sats! < 6) return;
      if (_hdop != null && _hdop! > 2.8) return;

      final prevRaw = _rawPos;
      _rawPos = newRaw;

      // ---- Ignore tiny jitter when standing still ----
      // If we've already got a smoothed position, drop micro-moves.
      if (_smoothPos != null) {
        final dist = _haversineMeters(_smoothPos!, newRaw);

        // Drop < 1.5m "wobble" (tune: 1.0 to 3.0)
        if (dist < 1.5) return;
      }

      // ---- Adaptive smoothing (slower => more smoothing) ----
      LatLng nextPos = newRaw;
      if (_smoothPos != null) {
        final speed = _speedMps ?? 0.0;

        // More smoothing at low speed; less smoothing when moving faster
        final alpha = (speed < 0.8) ? 0.15 : 0.35; // tune: 0.1..0.5
        nextPos = _smooth(_smoothPos!, newRaw, alpha);
      }

      // ---- Heading ----
      if (course != null) {
        _headingDeg = course;
      } else if (prevRaw != null) {
        _headingDeg = _bearingDeg(prevRaw, newRaw);
      }

      if (!mounted) return;
      setState(() {
        _smoothPos = nextPos;
        _pos = nextPos;
      });

      // Keep pulse synced (if map is ready)
      await _updatePulsePosition(nextPos);

      // Center camera behavior
      if (_mapCtrl != null && !_userInteracted) {
        if (!_didCenterOnce) {
          _didCenterOnce = true;
          await _mapCtrl!.animateCamera(CameraUpdate.newLatLng(nextPos));
        }
        // Optional: uncomment to "follow" continuously when user hasn't touched map
        // await _mapCtrl!.animateCamera(CameraUpdate.newLatLng(nextPos));
      }
    });
  }

  @override
  void dispose() {
    _gpsSub?.cancel();
    _mapCtrl?.dispose();
    super.dispose();
  }

  String _accuracyText() {
    // If you upload hdop/sats, show something meaningful; otherwise keep your default.
    if (_hdop != null) {
      // very rough rule of thumb; depends on receiver/environment
      final approxMeters = (_hdop! * 5.0).clamp(3.0, 50.0);
      return "±${approxMeters.toStringAsFixed(0)}m";
    }
    return "±5m";
  }

  @override
  Widget build(BuildContext context) {
    final pos = _pos;

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
              const GpsStatus(),
            ],
          ),
          const SizedBox(height: 4),
          Text("Real-time location & conditions", style: labelStyle()),
          const SizedBox(height: 14),

          Expanded(
            child: ClipRRect(
              borderRadius: BorderRadius.circular(14),
              child: Stack(
                children: [
                  if (pos == null)
                    Container(
                      color: softBg,
                      alignment: Alignment.center,
                      child: const Text(
                        "Waiting for GPS…",
                        style: TextStyle(color: Colors.white70),
                      ),
                    )
                  else
                    Positioned.fill(
                      child: GoogleMap(
                        initialCameraPosition: CameraPosition(
                          target: pos,
                          zoom: 16,
                        ),
                        onMapCreated: (c) {
                          _mapCtrl = c;
                          WidgetsBinding.instance.addPostFrameCallback((_) {
                            _updatePulsePosition(pos);
                          });
                        },
                        onCameraMoveStarted: () {
                          _userInteracted = true;
                        },
                        onCameraIdle: () {
                          if (_pos != null) _updatePulsePosition(_pos!);
                        },
                        markers: {
                          Marker(
                            markerId: const MarkerId("source"),
                            position: pos,
                            icon:
                                navIcon ??
                                BitmapDescriptor.defaultMarkerWithHue(
                                  BitmapDescriptor.hueAzure,
                                ),
                            anchor: const Offset(0.5, 0.5),
                            flat: true,
                            rotation: _headingDeg, // ✅ rotate by heading/course
                          ),
                        },
                        zoomControlsEnabled: true,
                        compassEnabled: true,
                        mapToolbarEnabled: true,
                        scrollGesturesEnabled: true,
                        zoomGesturesEnabled: true,
                        rotateGesturesEnabled: true,
                        tiltGesturesEnabled: false,
                        gestureRecognizers: {
                          Factory<OneSequenceGestureRecognizer>(
                            () => EagerGestureRecognizer(),
                          ),
                        },
                      ),
                    ),

                  // overlays
                  if (pos != null) ...[
                    Positioned(
                      left: 12,
                      top: 12,
                      child: LiveCoordinatesBox(pos: pos),
                    ),
                    Positioned(
                      right: 12,
                      top: 12,
                      child: _MiniInfoBox(
                        title: "Accuracy",
                        value: _accuracyText(),
                        valueColor: accentGreen,
                      ),
                    ),

                    // pulse (locked to marker)
                    if (_pulsePx != null)
                      Positioned(
                        left:
                            (_pulsePx!.dx + _pulseCenterNudge.dx) -
                            _pulseMaxRadius,
                        top:
                            (_pulsePx!.dy + _pulseCenterNudge.dy) -
                            _pulseMaxRadius,
                        child: IgnorePointer(
                          child: BlueWavePulse(
                            child: const SizedBox.shrink(),
                            maxRadius: _pulseMaxRadius,
                          ),
                        ),
                      ),
                  ],
                ],
              ),
            ),
          ),

          const SizedBox(height: 14),
          Row(
            children: [
              const Icon(Icons.info_outline, color: mutedText, size: 16),
              const SizedBox(width: 8),
              const Text(
                "Current Conditions",
                style: TextStyle(
                  color: Colors.white,
                  fontWeight: FontWeight.w600,
                ),
              ),
              const SizedBox(width: 12),
              Expanded(
                child: Row(
                  mainAxisAlignment: MainAxisAlignment.spaceBetween,
                  children: const [
                    _TinyKV(k: "Weather", v: "Clear"),
                    GPSTemperature(),
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

class LiveCoordinatesBox extends StatelessWidget {
  final LatLng pos;
  const LiveCoordinatesBox({super.key, required this.pos});

  @override
  Widget build(BuildContext context) {
    final coordText =
        "${pos.latitude.toStringAsFixed(6)}, ${pos.longitude.toStringAsFixed(6)}";
    return _MiniInfoBox(title: "Coordinates", value: coordText);
  }
}

/* -------------------- Temperature Pill -------------------- */
class GPSTemperature extends StatelessWidget {
  const GPSTemperature({super.key});

  @override
  Widget build(BuildContext context) {
    final ref = FirebaseDatabase.instance.ref('devices/latest/temperature');

    return StreamBuilder<DatabaseEvent>(
      stream: ref.onValue,
      builder: (context, snapshot) {
        String display = "-- °C";

        final v = snapshot.data?.snapshot.value;

        if (v is num) {
          display = "${v.toStringAsFixed(1)}°C";
        } else if (v is String) {
          final parsed = num.tryParse(v);
          if (parsed != null) display = "${parsed.toStringAsFixed(1)}°C";
        }

        return _TinyKV(k: "Temperature", v: display);
      },
    );
  }
}

/* -------------------- Tiny KV -------------------- */
class _TinyKV extends StatelessWidget {
  final String k;
  final String v;
  const _TinyKV({required this.k, required this.v});

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      mainAxisSize: MainAxisSize.min,
      children: [
        Text(k, style: labelStyle()),
        const SizedBox(height: 2),
        Text(
          v,
          style: const TextStyle(
            color: Colors.white,
            fontWeight: FontWeight.w600,
          ),
        ),
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
          Text(
            value,
            style: TextStyle(color: valueColor, fontWeight: FontWeight.w700),
          ),
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
              _ColorDot(
                color: const Color(0xFF2DB7FF),
                selected: selectedColor == 0,
                onTap: () => setState(() => selectedColor = 0),
              ),
              const SizedBox(width: 10),
              _ColorDot(
                color: const Color(0xFF2FE57A),
                selected: selectedColor == 1,
                onTap: () => setState(() => selectedColor = 1),
              ),
              const SizedBox(width: 10),
              _ColorDot(
                color: const Color(0xFFFF2D6C),
                selected: selectedColor == 2,
                onTap: () => setState(() => selectedColor = 2),
              ),
              const SizedBox(width: 10),
              _ColorDot(
                color: const Color(0xFFFFA41B),
                selected: selectedColor == 3,
                onTap: () => setState(() => selectedColor = 3),
              ),
              const SizedBox(width: 10),
              _ColorDot(
                color: Colors.white,
                selected: selectedColor == 4,
                onTap: () => setState(() => selectedColor = 4),
              ),
            ],
          ),

          const SizedBox(height: 16),
          Text(
            "Brightness: ${(brightness * 100).round()}%",
            style: const TextStyle(color: mutedText),
          ),
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
            border: Border.all(
              color: selected ? const Color(0xFF1B4DFF) : cardBorder,
            ),
          ),
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Icon(icon, color: selected ? Colors.white : mutedText, size: 18),
              const SizedBox(height: 4),
              Text(
                label,
                style: TextStyle(
                  color: selected ? Colors.white : mutedText,
                  fontSize: 12,
                ),
              ),
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

  const _ColorDot({
    required this.color,
    required this.selected,
    required this.onTap,
  });

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
              Text(
                "Sound Commands",
                style: TextStyle(
                  color: Colors.white,
                  fontWeight: FontWeight.w600,
                ),
              ),
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
              Text(
                "Vibration",
                style: TextStyle(
                  color: Colors.white,
                  fontWeight: FontWeight.w600,
                ),
              ),
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
              Text(
                "Recent Commands",
                style: TextStyle(
                  color: Colors.white,
                  fontWeight: FontWeight.w600,
                ),
              ),
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
            child: Text(
              "No commands sent yet",
              style: labelStyle().copyWith(fontStyle: FontStyle.italic),
            ),
          ),
        ],
      ),
    );
  }
}

/* -------------------- FAKE LINES ACCELEROMETER -------------------- */
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
      final y =
          size.height * 0.55 + math.sin(t * 10 * freq) * size.height * 0.18;

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
              Expanded(child: ActivityStat()),
              SizedBox(width: 12),
              Expanded(
                child: _MiniStat(
                  title: "Intensity",
                  value: "High",
                  valueColor: accentGreen,
                ),
              ),
              SizedBox(width: 12),
              Expanded(child: StepCountStat()),
              SizedBox(width: 12),
              Expanded(
                child: _MiniStat(title: "Distance", value: "6.2 km"),
              ),
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
                          style: TextStyle(
                            color: Colors.white,
                            fontWeight: FontWeight.w600,
                          ),
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
                                child: CustomPaint(
                                  painter: const GridPainter(step: 32),
                                ),
                              ),
                            ),
                            Positioned.fill(
                              child: Padding(
                                padding: const EdgeInsets.symmetric(
                                  horizontal: 8,
                                  vertical: 8,
                                ),
                                child: CustomPaint(
                                  painter: _FakeLineChartPainter(),
                                ),
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

class ActivityStat extends StatelessWidget {
  const ActivityStat({super.key});

  @override
  Widget build(BuildContext context) {
    final ref = FirebaseDatabase.instance.ref('devices/latest/motion/state');

    return StreamBuilder<DatabaseEvent>(
      stream: ref.onValue,
      builder: (context, snapshot) {
        String state = "--";
        Color color = mutedText;

        final v = snapshot.data?.snapshot.value;
        if (v != null) {
          state = v.toString();

          if (state == "running")
            color = accentGreen;
          else if (state == "walking")
            color = accentBlue;
        }

        return _MiniStat(
          title: "Activity",
          value: state.isNotEmpty
              ? state[0].toUpperCase() + state.substring(1)
              : "--",
          valueColor: color,
        );
      },
    );
  }
}

class StepCountStat extends StatelessWidget {
  const StepCountStat({super.key});

  @override
  Widget build(BuildContext context) {
    final ref = FirebaseDatabase.instance.ref(
      'devices/latest/motion/stepCount',
    );

    return StreamBuilder<DatabaseEvent>(
      stream: ref.onValue,
      builder: (context, snapshot) {
        String steps = "--";

        final v = snapshot.data?.snapshot.value;
        if (v is num) {
          steps = v.toInt().toString();
        }

        return _MiniStat(title: "Steps", value: steps);
      },
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
            style: TextStyle(
              color: valueColor,
              fontWeight: FontWeight.w700,
              fontSize: 14,
            ),
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
                        border: Border.all(
                          color: Colors.white.withOpacity(0.30),
                        ),
                      ),
                      child: const Icon(
                        Icons.play_arrow_rounded,
                        color: Colors.white,
                        size: 34,
                      ),
                    ),
                  ),

                  // bottom left text
                  Positioned(
                    left: 14,
                    bottom: 14,
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: const [
                        Text(
                          "Motion Detected",
                          style: TextStyle(
                            color: Colors.white,
                            fontWeight: FontWeight.w700,
                          ),
                        ),
                        SizedBox(height: 3),
                        Text(
                          "2025-12-11 14:32",
                          style: TextStyle(color: mutedText, fontSize: 12),
                        ),
                      ],
                    ),
                  ),

                  // duration + download (bottom right)
                  Positioned(
                    right: 14,
                    bottom: 14,
                    child: Row(
                      children: [
                        const Text(
                          "2:15",
                          style: TextStyle(color: Colors.white, fontSize: 12),
                        ),
                        const SizedBox(width: 10),
                        Container(
                          width: 34,
                          height: 34,
                          decoration: BoxDecoration(
                            color: Colors.black.withOpacity(0.25),
                            borderRadius: BorderRadius.circular(10),
                            border: Border.all(
                              color: Colors.white.withOpacity(0.20),
                            ),
                          ),
                          child: const Icon(
                            Icons.download_rounded,
                            color: Colors.white,
                            size: 18,
                          ),
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
              Text(
                "Recent Captures",
                style: TextStyle(
                  color: Colors.white,
                  fontWeight: FontWeight.w600,
                ),
              ),
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
                  padding: const EdgeInsets.symmetric(
                    horizontal: 8,
                    vertical: 4,
                  ),
                  decoration: BoxDecoration(
                    color: Colors.black.withOpacity(0.45),
                    borderRadius: BorderRadius.circular(10),
                    border: Border.all(color: Colors.white.withOpacity(0.10)),
                  ),
                  child: Text(
                    duration,
                    style: const TextStyle(color: Colors.white, fontSize: 11),
                  ),
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}

/* -------------------- GRID PAINTER -------------------- */
class GridPainter extends CustomPainter {
  final double step; // spacing between lines
  final double opacity; // line opacity (0..1)
  final double strokeWidth; // line thickness

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
