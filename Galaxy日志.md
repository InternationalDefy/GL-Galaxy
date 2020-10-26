11.18 上午
- 10:55 开始阅读文章 https://beltoforion.de/article.php?a=spiral_galaxy_renderer
- 11:03 密度波理论：解释银河中心快，四周慢的牵引运动。
- 11:07 星系分类，本文关注于Sa->Sc
- 11:18 大致了解Sa的Model和近似参量关系
- 11:19 饿了，吃饭去~
11.18 下午
- 12:45 吃完饭，继续
- 12:49 两种理论提出了一个简化的光密度公式
- 12:52 色调图的确认。普朗克辐射定律：黑体辐射，黑体表面温度和其辐射光色调的关系。罗素图：黑体运动时表面温度变化规律。颜色计算是一个复杂问题，拟使用specrend.c输入一个开尔文返回一个RGB
- 12:58 点光源的基础上可能需要加入星云材质？
- 12:59 H-II区的处理：当作一个偏红高光的星云，运动方式类似于其他星
- 13:05 星体运动处理。
- 13:06 文章阅读完成。
形态：光密度公式 色调：普朗克定律 背景：星云图 运动：运动模型 额外：H-II区
- 14:10 听歌，看B站，玩谷歌地球，摸鱼一个小时歌
- 14:41 设计思路
    - 类
        - 数据类 c dataComponent
            - cGalaxy
            - cStar
            - cDust
            - cH2
        - 管理类 s System
            - sGalaxyRender
            - sGalaxyInitializer
            - sGalaxyStep
        - 计算函数 f calculationFunctioN
            - fnGalaxyMovement
            - fnColorFromTemperature
            - fnOrbitPosition
    - 结构
        * wnd ->c->s
        * s ->c->f
        * c ->c
        * f ->f
    - 实现
        * 窗口类初始化
        * 初始化cGalaxy
        * 初始化OpenGL窗口
        * 绘制循环
            - PullEvents ESC->Exit
            - Render
                - openglpreset
                - drawdust
                - drawstars
                - drawh2
                - rendercall
                - openglpostset
            - Step
    - 算法
        * 参考文章与公式。
- 14:51 新建OpenGL项目
- 14:55 删除MouseCallback，删除键盘输入Camera事件，类似的事件不需要发送，对Camera类不予以修改，保留其处理鼠标事件的的能力
- 15:07 目前不需要Texture和Shader的函数，但其可以保留
- 15:12 空项目可用，现在添加新类
- 15:47 Galaxy和Star类的塑模，修改了初始化构造函数，用vector取代指针，删去直方图
- 15:51 ToggleDarkMatter的处理：恒为真，不处理其Toggle函数
- 16:00 建立galaxy_defines，里处理常量和单位转换
- 16:12 发现一个问题：cGalaxy里的init函数不应该拥有这么多内容，考虑到其逻辑设计上应该由initializer实现
- 16:24 TimeStep的逻辑设计上由Step类实现，但是考虑到实际上time关联的不是绝对时间，而是程序运行开始计算的类存在时间，也可以由cGalaxy类管理。
- 16:32 是否需要StarPos的Getter和旋转核，这个对于固定观察点没意义，而且即使需要改变观察点，也不倾向于把此逻辑和Component耦合。
- 16:33 在galaxy_functions中添加OrbitCalculator
- 17:10 创建类TimeStepTicker处理TimeStep的问题，现在Galaxy中的SingleStep是无用函数
- 17:28 塑模接近完成，还需initializer结合实现初始化逻辑。
- 17:32 Reset是无用的操作可以删除的函数。但是考虑到其中耦合了初始化的逻辑，所以进行解耦。
- 17:48 改设计之后Galaxy的构造和初始化耦合了，给自己加个限制吧。。。
- 18:00 initializer待实现，先去吃晚饭~
11.18 晚上
- 19:15 吃完回来，继续~
- 19:57 塑模完成，现对塑模测试。
- 20:04 出现头文件重定义问题，把galaxy_components.h分两个文件。编译通过
- 20:07 断点测试，姑且认为塑模是没有问题的。
- 20:11 今天到此为止，玩儿游戏~
- 20:37 不是很想玩游戏，继续好了~
- 20:41 Renderer的流程
    - Color和Position的初始化
    - Matrix的配置和RendererCall
    - Color和Position的更新
