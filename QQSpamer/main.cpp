#include<string>
#include<vector>
#include<fstream>

#include<process.h>
#include<Windows.h>
#include<WinUser.h>

using namespace std;

#define LoadTXT 1
#define Help 3

DWORD interval;

wchar_t text[100];
BOOL isDown = false;
HHOOK myhook;
DWORD vk_code;

HMENU hMenu;
HWND windowTitle;
HWND windowInterval;
HWND windowHandle = 0;

vector<string> contents;

void SendMsg(HWND hWnd, const CHAR* msg)
{
	UINT _length = strlen(msg);
	for (UINT i = 0; i < _length; i++)
	{
		if (msg[i] < 0)
		{
			WPARAM wChar = (BYTE)msg[i];
			wChar <<= 8;
			wChar |= (BYTE)msg[++i];
			SendMessageA(hWnd, WM_IME_CHAR, wChar, NULL);
		}
		else
		{
			WPARAM wChar = (BYTE)msg[i];
			SendMessageA(hWnd, WM_IME_CHAR, wChar, NULL);
		}
	}
	Sleep(10);
}

void Begin(void* p)
{
	while (isDown)
	{
		for (unsigned int i = 0; i < contents.size(); i++)
		{
			SendMsg(windowHandle, contents[i].c_str());
			SendMessage(windowHandle, WM_KEYDOWN, VK_RETURN, 0);
			Sleep(10);
			SendMessage(windowHandle, WM_KEYUP, VK_RETURN, 0);
		}
		Sleep(interval);
	}
	_endthread();
}

string WcharToChar(const wchar_t* wp, size_t m_encode = CP_ACP)
{
	string str;
	int len = WideCharToMultiByte(m_encode, 0, wp, wcslen(wp), NULL, 0, NULL, NULL);
	char* m_char = new char[len + 1];
	WideCharToMultiByte(m_encode, 0, wp, wcslen(wp), m_char, len, NULL, NULL);
	m_char[len] = '\0';
	str = m_char;
	delete[] m_char;
	m_char = NULL;
	return str;
}

void AddMenus(HWND hWnd)
{
	hMenu = CreateMenu();
	AppendMenuW(hMenu, MF_STRING, Help, L"帮助");
	SetMenu(hWnd, hMenu);
}

void AddControls(HWND hWnd)
{
	CreateWindowW(L"static", L"窗口标题:", WS_VISIBLE | WS_CHILD, 20, 20, 110, 20, hWnd, NULL, NULL, NULL);
	windowTitle = CreateWindowW(L"edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 100, 20, 100, 20, hWnd, NULL, NULL, NULL);
	CreateWindowW(L"static", L"刷屏间隔(ms):", WS_VISIBLE | WS_CHILD, 105, 60, 100, 40, hWnd, NULL, NULL, NULL);
	windowInterval = CreateWindowW(L"edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 200, 60, 70, 20, hWnd, NULL, NULL, NULL);
	CreateWindowW(L"button", L"读取txt", WS_VISIBLE | WS_CHILD, 40, 50, 55, 40, hWnd, (HMENU)LoadTXT, NULL, NULL);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	string temp;
	ifstream file;
	switch (msg)
	{
	case WM_COMMAND:
		switch (wParam)
		{
		case LoadTXT:
			file.open("config.txt");
			if (!file.is_open())
			{
				MessageBoxW(NULL, L"错误:没在目录下找到config.txt", L"ERROR", MB_OK);
				return 0;
			}
			contents.clear();
			while (!file.eof())
			{
				file >> temp;
				contents.push_back(temp);
			}
			file.close();
			MessageBoxW(NULL, L"读取成功", L"消息", MB_OK);
			break;
		case Help:
			MessageBoxW(NULL, L"请在EXE目录下创建编码为ANSI名为config.txt的文件\n\t按HOME键开始再按一次结束", L"消息", MB_OK);
			break;
		default:
			break;
		}
		break;
	case WM_CREATE:
		AddMenus(hWnd);
		AddControls(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CLOSE:
		UnhookWindowsHookEx(myhook);
		DestroyWindow(hWnd);
		break;
	default:
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}
}

LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= HC_ACTION && wParam == WM_KEYDOWN)
	{
		vk_code = ((KBDLLHOOKSTRUCT*)lParam)->vkCode;

		if (vk_code == VK_HOME)
		{
			isDown = !isDown;
			if (isDown)
			{
				GetWindowText(windowTitle, text, 100);
				windowHandle = FindWindowW(NULL, text);
				if (windowHandle == NULL)
				{
					MessageBoxA(NULL, "没找到窗口", "ERROR", MB_OK);
					isDown = !isDown;
					return 0;
				}
				GetWindowText(windowInterval, text, 100);
				interval = atoi(WcharToChar(text).c_str());
				SetForegroundWindow(windowHandle);
				MoveWindow(windowHandle, 500, 500, 634, 505, 0);
				POINT pos;
				pos.x = 25;
				pos.y = 423;
				ClientToScreen(windowHandle, &pos);
				SetCursorPos(pos.x, pos.y);
				SendMessage(windowHandle, MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0);
				Sleep(20);
				_beginthread(Begin, 0, NULL);
			}
		}
	}
	return CallNextHookEx(myhook, code, wParam, lParam);
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR lpCmdLine, INT nCmdShow)
{
	WNDCLASSW wc = { 0 };
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hInstance = hInstance;
	wc.lpszClassName = L"MyWindowClass";
	wc.lpfnWndProc = WndProc;
	if (!RegisterClassW(&wc)) { return -1; }
	CreateWindowW(L"MyWindowClass", L"QQ刷屏", WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_THICKFRAME | WS_VISIBLE, 100, 100, 300, 175, NULL, NULL, NULL, NULL);
	MSG msg;
	myhook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)&KeyboardProc, GetModuleHandle(NULL), NULL);
	while (GetMessage(&msg, NULL, NULL, NULL) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}