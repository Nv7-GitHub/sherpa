import 'package:flutter/cupertino.dart';
import 'package:sherpa/app.dart';

void main() {
  runApp(const SherpaApp());
}

class SherpaApp extends StatelessWidget {
  const SherpaApp({super.key});

  // This widget is the roo`t of your application.
  @override
  Widget build(BuildContext context) {
    return const CupertinoApp(
      title: 'Sherpa',
      theme: CupertinoThemeData(
        brightness: Brightness.light,
      ),
      home: SherpaPage(),
    );
  }
}