- 21:07 对Container类进行塑模，它需要Render所需要的所有数据
默认RenderType为1 CameraOrbient为0 建立颜色管理的数据 
- 21:24 渲染还需要Camera的管理，和GL_POINT_SPRITE的拓展，先去洗澡，之后继续。
- 21:54 洗完澡了，继续。
- Render的逻辑实现于Wnd类还是Renderer里呢？Renderer类设计上只负责Render，类似于OpenGLSetup应该设置于Wnd类中
- 21:01 先考虑OpenGL初始化语句的集合。需要使用到POINT_SPRITE模式，尝试用google
- 22:32 glad版本和gl.h版本有所冲突，关于绘制模块，这个算法给出的东西都太老了，自学点精灵准备从头写起绘制模块。今日总结
    - 完成了数据塑模和程序结构
    - 尚缺：
        - 颜色配置
        - 绘制模块
        - 更新模块
- 22:54 睡前总结一下模块任务
    - Init阶段
        - OpenGL初始化
            - ShadeModel（？旧特性）
            - InitPointSpriteExtension
                - 设置点模式
                - texture的读入和初始化
            - 打开Blend和LineSmooth
        - Camera的初始化和设置 -> 平行相机 参数待定
        - glLoadIdentity（？旧特性）
    - RenderLoop阶段
        - flags的处理（不用管）
        - 绘制函数可以参考！
        - deltaTime的处理
11.19上午
- 9:17 建立新类GalaxyRenderer为system组件，建立新类ColorFunction处理颜色
- 38 调库specrend，出现问题glm和specrend参数类型不对接，float* 到double* 的向上转义几乎必然导致未定义行为。
- 40 将此Function另写一个float版本？
- 56 颜色函数重写完成，考虑到col实际上是col数组的引用，这里打个断点，测试。
- 03 Color的初始化暂时没有问题。
- 04 开始考虑最后一步，绘制的问题，需要学习新特性下point sprite的使用
- 15 POINT_SPRITE跨GLAD直接Enable不会立刻崩溃，
- 37 学习了一段时间，发现POINT_SPRITE是弃用特性，改用GLSL POINT_SIZE https://www.youtube.com/watch?v=73XoxNQ4WNc
- 50 把Texture和main函数解耦，先读入Texture
- 01 是时候把main函数和除了窗口函数外的所有东西解耦了
- 06 Camera Color Shader ?
shader移至renderer中处理
camera似乎没必要，目前先注释掉？
- 25 Shaderclass 和 camera不是固定于何处的问题，对于renderer而言，需要用则添加它即可
- 26 吃饭
11.19 下午
- 13:36 继续
- 13:56 进行了一些简单的学习，添加PointShader
- 14:21 思路：在循环里把VBO填上，出来调RenderCall，Dust，Star，H2使用不同的Shader
- 15:19 摸了一个小时🐟，因为实在很困。。。
- 15:20 GLFW Window可用，使用DynamicDraw处理Point
- 16:20 PointSprite的渲染OK！参考文章 https://stackoverflow.com/questions/17397724/point-sprites-for-particle-system/17400234
- 17:19 pos 初始值有问题 变长数组也有问题
- 17:37 咬牙切齿地完成了最后的调参。没心情写日志了。
- 18:03 模型有错误。错误出且只出在StartingPosition
    1. Vel和Pos有问题
    2. Vel和Pos是什么？
    - vel无用，可以剔除
    - posold也是废项，可以剔除。
    - 剔除，问题2解决
    3. 初始生产函数？
    4. 直角坐标系到视口坐标系状态转换。
- 18:16 Step Last一个初状态生成函数。
- 18:32 原模型工具集有问题
- 19:27 下载140工具集，准备重试。
- 20:04 原模型无论如何用不成了，居然使用上古工具和近代工具集，无疑和现代编程背道而驰。自己研究天文学然后准备写分布函数吧。
- 20:31 建模无力，天文苦手。。。准备用随机函数
- 20:47 C++11浮点随机数发生器，简单暴力
- 21:22 随机数+参数调整，用程序员的方法取代了天文模型，对不起历代物理学家了。。。
11.20 早上
- 7:59 继续
- 10:03 随机化建模+调参
- 10:27 随机化DustSize
- 10:28 有bug 边缘残留
- 11:13 Finish