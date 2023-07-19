import 'dart:convert';
import 'dart:math';
import 'dart:typed_data';

import 'package:flutter/cupertino.dart';
import 'package:permission_handler/permission_handler.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:flutter_map/flutter_map.dart';
import 'package:flutter_map_location_marker/flutter_map_location_marker.dart';
import 'package:latlong2/latlong.dart';
import 'package:geolocator/geolocator.dart';

class SherpaPage extends StatefulWidget {
  const SherpaPage({super.key});

  @override
  State<SherpaPage> createState() => _SherpaPageState();
}

enum Status {
  searching,
  connecting,
  connected,
}

class _SherpaPageState extends State<SherpaPage> {
  final ble = FlutterBluePlus.instance;
  Status status = Status.searching;
  late BluetoothDevice device;
  late BluetoothCharacteristic megaCharacteristic;

  // Nano
  late BluetoothCharacteristic gpsCharacteristic;
  late BluetoothCharacteristic statusCharacteristic;
  Guid gpsUuid = Guid("f731ae25-0001-4677-9604-4a75debdaad0");
  Guid statusUuid = Guid("f731ae25-0002-4677-9604-4a75debdaad0");

  // Ble data
  bool mega = false;
  int bleStatus = 0;
  double bleLat = 0;
  double bleLng = 0;
  double bleHeading = 0;

  // Listener
  String buff = "";
  void listenerMega(List<int> value) {
    String msg = const Utf8Decoder().convert(value);
    buff += msg.trim();
    if (msg.endsWith("\n")) {
      Map<String, dynamic> data = jsonDecode(buff.trim());
      buff = "";
      setState(() {
        bleStatus = data["status"];
        if (bleStatus == 1) {
          bleLat = (data["lat"] as num).toDouble();
          bleLng = (data["lng"] as num).toDouble();
          bleHeading = (data["heading"] as num).toDouble();
          writeBleMega();
        }
      });
    }
  }

  // Writer
  void writeBleMega() async {
    Position pos = await Geolocator.getCurrentPosition();
    Map<String, dynamic> data = {
      "lat": pos.latitude,
      "lng": pos.longitude,
      "heading": pos.heading,
      "status": 1,
    };
    String msg = "${jsonEncode(data)}\n";
    for (int i = 0; i < msg.length; i += 20) {
      await megaCharacteristic.write(
          utf8.encode(msg.substring(i, min(i + 20, msg.length))),
          withoutResponse: true);
    }
  }

  void gpsListener(List<int> value) {
    final bytes = Uint8List.fromList(value);
    final byteData = ByteData.sublistView(bytes);
    for (var i = 0; i < bytes.length; i += 4) {
      double value = byteData.getFloat32(i);
      if (i == 0) {
        bleLat = value;
      } else if (i == 4) {
        bleLng = value;
      } else if (i == 8) {
        bleHeading = value;
      }
    }

    setState(() {
      bleLat = bleLat;
      bleLng = bleLng;
      bleHeading = bleHeading;
    });

    // Write data
    gpsWrite();
  }

  void gpsWrite() async {
    Position pos = await Geolocator.getCurrentPosition();
    final data =
        Float32List.fromList([pos.latitude, pos.longitude, pos.heading]);
    final bytes = data.buffer.asUint8List();
    await gpsCharacteristic.write(bytes.toList());
    await statusCharacteristic.write([1]);
  }

  void statusListener(List<int> value) {
    setState(() {
      bleStatus = value[0];
    });
  }

