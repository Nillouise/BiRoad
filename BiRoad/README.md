注意：貌似git没有把lib和dll文件上传到github，这个需要自己再手动下一遍吧。

配置环境的要点：
1 配置好include和lib的路径，可以用$(ProjectDir)这个环境变量
2 把linker 的System中的SubSystem 从console改成window，这样可以让console窗口不弹出来。
3 把dll文件放到跟exe文件同一个目录。
4 注意configuration是64还是x86
5 配置SDL image时，要把所有（4个以上）的dll文件都复制到执行目录里才行，不然加载png时既不报错，也无法显示图片

后台应该怎么做？
在handle那里需要处理对面网络的消息，那么问题来了，这网络是阻塞的还是非阻塞回调？还是说用阻塞多线程比较好？
总的来说，就是另外一条蛇的数据是由对应的网络链接来操作的。
先搞个网络库吧。
感觉还要写个AI。双人对战，snake岂不是要从maze独立出来？ai操作也不能隔着maze呀。

那么需要弄懂的功能就是ai编写，网络库使用，线程问题，游戏要增加输入框，以及一点文件读写功能。


asio
直接在vs2015的preprocessor那里定义：
	ASIO_STANDALONE 
	ASIO_HAS_BOOST_DATE_TIME
由于没定义宏可能让vs的intellisense显示出错误，而build的时候却是正常的，所以自己跟踪到header文件里，看看需要定义哪些宏。


libevent
1 这库在官网的版本有bug的，在github上的反而没有（在release 的tag目录的版本），以后找库多用github，不要在官网上找了！！！！
2 用到网络库的程序基本都要额外连接 ws2_32.lib wsock32.lib
3 解决我问题的网页 http://blog.csdn.net/xiaoluer/article/details/56280681
4 编译这库用的是vs 自带的nmake /a /f Makefile.namke 命令就行，不需要cmake等花里胡哨的，可能别的库也都支持nmake了吧
