#include <../.shared/shared.hpp>
#include <Windows.h>
#include <iostream>

class drv
{
public:
	static bool init_handler(unsigned int pid)
	{
		target_pid = pid;

		if (!get())
		{
			printf("Failed to get handler\n");
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
	static bool get()
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
		data->target_pid = target_pid;
		handlerobj(data);
	}
private:
	static inline thandler handlerobj;
	static inline unsigned int target_pid;
};

int main() 
{
	drv::init_handler(GetCurrentProcessId());

	uint64_t base = drv::get_base();
	uint64_t cr3 = drv::get_cr3();
	uint64_t peb = drv::get_peb();

	printf("Base: %p\n", base);
	printf("CR3: %p\n", cr3);
	printf("PEB: %p\n", peb);

	while (true)
	{
		auto test = drv::read<uint64_t>(base);

		printf("test: %llx\n", test);
		Sleep(1000);
	}

	return 0;
}