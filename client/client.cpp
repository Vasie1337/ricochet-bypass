#include <Windows.h>
#include <iostream>
#include <thread>

#include <../.shared/shared.hpp>

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

		return true;
	}

	static auto proc() -> uint64_t { return target_proc; }
	static auto base() -> uint64_t { return target_base; }
	static auto cr3() -> uint64_t { return target_cr3; }
	static auto peb() -> uint64_t { return target_peb; }

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
};

int main() 
{
	if (!drv::init_handler("cod.exe"))
	{
		printf("Failed to init handler\n");
		return 1;
	}

	uint64_t proc = drv::proc();
	uint64_t base = drv::base();
	uint64_t cr3 = drv::cr3();
	uint64_t peb = drv::peb();

	printf("PROC: %llx\n", proc);
	printf("BASE: %llx\n", base);
	printf("CR3: %llx\n", cr3);
	printf("PEB: %llx\n", peb);
	printf("\n");

	int gamemode = drv::read<int>(base + 0x11A693B0);
	printf("gamemode: %d\n", gamemode);

	return 0;
}