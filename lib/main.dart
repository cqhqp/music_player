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
        // primarySwatch: Colors.red,
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
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Row(
        children: [
          Container(
            width: 200,
            child: MenuList(menu: menu),
          ),
          Expanded(
            flex: 2,
            child: Container(
              color: Color.fromARGB(255, 255, 255, 255), // Placeholder color
              child: Column(
                children: [
                  TopMenuBar(
                    onPlaylistPressed: () {
                      _showPlayList(context);
                    },
                  ),
                  Expanded( // Main Window
                    // child: RepositoryPage(),
                    child: AlbumGrid(albumRepository: albumRepository),
                    // child: AlbumRepositoryWidget(albumRepository: albumRepository),
                  ),
                ],
              ),
            ),
          ),
        ],
      ),
    );
  }

  // Add the following lines to define variables and methods used in the build method

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
    
  final albumRepository = AlbumRepository([
    Album("Album 1", "Song 1", "Artist 1"),
    Album("Album 2", "Song 2", "Artist 2"),
    Album("Album 3", "Song 3", "Artist 3"),
    Album("Album 4", "Song 4", "Artist 4"),
    Album("Album 5", "Song 5", "Artist 5"),
    Album("Album 6", "Song 6", "Artist 6"),
    Album("Album 7", "Song 7", "Artist 7"),
    Album("Album 8", "Song 8", "Artist 8"),
    // 添加更多专辑
  ]);

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
}

class TopMenuBar extends StatelessWidget {
  final VoidCallback? onPlaylistPressed;

