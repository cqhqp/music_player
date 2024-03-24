import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import './audio_plugin.dart';
import './audio_file.dart';
import 'package:file_picker/file_picker.dart';

void main() {
  runApp(const MainApp());
}

class MainApp extends StatelessWidget {
  const MainApp({super.key});

  @override
  Widget build(BuildContext context) {
    return const MaterialApp(
      home: AudioPlayerScreen(),
    );
  }
}

class AudioPlayerScreen extends StatefulWidget {
  const AudioPlayerScreen({Key? key}) : super(key: key);

  @override
  _AudioPlayerScreenState createState() => _AudioPlayerScreenState();
}

class _AudioPlayerScreenState extends State<AudioPlayerScreen> {
  bool _isPlaying = false;
  int _currentIndex = 0; // 当前播放歌曲的索引
  double _volume = 0.0;
  double _progress = 0.0;
  List<AudioFile> _songs = [];
  AudioPlugin _audioPlugin = new AudioPlugin();
  AudioFile file = new AudioFile("name", "type", 0, null);
  String _song_name = "NONE";
  String _song_artist = "NONE";
  String _song_album = "NONE";
  static const EventChannel eventChannel = EventChannel('samples.flutter.io/charging');

  @override
  void initState() {
    super.initState();
    _isPlaying = false;
    eventChannel.receiveBroadcastStream().listen(_onEvent, onError: _onError); // 订阅底层事件
  }

  @override
  void dispose() {
    super.dispose();
  }

  void _onEvent(Object? event) {
    String str = event.toString();
    setState(() {
      print("event:"+str);
    });
  }

  void _onError(Object error) {
    setState(() {
    });
  }

  // 停止
  void _open() async {
    FilePickerResult? result = await FilePicker.platform.pickFiles(
      type: FileType.custom,
      allowedExtensions: ['mp3', 'MP3'],
    );
    if (result != null) {
      print(result.files.single.name);
      print(result.files.single.extension);
      print(result.files.single.size);
      print(result.files.single.path);
      file = new AudioFile(
          result.files.single.name,
          result.files.single.extension,
          result.files.single.size,
          result.files.single.path);

      setState(() {
        _songs.add(file);
        _play();
      });
    } else {
      // User canceled the picker
    }
  }

  // 播放
  void _play() async {
    _isPlaying = true;    
    Map<String?, String?> out = await _audioPlugin.play(file.path);
    _song_name = out["title"].toString();
    _song_artist = out["artist"].toString();
    _song_album = out["album"].toString();
  }

  // 播放和暂停
  void _playPause() async {
    setState(() {
      _isPlaying = !_isPlaying;
    });
    if (_isPlaying) {
      _play();
    } else {
      _audioPlugin.pause();
    }
  }

  // 停止
  void _stop() async {
    setState(() {
      _audioPlugin.stop();
    });
  }

  // 音乐信息
  Widget _buildInfo() {
    return Column(mainAxisAlignment: MainAxisAlignment.center, children: [
      // Image.asset('images/hello.jpeg', width: 200, height: 200),
      Text(
        // 歌曲名
        _song_name,
        style: TextStyle(fontSize: 50),
      ),

      SizedBox(height: 10),
      // 其他歌曲信息（如艺术家、专辑等）
      Text(
        // 歌曲名        
        _song_artist+":"+_song_album,
        style: TextStyle(fontSize: 20),
      ),

      SizedBox(height: 10),
    ]);
  }

  Widget _buildPlayback() {
    // 播放控制
    return Row(children: <Widget>[
      // 左边的控件
      Container(
        // color: Colors.blue,
        child: Row(
          children: [
            IconButton(
              icon: Icon(_isPlaying ? Icons.pause : Icons.play_arrow),
              iconSize: 50,
              onPressed: _playPause,
              tooltip: _isPlaying ? 'Pause' : 'Play',
            ),
            IconButton(
              icon: const Icon(Icons.stop),
              iconSize: 50,
              onPressed: _stop,
              tooltip: 'Stop',
            ),
            IconButton(
              icon: const Icon(Icons.skip_next),
              iconSize: 50,
              onPressed: () {},
              tooltip: 'Next Song',
            ),
            IconButton(
              icon: const Icon(Icons.skip_previous),
              iconSize: 50,
              onPressed: () {},
              tooltip: 'Previous Song',
            ),
          ],
        ),
      ),

      // 占据剩余空间的Expanded widget
      Expanded(
        child: Container(
          color: Colors.transparent, // 透明背景，仅用于占据空间
        ),
      ),

      // 右边的控件
      Container(
        // width: 50, // 或者其他固定宽度
        // height: 50,
        // color: Colors.red,
        child: Row(children: [
          IconButton(
            icon: const Icon(Icons.music_video),
            iconSize: 50,
            onPressed: _open,
            tooltip: 'open music',
          ),
          IconButton(
            icon: const Icon(Icons.volume_up),
            iconSize: 50,
            onPressed: () {},
            tooltip: 'Previous Song',
          ),
          Container(
            width: 200.0, // 设置Container的宽度，Slider会继承这个宽度
            child: Slider(
              value: _volume,
              min: 0.0,
              max: 1.0,
              onChanged: (double newValue) {
                setState(() {
                  _volume = newValue;
                });
              },
            ),
          )
        ]),
      ),
    ]);
  }

  Widget _buildPlaylistItem(String song, int index) {
    return ListTile(
      title: Text(song),
      leading: Radio(
        value: index,
        groupValue: _currentIndex,
        onChanged: (int? value) {
          if (value != null) {
            setState(() {
              _currentIndex = value;
              _playPause();
            });
          }
        },
        activeColor: Theme.of(context).primaryColor,
      ),
      onTap: () {
        setState(() {
          _currentIndex = index;  
          file = _songs[_currentIndex];
          _play();
        });
      },
    );
  }

  Widget _buildPlaylist() {
    return ListView.builder(
      shrinkWrap: true,
      itemCount: _songs.length,
      itemBuilder: (BuildContext context, int index) {
        return _buildPlaylistItem(
            _songs[index].filename.substring(0, _songs[index].filename.lastIndexOf('.')), index);
      },
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      // appBar: AppBar(
      //   title: Text('Audio Player'),
      // ),
      body: Center(
        child: Row(
          mainAxisAlignment: MainAxisAlignment.start,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Expanded(
              flex: 7, // 左侧70%

              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  // 占据剩余空间的Expanded widget
                  Expanded(
                    child: Container(
                      color: Colors.transparent,
                    ),
                  ),

                  _buildInfo(),

                  // 占据剩余空间的Expanded widget
                  Expanded(
                    child: Container(
                      color: Colors.transparent,
                    ),
                  ),
                  Container(
                    child: Slider(
                      value: _progress,
                      min: 0.0,
                      max: 100.0,
                      onChanged: (double newValue) {
                        setState(() {
                          _progress = newValue;
                        });
                      },
                    ),
                  ),

                  // 第二个右侧按钮
                  _buildPlayback(),
                ],
              ),
            ),
            Expanded(
              flex: 3, // 左侧按钮占20%
              child: _buildPlaylist(),
            )
          ],
        ),
      ),
    );
  }
}
