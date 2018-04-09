# SillyEcho

**傻子一样的语音助手，目前采用科大讯飞WEBAPI方式，内部通过模块抽象可方便扩展BAT、图灵机器人等REST API。**

### 1.使用说明
**硬件平台：** MINI2440开发板（同样可支持树莓派和LINUX虚拟机，但需要编译相应库）<br>
**软件平台：** LINUX<br>
**其它外设：** 板上麦克风（拾音距离近）或USB声卡（拾音距离远，但回声消除目前不支持，会一直触发语音识别），3.5mm接口音箱（最好可接电源，不然还需要定时充电）<br>
**目前支持的服务：** 询问时间、询问天气、播放新闻（MP3格式）、播放相声（M4A格式）<br>
**涉及知识点：** 线程间通信、线程间同步、HTTP、SOCKET、libcurl、libcjson、libmad、libfaad、WAV、PCM、MP3、MPEG-4标准、ALSA、webrtc-vad、uboot、kernel、rootfs


### 2.详细开发流程记录
[#MINI2440实现语音识别# （一）整体概述和实现流程记录](https://blog.csdn.net/sinat_26551021/article/details/79476056)
<br>[#MINI2440实现语音识别# （二）驱动声卡UDA1341遇到的问题和解决办法](https://blog.csdn.net/sinat_26551021/article/details/79484042)
<br>[#MINI2440实现语音识别# （三）REST API访问和VAD端点检测](https://blog.csdn.net/sinat_26551021/article/details/79602843)

### 3.原理介绍
目前程序由六个线程组成，包括录音检测线程（detect.c）、语义识别线程(semantic.c)、语音合成线程（tts.c）、新闻服务线程<br>（service_news.c）、相声服务线程（service_crosstalk.c）、音频播放线程（playback.c）。线程间通过消息队列、链表进行数据传递，采用互斥锁、条件变量实现线程间同步。程序流程图：

![](https://github.com/woxiaoniu/README-PICTURE/blob/master/sillyecho%E6%B5%81%E7%A8%8B.png)

### 4.联系方式

欢迎各位大佬来指导、交流和讨论。<br>

E-mail: zengchao@cqu.edu.cn<br>