  const TopMenuBar({Key? key, this.onPlaylistPressed}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Container(
      color: Color.fromARGB(255, 226, 226, 226), // Placeholder color
      height: 53, // 设置TopMenuBar的高度为60像素
      child: Row(
        mainAxisAlignment: MainAxisAlignment.center, // 让所有子部件在水平方向居中对齐
        mainAxisSize: MainAxisSize.max, // 让Row尽可能占据水平空间
        children: [
          Expanded(
            flex: 2,
            child: PlaybackControls(),
          ),
          Expanded(
            flex: 3,
            child: MusicInfoCard(),
          ),
          Expanded(
            flex: 2,
            child: VolumeControls(),
          ),
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
    double button_icon_size = 19;
    double transform_scale = 1.2;
    double icon_width = 0;

    return Center( // 使用Center居中
      child: ConstrainedBox(
        constraints: BoxConstraints(
          maxWidth: MediaQuery.of(context).size.width, // 设置最大宽度
        ),
        child: IntrinsicWidth(
          child: Row(
            mainAxisAlignment: MainAxisAlignment.center, // 水平方向居中
            children: [
              Transform.scale(
                scale: transform_scale,
                child: IconButton(
                  icon: Icon(Icons.shuffle),
                  iconSize: button_icon_size,
                  onPressed: () {},
                  constraints: BoxConstraints(minWidth: 0, minHeight: 0),
                ),
              ),
              SizedBox(width: icon_width),
              Transform.scale(
                scale: transform_scale,
                child: IconButton(
                  icon: Icon(Icons.skip_previous),
                  iconSize: button_icon_size,
                  onPressed: () {},
                  constraints: BoxConstraints(minWidth: 0, minHeight: 0),
                ),
              ),
              SizedBox(width: icon_width),
              Transform.scale(
                scale: transform_scale,
                child: IconButton(
                  icon: Icon(Icons.pause), // Assuming initial state is paused
                  iconSize: button_icon_size,
                  onPressed: _playPause,
                  constraints: BoxConstraints(minWidth: 0, minHeight: 0),
                ),
              ),
              SizedBox(width: icon_width),
              Transform.scale(
                scale: transform_scale,
                child: IconButton(
                  icon: Icon(Icons.skip_next),
                  iconSize: button_icon_size,
                  onPressed: () {},
                  constraints: BoxConstraints(minWidth: 0, minHeight: 0),
                ),
              ),
              SizedBox(width: icon_width),
              Transform.scale(
                scale: transform_scale,
                child: IconButton(
                  icon: Icon(Icons.repeat),
                  iconSize: button_icon_size,
                  onPressed: () {},
                  constraints: BoxConstraints(minWidth: 0, minHeight: 0),
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}



class VolumeControls extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Center(
      // flex: 1,
      child: Container(
        width: 200, // 设置容器的固定宽度
        height: 60, // 设置容器的固定高度
      color: Color.fromARGB(255, 226, 226, 226), // Placeholder color
        child: Row(
          mainAxisSize: MainAxisSize.min, // 让 Row 尽可能紧凑地包裹其子部件
          mainAxisAlignment: MainAxisAlignment.center, // 让子部件在水平方向居中对齐
          crossAxisAlignment: CrossAxisAlignment.center,
          children: [
            SizedBox(
              // width: 30, // 调整按钮之间的间距
              height: double.infinity, // 使 Slider 填充 Container 的整个高度
              child: Transform.scale(
                scale: 0.8, // 减小按钮的大小
                child: IconButton(
                  icon: const Icon(Icons.volume_down, size: 20),
                  onPressed: () {},
                ),
              ),
            ),
            
            SizedBox(
              width: 100, // 设置 Slider 的宽度为 100
              height: 60, // 使 Slider 和容器一样高
              child: SliderTheme(
                data: SliderTheme.of(context).copyWith(
                  trackHeight: 2, // 设置轨道的高度
                  // trackShape: RectangularSliderTrackShape(), // 设置轨道形状为矩形
                  thumbShape: RoundSliderThumbShape(enabledThumbRadius: 6.0), // 设置滑块形状为圆形
                  overlayShape: RoundSliderOverlayShape(overlayRadius: 0.0), // 设置覆盖形状为圆形
                ),
                child: Slider(
                  value: 0.5,
                  onChanged: (double value) {
                    // 根据值调整音量
                  },
                ),
              ),
            ),



            SizedBox(
              // width: 30, // 调整按钮之间的间距
              height: double.infinity, // 使 Slider 填充 Container 的整个高度
              child: Transform.scale(
                scale: 1, // 减小按钮的大小
                child: IconButton(
                  icon: const Icon(Icons.volume_up, size: 20),
                  onPressed: () {},
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }
}

// Album






// Album 类
class Album {
  final String name;
  final String song;
  final String artist;

  Album(this.name, this.song, this.artist);
}

// 专辑仓库类
class AlbumRepository {
  List<Album> albums;

  AlbumRepository(this.albums);

  void addAlbum(Album album) {
    albums.add(album);
  }
}

// 专辑卡片组件
class AlbumCard extends StatelessWidget {
  final Album album;
  final double scaleFactor;

  const AlbumCard({Key? key, required this.album, this.scaleFactor = 1.2}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Transform.scale(
      scale: scaleFactor,
      child: Container(
        clipBehavior: Clip.none, // 告诉 Flutter 不要剪裁溢出的部分
        child: FractionallySizedBox(
          widthFactor: 0.8, // 设置 Card 宽度为 AlbumCard 宽度的 80%
          child: Card(
            child: Container(
              child: AspectRatio(
                aspectRatio: 1.0, // 设置宽高比为1:1，保持正方形
                child: Column(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    Icon(Icons.album, size: 70), // 专辑图标
                    SizedBox(height: 8),
                    Text(album.name), // 专辑名称
                  ],
                ),
              ),
            ),
          ),
        ),
      ),
    );
  }
}



// 专辑网格组件
class AlbumGrid extends StatelessWidget {
  final AlbumRepository albumRepository;

  const AlbumGrid({Key? key, required this.albumRepository}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return GridView.builder(
      padding: EdgeInsets.all(33.0),
      gridDelegate: SliverGridDelegateWithFixedCrossAxisCount(
        crossAxisCount: 4, // 每行显示4个专辑
        crossAxisSpacing: 6.0,
        mainAxisSpacing: 26.0,
        childAspectRatio: 1.0, // 专辑卡片保持正方形
      ),
      itemCount: albumRepository.albums.length,
      itemBuilder: (context, index) {
        return Column(
          crossAxisAlignment: CrossAxisAlignment.center,
          children: [
            AlbumCard(album: albumRepository.albums[index]), // 专辑卡片
            Spacer(),
            // SizedBox(height: 0),
            Text(
              'Title ${index + 1}', // 标题
              style: TextStyle(fontSize: 13),
            ),
          ],
        );
      },
    );
  }
}


















// music info card
class MusicInfoCard extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Card(
      shape: RoundedRectangleBorder(
        borderRadius: BorderRadius.circular(3.0),
      ),
      child: Container(
        width: 300.0,
        height: 42.5,
        child: Row(
          crossAxisAlignment: CrossAxisAlignment.center,
          children: [
            ClipRect(
              child: AlbumPicture(),
            ),
            SizedBox(width: 3.0),
            Expanded(
              child: MusicState(),
            ),
          ],
        ),
      ),
    );
  }
}

class AlbumPicture extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Container(
      width: 42.5,
      height: 42.5,
      decoration: BoxDecoration(
        color: Colors.grey,
        borderRadius: BorderRadius.only(
          topLeft: Radius.circular(3.0),
          bottomLeft: Radius.circular(3.0),
        ),
      ),
    );
  }
}

class MusicState extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.center,
      children: [
        SongInfo(),
        SizedBox(height: 13.5),
        CustomSlider(),
      ],
    );
  }
}

class SongInfo extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.center,
      children: [
        Padding(
          padding: EdgeInsets.only(top: 0.0, bottom: 0.5),
          child: Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            crossAxisAlignment: CrossAxisAlignment.center,
            children: [
              Expanded(
                child: Text(
                  'Song Title',
                  textAlign: TextAlign.center,
                  style: TextStyle(fontSize: 12.4),
                ),
              ),
              IconButton(
                onPressed: () {
                  // Add your like functionality here
                },
                icon: Transform.scale(
                  scale: 1.6,
                  child: Icon(Icons.star_border, size: 10.0),
                ),
                constraints: BoxConstraints(minWidth: 0, minHeight: 0),
              )
            ],
          ),
        ),
        Padding(
          padding: EdgeInsets.only(top: 0.0, bottom: 0.0),
          child: Text(
            'Singer - Album Name',
            style: TextStyle(fontSize: 11.5, color: Colors.grey, height: 0.1),
          ),
        )
      ],
    );
  }
}

class CustomSlider extends StatefulWidget {
  @override
  _CustomSliderState createState() => _CustomSliderState();
}

class _CustomSliderState extends State<CustomSlider> {
  double _value = 0.5;

  @override
  Widget build(BuildContext context) {
    return SliderTheme(
      data: SliderTheme.of(context).copyWith(
        trackHeight: 1.5,
        thumbShape: RoundSliderThumbShape(enabledThumbRadius: 0.0),
        overlayShape: RoundSliderOverlayShape(overlayRadius: 0.0),
      ),
      child: Slider(
        value: _value,
        onChanged: (double value) {
          setState(() {
            _value = value;
          });
          // Add functionality to change progress here
        },
      ),
    );
  }
}



// Menu List

class MenuList extends StatefulWidget {
  final List<String> menu;

  const MenuList({Key? key, required this.menu}) : super(key: key);

  @override
  _MenuListState createState() => _MenuListState();
}

class _MenuListState extends State<MenuList> {
  int _selectedIndex = -1; // 初始未选中

  @override
  Widget build(BuildContext context) {
    return ClipRRect(
      borderRadius: BorderRadius.circular(8.0), // 添加圆角
      child: Container(
        decoration: BoxDecoration(
          border: Border.all(color: Colors.grey.withOpacity(0.5)), // 添加边框
        ),
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
                itemCount: widget.menu.length,
                itemBuilder: (context, index) {
                  return InkWell(
                    onTap: () {
                      setState(() {
                        _selectedIndex = index; // 更新选中索引
                      });
                      // 这里添加处理菜单项点击的逻辑
                    },
                    child: Container(
                      color: _selectedIndex == index ? Colors.grey.withOpacity(0.5) : null,
                      child: ListTile(
                        title: Text(widget.menu[index]),
                      ),
                    ),
                  );
                },
              ),
            ),
          ],
        ),
      ),
    );
  }
}
