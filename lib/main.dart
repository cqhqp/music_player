import 'package:flutter/material.dart';

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
  final List<String> _songs =  [
    '周杰伦-漂移1.mp3',
    '周杰伦-漂移2.mp3',
    '周杰伦-漂移3.mp3',
    '周杰伦-漂移4.mp3',
    '周杰伦-漂移5.mp3',
    '周杰伦-漂移6.mp3',
    '周杰伦-漂移7.mp3',
    '周杰伦-漂移8.mp3',
    '周杰伦-漂移9.mp3',
    '周杰伦-漂移10.mp3',
    '周杰伦-漂移11.mp3',
    '周杰伦-漂移12.mp3',
    '周杰伦-漂移13.mp3',
    '周杰伦-漂移14.mp3',
    '周杰伦-漂移15.mp3',
    '周杰伦-漂移16.mp3',
    '周杰伦-漂移17.mp3',
    '周杰伦-漂移18.mp3',
    '周杰伦-漂移19.mp3',
    '周杰伦-漂移20.mp3',
    '周杰伦-漂移21.mp3',
    '周杰伦-漂移22.mp3',
    '周杰伦-漂移23.mp3',
    '周杰伦-漂移24.mp3',
    '周杰伦-漂移25.mp3',
    '周杰伦-漂移26.mp3',
    '周杰伦-漂移27.mp3',
    '周杰伦-漂移28.mp3',
    '周杰伦-漂移29.mp3',
    '周杰伦-漂移30.mp3',
  ]; // 假设音频文件名为 SongX.mp3

  @override
  void initState() {
    super.initState();
    _isPlaying = false;
  }

  @override
  void dispose() {
    super.dispose();
  }

  void _playPause() async {
    setState(() {
      _isPlaying = !_isPlaying;
    });
  }

  void _stop() async {
    setState(() {});
  }

  Widget _buildInfo() {
    return const Column(mainAxisAlignment: MainAxisAlignment.center, children: [
      // Image.asset('images/hello.jpeg', width: 200, height: 200),
      Text(
        // 歌曲名
        '漂移',
        style: TextStyle(fontSize: 50),
      ),

      SizedBox(height: 10),
      // 其他歌曲信息（如艺术家、专辑等）
      Text(
        // 歌曲名
        '周杰伦：头文字D',
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
            onPressed: () {},
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
          _playPause();
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
            _songs[index].substring(0, _songs[index].lastIndexOf('.')), index);
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
