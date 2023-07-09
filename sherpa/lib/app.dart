import 'dart:convert';
import 'dart:typed_data';

import 'package:flutter/cupertino.dart';
import 'package:permission_handler/permission_handler.dart';
import 'package:flutter_bluetooth_serial/flutter_bluetooth_serial.dart';

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
  final ble = FlutterBluetoothSerial.instance;
  Status status = Status.searching;
  BluetoothConnection? connection;

  void receiveData(Uint8List data) {
    String msg = const Utf8Decoder().convert(data);
    print("RECEIVED MESSAGE: $msg");
  }

  Future<void> init() async {
    await Permission.bluetoothScan.request();

    ble.startDiscovery().listen((ev) {
      print("FOUND DEVICE: ${ev.device.name}");

      if (ev.device.name == "Sherpa") {
        setState(() {
          status = Status.connecting;
        });

        BluetoothConnection.toAddress(ev.device.address).then((conn) {
          setState(() {
            status = Status.connected;
            connection = conn;
            conn.input!.listen(receiveData);
            conn.output.add(Uint8List.fromList(utf8.encode("CONN\n")));
            print("CONNECTED");
          });
        });
      }
    });
  }

  @override
  void initState() {
    super.initState();
    init();
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
        return const Text("Connected to Sherpa");
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
