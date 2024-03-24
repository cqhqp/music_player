
class AudioFile {
  late String filename;
  late String? type;
  late int size;  
  late String? path;
  String singer = "未知";
  String album = "未知";

  AudioFile(String name, String? type, int size, String? path) {  
    this.filename = name;  
    this.type = type;  
    this.size = size;  
    this.path = path;  
  }  
}