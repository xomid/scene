#include <iostream>
#include <oui_window_manager.h>
#include "container.h"

int main() {
	if (OUI::init()) return 0;
	WindowManager wmg;
	Rect rc(100, 100, 700, 600);
	wmg.append(L"foo", new UIContainer(), &rc);
	/*if (argc > 1) {
		if (!wcscmp(argv[1], L"--inspect")) {
			auto wnd = wmg.get(0);
			if (wnd) wnd->bDrawBoxModel = true;
		}
	}*/

	return wmg.run();
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	//int argc = 0;
	//auto winArgv = GetCommandLineW();
	//const auto argv = CommandLineToArgvW(winArgv, &argc);
	return main();
}
