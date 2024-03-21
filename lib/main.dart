import 'package:flutter/material.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Menu Demo',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: AudioPlayerScreen(),
    );
  }
}

class AudioPlayerScreen extends StatefulWidget {
  @override
  _AudioPlayerScreenState createState() => _AudioPlayerScreenState();
}

class _AudioPlayerScreenState extends State<AudioPlayerScreen> {
  bool _isPlaying = false;
  int _currentIndex = 0; // 当前播放歌曲的索引
  double _volume = 0.5;
  double _progress = 0.0;
  final List<String> menu = [
    'tabs1',
    'tabs2',
    'tabs3',
  ]; // 假设音频文件名为 SongX.mp3
  List<String> playList = [
    'song1',
    'song2',
    'song3',
  ];

  void _showPlayList(BuildContext context) {
    showGeneralDialog(
      context: context,
      barrierDismissible: true,
      barrierLabel: MaterialLocalizations.of(context).modalBarrierDismissLabel,
      barrierColor: Colors.transparent, // 设置为透明
      transitionDuration: const Duration(milliseconds: 200),
      pageBuilder: (
        BuildContext buildContext,
        Animation<double> animation,
        Animation<double> secondaryAnimation,
      ) {
        return Align(
          alignment: Alignment.centerRight, // 从右侧弹出
          child: Container(
            width: MediaQuery.of(context).size.width * 0.3, // 设置宽度为屏幕宽度的30%
            height: MediaQuery.of(context).size.height, // 菜单的高度与屏幕高度一致
            color: Colors.white,
            child: Scaffold(
              appBar: AppBar(
                title: Text('Play List'),
                leading: Container(), // 清空leading
                actions: [
                  IconButton(
                    icon: Icon(Icons.close), // 使用close图标
                    onPressed: () {
                      Navigator.of(context).pop();
                    },
                  ),
                ],
              ),
              body: ListView.builder(
                itemCount: playList.length, // 使用播放列表的长度
                itemBuilder: (context, index) {
                  return ListTile(
                    title: Text(playList[index]), // 使用播放列表中的歌曲名
                    onTap: () {
                      _handleListItemTap(context, index);
                    },
                  );
                },
              ),
            ),
          ),
        );
      },
    );
  }

  void _handleListItemTap(BuildContext context, int index) {
    setState(() {
      _currentIndex = index;
      // 这里可以添加播放选中歌曲的逻辑
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Row(
        children: [
          Container(
            width: MediaQuery.of(context).size.width * 0.25, // 设置宽度为屏幕宽度的25%
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Padding(
                  padding: const EdgeInsets.all(8.0),
                  child: TextField(
                    decoration: InputDecoration(
                      hintText: 'Search',
                    ),
                  ),
                ),
                Expanded(
                  child: ListView.builder(
                    shrinkWrap: true,
                    itemCount: menu.length,
                    itemBuilder: (context, index) {
                      return ListTile(
                        title: Text(menu[index]),
                        onTap: () {
                          // Handle menu item tap
                        },
                      );
                    },
                  ),
                ),
              ],
            ),
          ),
          Expanded(
            flex: 2,
            child: Container(
              color: Colors.grey[200], // Placeholder color
              child: Column(
                children: [
                  TopMenuBar(
                    onPlaylistPressed: () {
                      _showPlayList(context);
                    },
                  ),
                  Expanded(
                    child: Container(),
                  ),
                ],
              ),
            ),
          ),
        ],
      ),
    );
  }
}

class TopMenuBar extends StatelessWidget {
  final VoidCallback? onPlaylistPressed;

  const TopMenuBar({Key? key, this.onPlaylistPressed}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Container(
      color: Colors.blue, // Placeholder color
      child: Row(
        children: [
          PlaybackControls(),
          VolumeControls(), // Add VolumeControls here
          Spacer(),
          IconButton(
            icon: Icon(Icons.queue_music),
            onPressed: onPlaylistPressed,
          ),
        ],
      ),
    );
  }
}

class PlaybackControls extends StatelessWidget {
  void _playPause() {
    // Play/pause logic
  }

  @override
  Widget build(BuildContext context) {
    return Row(
      children: [
        IconButton(
          icon: Icon(Icons.shuffle),
          onPressed: () {},
        ),
        IconButton(
          icon: Icon(Icons.skip_previous),
          onPressed: () {},
        ),
        IconButton(
          icon: Icon(Icons.pause), // Assuming initial state is paused
          onPressed: _playPause,
        ),
        IconButton(
          icon: Icon(Icons.skip_next),
          onPressed: () {},
        ),
        IconButton(
          icon: Icon(Icons.repeat),
          onPressed: () {},
        ),
      ],
    );
  }
}

class VolumeControls extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.center,
      children: [
        Slider(
          value: 0.5, // Placeholder value, replace with _volume
          onChanged: (double value) {
            // Adjust volume based on value
          },
        ),
      ],
    );
  }
}