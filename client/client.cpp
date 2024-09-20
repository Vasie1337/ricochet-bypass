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

		return true;
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
	static auto get_cr3() -> uint64_t
	{
		uint64_t cr3 = 0;
		_comm_data data = { 0 };
		data.type = _comm_type::cr3;
		data.src_address = &cr3;
		send_request(&data);
		return cr3;
	}
	static auto get_base() -> uint64_t
	{
		uint64_t base = 0;
		_comm_data data = { 0 };
		data.type = _comm_type::base;
		data.src_address = &base;
		send_request(&data);
		return base;
	}
	static auto get_peb() -> uint64_t
	{
		uint64_t peb = 0;
		_comm_data data = { 0 };
		data.type = _comm_type::peb;
		data.src_address = &peb;
		send_request(&data);
		return peb;
	}
	static auto get_proc(const char* proc_name) -> uint64_t
	{
		uint64_t proc = 0;
		_comm_data data = { 0 };
		data.type = _comm_type::proc;
		memcpy(data.str_buffer, proc_name, strlen(proc_name));
		data.src_address = &proc;
		send_request(&data);
		return proc;
	}
	template <typename T>
	static T read(uint64_t src)
	{
		T buffer = { 0 };
		read(&buffer, reinterpret_cast<void*>(src), sizeof(T));
		return buffer;
	}
	template <typename T>
	static void write(uint64_t dst, T value)
	{
		write(reinterpret_cast<void*>(dst), &value, sizeof(T));
	}
protected:
	typedef __int64(__fastcall* thandler)(void* a1);
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
private:
	
	static inline thandler handlerobj;
	static inline uint64_t target_proc;
};

int main() 
{
	if (!drv::init_handler("notepad.exe"))
	{
		printf("Failed to init handler\n");
		return 0;
	}

	uint64_t base = drv::get_base();
	uint64_t cr3 = drv::get_cr3();
	uint64_t peb = drv::get_peb();

	printf("BASE: %llx\n", base);
	printf("CR3: %llx\n", cr3);
	printf("PEB: %llx\n", peb);

	while (true)
	{
		short test = drv::read<short>(base);
		printf("test: %lx\n", test);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return 0;
}