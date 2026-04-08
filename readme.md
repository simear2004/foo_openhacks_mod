### Introduction
OpenHacks is an enhancement component for the foobar2000 player.

Requirements:
- Windows 10 1607+
- foobar2000 v1.5+
- foobar2000 v2.0+ x86/x64 

### Features
- Hide DUI menu bar
- Hide DUI status bar
- Draggable and resizable borderless main window
- ...

Mod 版的功能：  
1、新增：3个API接口，默认为true：  
     EnableWin10Shadow = true;                   //win10下启用窗口阴影，活动窗口时会产生边框(可js脚本消除处理)  
     DisableResizeWhenMaximized = true;     //最大化窗口时禁用窗口大小调整  
     DisableResizeWhenFullscreen = true;      //全屏时禁用窗口大小调整  
2、新增：双击伪标题栏最大化，反之还原窗口  
3、新增：全屏时双击伪标题栏退出全屏  
4、新增：最大化或全屏时禁止伪标题栏区域的拖动功能  
5、新增：拖动窗口边缘的滚动条过程中，如果鼠标离开窗口范围仍然执行拖动操作  
6、修改：调整窗口大小 sizing 的响应距离，以免与窄条滚动条重叠导致混乱的情况  
7、修改：组件以 Mod 版本发布，以区别 ohyeah 大佬的原版

感谢：
ohyeah(原版作者)  
dreamawake(foobox主题作者测试反馈)

Mod version features:
1. Added: 3 API interfaces, default is true:
     EnableWin10Shadow = true; //Enable window shadow under win10, and a border will be generated when the window is active (can be eliminated by js script)
     DisableResizeWhenMaximized = true; //Disable window resizing when maximizing the window
     DisableResizeWhenFullscreen = true; //Disable window resizing when full screen
2. New: double-click the pseudo title bar to maximize it, otherwise restore the window
3. New: Double-click the pseudo title bar to exit full screen when in full screen mode.
4. New: Disable the dragging function of the pseudo title bar area when maximizing or full screen
5. New: During the process of dragging the scroll bar on the edge of the window, if the mouse leaves the window range, the drag operation will still be performed.
6. Modification: Adjust the response distance of window size sizing to avoid confusion caused by overlapping with narrow scroll bars.
7. Modification: The component is released in Mod version to distinguish it from the original version of ohyeah boss
Thanks to:
ohyeah (original author)
dreamawake (foobox theme author test feedback)

### Build
Visual Studio 2022


