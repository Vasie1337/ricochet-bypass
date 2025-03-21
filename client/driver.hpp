#pragma once
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

	static void move_mouse(int x, int y)
	{
		set_mouse(x, y, MOVE_RELATIVE, 0);
	}

	static void read(void* dst, void* src, size_t size)
	{
		_comm_data data = { 0 };
		data.type = _comm_type::read;
		data.size = size;
		data.dst_address = dst;
		data.src_address = src;
		send_request(&data);
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

	template <typename T> static T read(uint64_t src);
	template <typename T> static void write(uint64_t dst, T value);

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
	static void set_mouse(int x, int y, unsigned short flags, unsigned short button_flags)
	{
		_mouse_data mouse_data = {};
		mouse_data.x = x;
		mouse_data.y = y;
		mouse_data.flags = flags;
		mouse_data.button_flags = button_flags;
		_comm_data data = { 0 };
		data.type = _comm_type::mouse;
		data.mouse_data = mouse_data;
		send_request(&data);
	}
private:
	typedef __int64(__fastcall* thandler)(void* a1);
	static inline thandler handlerobj;

	static inline uint64_t target_proc;
	static inline uint64_t target_base;
	static inline uint64_t target_cr3;
	static inline uint64_t target_peb;
	static inline HANDLE target_pid;

	// Mouse flags

	const static int MOVE_RELATIVE = 0;
	const static int MOVE_ABSOLUTE = 1;
	 
	const static int LEFT_BUTTON_DOWN = 0x0001;
	const static int LEFT_BUTTON_UP = 0x0002;
	const static int RIGHT_BUTTON_DOWN = 0x0004;
	const static int RIGHT_BUTTON_UP = 0x0008;
	const static int MIDDLE_BUTTON_DOWN = 0x0010;
	const static int MIDDLE_BUTTON_UP = 0x0020;
};

template<typename T>
T drv::read(uint64_t src)
{
	T value;
	read(&value, reinterpret_cast<void*>(src), sizeof(T));
	return value;
}

template<typename T>
void drv::write(uint64_t dst, T value)
{
	write(reinterpret_cast<void*>(dst), &value, sizeof(T));
}