  Future<bool> checkDevice(BluetoothDevice d, bool alreadyConnected) async {
    if (d.name.startsWith("Sherpa") && status == Status.searching) {
      ble.stopScan();

      if (d.name == "SherpaM") {
        mega = true; // Connected to arduino MEGA
      }

      setState(() {
        status = Status.connecting;
        device = d;
      });

      if (!alreadyConnected) {
        await d.connect(); // Need to connect since this was from a scan
      }

      // Discover services
      if (mega) {
        List<BluetoothService> services = await d.discoverServices();
        for (BluetoothService service in services) {
          List<BluetoothCharacteristic> characteristics =
              service.characteristics;
          for (BluetoothCharacteristic c in characteristics) {
            if (c.properties.write &&
                c.properties.writeWithoutResponse &&
                c.properties.read &&
                c.properties.notify) {
              await c.write(utf8.encode("CONN\n"),
                  withoutResponse: true); // CONNECT MESSAGE
              await c.setNotifyValue(true);

              setState(() {
                status = Status.connected;
                megaCharacteristic = c;
                c.value.listen(listenerMega);
              });

              return true;
            }
          }
        }
      } else {
        List<BluetoothService> services = await d.discoverServices();
        for (BluetoothService service in services) {
          List<BluetoothCharacteristic> characteristics =
              service.characteristics;
          for (BluetoothCharacteristic c in characteristics) {
            if (c.uuid == gpsUuid) {
              gpsCharacteristic = c;
              await c.setNotifyValue(true);
            } else if (c.uuid == statusUuid) {
              statusCharacteristic = c;
              await c.setNotifyValue(true);
            }
          }
        }

        setState(() {
          status = Status.connected;
          gpsCharacteristic.value.listen(gpsListener);
          statusCharacteristic.value.listen(statusListener);
          statusCharacteristic.write([3]); // Waiting for GPS lock
        });
      }
    }

    return false;
  }

  Future<void> init() async {
    await Permission.bluetoothScan.request();
    await Permission.locationWhenInUse.request();

    // Check if connected device
    for (BluetoothDevice d in await ble.connectedDevices) {
      if (await checkDevice(d, true)) {
        return;
      }
    }

    // Scan
    ble.startScan();
    ble.scanResults.listen((results) async {
      for (ScanResult r in results) {
        if (await checkDevice(r.device, false)) {
          return;
        }
      }
    });
  }

  @override
  void initState() {
    super.initState();
    init();
  }

  @override
  void dispose() {
    super.dispose();
    if (status != Status.searching) {
      device.disconnect();
    }
  }

  Widget buildBody(BuildContext context) {
    switch (status) {
      case Status.searching:
        return const Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            Spacer(flex: 20),
            CupertinoActivityIndicator(radius: 20.0),
            Spacer(),
            Text("Searching for devices"),
            Spacer(flex: 20),
          ],
        );

      case Status.connecting:
        return const Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            Spacer(flex: 20),
            CupertinoActivityIndicator(radius: 20.0),
            Spacer(),
            Text("Connecting to Sherpa"),
            Spacer(flex: 20),
          ],
        );

      case Status.connected:
        switch (bleStatus) {
          case 0:
            return Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: <Widget>[
                const Spacer(flex: 20),
                const Icon(CupertinoIcons.exclamationmark_circle,
                    color: CupertinoColors.systemRed, size: 40),
                const Spacer(),
                Text(mega ? "JSON Parse Fail" : "No data received"),
                const Spacer(flex: 20),
              ],
            );

          case 1:
            return FlutterMap(
              options: MapOptions(),
              children: [
                TileLayer(
                  urlTemplate:
                      'http://{s}.google.com/vt/lyrs=s,h&x={x}&y={y}&z={z}',
                  subdomains: const ['mt0', 'mt1', 'mt2', 'mt3'],
                  maxZoom: 20,
                ),
                CurrentLocationLayer(
                  followOnLocationUpdate: FollowOnLocationUpdate.once,
                ),
                MarkerLayer(
                  markers: [
                    Marker(
                      point: LatLng(bleLat, bleLng),
                      width: 20,
                      height: 20,
                      builder: (context) => const DefaultLocationMarker(
                          color: CupertinoColors.activeOrange),
                    )
                  ],
                )
              ],
            );

          case 2:
            return const Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: <Widget>[
                Spacer(flex: 20),
                Icon(CupertinoIcons.exclamationmark_circle,
                    color: CupertinoColors.systemRed, size: 40),
                Spacer(),
                Text("IMU Read Fail"),
                Spacer(flex: 20),
              ],
            );

          case 3:
            return const Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: <Widget>[
                Spacer(flex: 20),
                CupertinoActivityIndicator(radius: 20.0),
                Spacer(),
                Text("Waiting for GPS Lock"),
                Spacer(flex: 20),
              ],
            );

          default:
            return Text("Unknown status code: $bleStatus");
        }
    }
  }

  @override
  Widget build(BuildContext context) {
    return CupertinoPageScaffold(
      navigationBar: const CupertinoNavigationBar(
        middle: Text('Sherpa'),
      ),
      child: Center(
        child: buildBody(context),
      ),
    );
  }
}
