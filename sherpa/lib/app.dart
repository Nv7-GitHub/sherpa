import 'dart:convert';
import 'dart:typed_data';

import 'package:flutter/cupertino.dart';
import 'package:permission_handler/permission_handler.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';

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
  late BluetoothCharacteristic characteristic;

  // Ble data
  int bleStatus = 0;
  int bleLat = 0;
  int bleLng = 0;

  // Listener
  String buff = "";
  void listener(List<int> value) {
    String msg = const Utf8Decoder().convert(value);
    buff += msg.trim();
    if (msg.endsWith("\n")) {
      Map<String, dynamic> data = jsonDecode(buff.trim());
      buff = "";
      setState(() {
        bleLat = data["lat"];
        bleLng = data["lng"];
        bleStatus = data["status"];
      });
    }
  }

  Future<bool> checkDevice(BluetoothDevice d, bool alreadyConnected) async {
    if (d.name == "Sherpa" && status == Status.searching) {
      ble.stopScan();

      setState(() {
        status = Status.connecting;
        device = d;
      });

      if (!alreadyConnected) {
        await d.connect(); // Need to connect since this was from a scan
      }

      // Discover services
      List<BluetoothService> services = await d.discoverServices();
      for (BluetoothService service in services) {
        List<BluetoothCharacteristic> characteristics = service.characteristics;
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
              characteristic = c;
              c.value.listen(listener);
            });

            return true;
          }
        }
      }
    }

    return false;
  }

  Future<void> init() async {
    await Permission.bluetoothScan.request();

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
    if (Status != Status.searching) {
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
            return const Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: <Widget>[
                Spacer(flex: 20),
                Icon(CupertinoIcons.exclamationmark_circle,
                    color: CupertinoColors.systemRed, size: 40),
                Spacer(),
                Text("JSON Parse Fail"),
                Spacer(flex: 20),
              ],
            );

          case 1:
            return const Text("Successfully connected!");

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
