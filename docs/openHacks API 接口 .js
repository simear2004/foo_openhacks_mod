var UIHacks;
var uiHacks = utils.CheckComponent("foo_openhacks_mod");
if (uiHacks) UIHacks = new ActiveXObject("OpenHacksMod");

//OpenHacks 功能设置
function uiHacksSet() {
	if (!uiHacks) return;
	UIHacks.MenuBarVisible = false;                  //显示菜单栏
	UIHacks.StatusBarVisible = false;                //显示状态栏
	//UIHacks.Fullscreen = false;                    //启用全屏
	//UIHacks.WindowState = 0;                       //0 = 默认，1 = 最小化，2 = 最大化
	UIHacks.WindowFrameStyle = 2;                    //0 = 默认，1 = 无标题栏，2 = 无边框

	UIHacks.EnableWin10Shadow = true;                //win10下启用窗口阴影，活动窗口时会产生边框
	UIHacks.DisableResizeWhenMaximized = true;       //最大化窗口时禁用窗口大小调整
	UIHacks.DisableResizeWhenFullscreen = true;      //全屏时禁用窗口大小调整
}

//OpenHacks 仿真标题栏设置
function uiHacksCaption() {
	if (!uiHacks) return;
	UIHacks.PseudoCaptionTop = 0; // 请修改为你需要的值或变量
	UIHacks.PseudoCaptionTopEnabled = true; // 是否启用

	UIHacks.PseudoCaptionLeft = iw*2; // 请修改为你需要的值或变量
	UIHacks.PseudoCaptionLeftEnabled = true; // 是否启用

	UIHacks.PseudoCaptionRight = 0; // 请修改为你需要的值或变量
	UIHacks.PseudoCaptionRightEnabled = false; // 是否启用

	UIHacks.PseudoCaptionBottom = 0; // 请修改为你需要的值或变量
	UIHacks.PseudoCaptionBottomEnabled = false; // 是否启用

	// 设置伪标题栏区域大小
	UIHacks.PseudoCaptionWidth = ww-iw*14; // 请修改为你需要的值或变量
	UIHacks.PseudoCaptionHeight = tith; // 请修改为你需要的值或变量
}

//OpenHacks 切换方法，在需要的地方调用
function toggleMenuBar() {
	if (!uiHacks) return;
    UIHacks.ToggleMenuBar();    // 切换菜单栏显示/隐藏
}

function toggleStatusBar() {
	if (!uiHacks) return;
    UIHacks.ToggleStatusBar();  // 切换状态栏显示/隐藏
}

function toggleFullscreen() {
	if (!uiHacks) return;
    UIHacks.ToggleFullscreen(); // 切换全屏
}
