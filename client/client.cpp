#include <include.hpp>

class drv
{
public:
	static bool init_handler(std::string proc_name)
	{
		if (!init())
		{
			printf("Failed to get handler\n");
			return false;
		}

		target_proc = get_proc(proc_name.c_str());
		if (!target_proc)
		{
			printf("Failed to get target process\n");
			return false;
		}

		target_base = get_base(proc_name.c_str());
		if (!target_base)
		{
			printf("Failed to get target base\n");
			return false;
		}

		target_cr3 = get_cr3();
		if (!target_cr3)
		{
			printf("Failed to get target cr3\n");
			return false;
		}

		target_peb = get_peb();
		if (!target_peb)
		{
			printf("Failed to get target peb\n");
			return false;
		}

		target_pid = get_pid();
		if (!target_pid)
		{
			printf("Failed to get target pid\n");
			return false;
		}

		return true;
	}

	static auto proc() -> uint64_t { return target_proc; }
	static auto base() -> uint64_t { return target_base; }
	static auto cr3() -> uint64_t { return target_cr3; }
	static auto peb() -> uint64_t { return target_peb; }
	static auto pid() -> HANDLE { return target_pid; }

	static void read(void* dst, void* src, size_t size)
	{
		_comm_data data = { 0 };
		data.type = _comm_type::read;
		data.size = size;
		data.dst_address = dst;
		data.src_address = src;
		send_request(&data);
	}

	template <typename T>
	static T read(uint64_t src)
	{
		T buffer = { 0 };
		read(&buffer, reinterpret_cast<void*>(src), sizeof(T));
		return buffer;
	}

	static void write(void* dst, void* src, size_t size)
	{
		_comm_data data = { 0 };
		data.type = _comm_type::write;
		data.size = size;
		data.dst_address = dst;
		data.src_address = src;
		send_request(&data);
	}
	
	template <typename T>
	static void write(uint64_t dst, T value)
	{
		write(reinterpret_cast<void*>(dst), &value, sizeof(T));
	}

protected:
	static bool init()
	{
		LoadLibraryA("user32.dll");

		auto win32u = LoadLibraryA("win32u.dll");
		if (!win32u) {
			printf("Failed to get win32u.dll handle");
			return false;
		}

		handlerobj = reinterpret_cast<thandler>(GetProcAddress(win32u, "NtUserGetWindowFeedbackSetting"));
		if (!handlerobj) {
			printf("Failed to get handler");
			return false;
		}

		return true;
	}
	static void send_request(_comm_data* data)
	{
		data->magic = 0x1337;
		data->target_proc = reinterpret_cast<void*>(target_proc);
		xor_comm_data(data);
		handlerobj(data);
	}
	static uint64_t get_cr3()
	{
		uint64_t cr3 = 0;
		_comm_data data = { 0 };
		data.type = _comm_type::cr3;
		data.src_address = &cr3;
		send_request(&data);
		return cr3;
	}
	static uint64_t get_base(const char* mod_name)
	{
		uint64_t base = 0;
		_comm_data data = { 0 };
		data.type = _comm_type::base;
		memcpy(data.str_buffer, mod_name, strlen(mod_name));
		data.src_address = &base;
		send_request(&data);
		return base;
	}
	static uint64_t get_peb()
	{
		uint64_t peb = 0;
		_comm_data data = { 0 };
		data.type = _comm_type::peb;
		data.src_address = &peb;
		send_request(&data);
		return peb;
	}
	static HANDLE get_pid()
	{
		HANDLE pid = 0;
		_comm_data data = { 0 };
		data.type = _comm_type::pid;
		data.src_address = &pid;
		send_request(&data);
		return pid;
	}
	static uint64_t get_proc(const char* proc_name)
	{
		uint64_t proc = 0;
		_comm_data data = { 0 };
		data.type = _comm_type::proc;
		memcpy(data.str_buffer, proc_name, strlen(proc_name));
		data.src_address = &proc;
		send_request(&data);
		return proc;
	}
private:
	typedef __int64(__fastcall* thandler)(void* a1);
	static inline thandler handlerobj;

	static inline uint64_t target_proc;
	static inline uint64_t target_base;
	static inline uint64_t target_cr3;
	static inline uint64_t target_peb;
	static inline HANDLE target_pid;
};

class overlay
{
private:
	static inline HWND hwnd;
	static inline HINSTANCE hInstance;

public:
	static bool create(HINSTANCE hInst)
	{
		hInstance = hInst;

		WNDCLASSEX wc = { 0 };
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.lpfnWndProc = WndProc;
		wc.hInstance = hInstance;
		wc.lpszClassName = L"OverlayWindow";
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

		if (!RegisterClassEx(&wc))
			return false;

		hwnd = CreateWindowExW(
			WS_EX_TOPMOST,
			wc.lpszClassName, L"Overlay", 
			WS_POPUP, 
			0, 0,
			GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 
			nullptr, nullptr, hInstance, nullptr);

		if (!hwnd)
			return false;

		SetWindowLong(hwnd, GWL_EXSTYLE, (int)GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT);


		SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, ULW_COLORKEY);
		SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);

		ShowWindow(hwnd, SW_SHOW);
		UpdateWindow(hwnd);

		return true;
	}

	static void render()
	{

	}

	static void destroy()
	{
		if (hwnd)
		{
			DestroyWindow(hwnd);
			hwnd = nullptr;
		}
	}

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			hdc = BeginPaint(hwnd, &ps);

			SetTextColor(hdc, RGB(0, 255, 0));
			SetBkMode(hdc, TRANSPARENT);

			RECT rect;
			GetClientRect(hwnd, &rect);

			uint64_t base = drv::base();
			short mz = drv::read<short>(base);
			std::string text = "MZ: " + std::to_string(mz);
			DrawTextA(hdc, text.c_str(), -1, &rect, DT_CENTER | DT_TOP | DT_SINGLELINE);

			EndPaint(hwnd, &ps);
			break;
		}
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		return 0;
	}
};

int main() 
{
	if (!drv::init_handler("notepad.exe"))
	{
		printf("Failed to init handler\n");
		return 1;
	}
	
	uint64_t proc = drv::proc();
	uint64_t base = drv::base();
	uint64_t cr3 = drv::cr3();
	uint64_t peb = drv::peb();
	HANDLE pid = drv::pid();
	
	printf("PROC: %llx\n", proc);
	printf("BASE: %llx\n", base);
	printf("CR3: %llx\n", cr3);
	printf("PEB: %llx\n", peb);
	printf("PID: %x\n", pid);
	printf("\n");

	std::thread([&]()
	{
		while (true)
		{
			uint64_t base = drv::base();
			short mz = drv::read<short>(base);
			printf("MZ: %x\n", mz);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}).detach();

	overlay::create(GetModuleHandleA(0));

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	overlay::destroy();

	return 0;
}