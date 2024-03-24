import 'package:flutter/services.dart';

class AudioPlugin {
  Map<String, dynamic> arguments = {
    'key1': 'value1',
    'key2': 123,
    'key3': true,
  };

  static const MethodChannel _channel =
      MethodChannel('flutter_windows_cpp_plugin'); // 通道名

  Future<void> stop() async {
    try {
      final bool result = await _channel.invokeMethod("stop", arguments); // 方法名
      print("call stop result=$result.");
    } on PlatformException catch (e) {
      print("call stop Error!");
    }
  }

  // 播放音频
  Future<Map<String?, String?>> play(String? path) async {
    Map<String?, String?> out={};
    if (path == null) {
      return out;
    }else {
      try {
        final Map<Object?, Object?> result = await _channel.invokeMethod('play', path);
        Map<String?, String?> stringMap = result.cast<String?, String?>();

        print(result["title"]);
        print(result["artist"]);
        print(result["album"]);
        return stringMap;
      } on PlatformException catch (e) {
        print("Error playing audio");
      }
    }
    return out;
  }

  // 暂停音频
  Future<void> pause() async {
    try {
      final bool result = await _channel.invokeMethod('pause');
    } on PlatformException catch (e) {
      print("Error pausing audio");
    }
  }
}
