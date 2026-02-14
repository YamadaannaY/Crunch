
#include "DebugHelper.h"
/*
1️⃣日志和调试
参数	作用
-log	打开控制台日志输出
-FullStdOutLogOutput	将日志输出到标准输出（方便 IDE 捕获）
-verbose	显示详细日志信息
-NoSplash	跳过启动画面，提高调试启动速度
-CrashForUAT	强制崩溃日志用于打包测试
2️⃣ 游戏运行模式
参数	作用
-game	以游戏模式启动，而不是编辑器模式
-editor	以编辑器模式启动
-windowed	窗口模式启动
-fullscreen	全屏模式启动
-ResX=1920 -ResY=1080	指定分辨率
-NoSound	禁用音频
-NoTextureStreaming	禁用纹理流，提高调试性能
-dx12 / -d3d12	使用 DX12 渲染器
-vulkan	使用 Vulkan 渲染器（Linux / Win 支持）
3️⃣ 地图和关卡
参数	作用
-map /Game/Levels/MyLevel	直接加载指定地图
-StartWithEditor	从指定地图直接进入编辑器视角
-UseDebugCamera	使用调试相机（飞行模式）
4️⃣ 性能和调试工具
参数	作用
-stat fps	显示 FPS 统计
-stat unit	显示 CPU/GPU/Frame 时间统计
-stat memory	显示内存统计
-benchmark	基准测试模式，自动运行
-nomovie	跳过启动动画和电影片段
-NoTextureStreaming	禁用纹理流，适合性能测试
5️⃣ 网络和多人模式
参数	作用
-log	打印网络调试信息
-MaxPlayers=4	设置最大玩家数（多人测试）
-port=7777	指定游戏服务器端口
-server	启动服务器实例
-listen	启动可接受客户端连接的 listen 服务器

 */