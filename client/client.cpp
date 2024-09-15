#include <../.shared/shared.hpp>
#include <Windows.h>
#include <iostream>



class handler
{
public:
	handler(unsigned int target_pid) : target_pid(target_pid)
	{
		if (!get())
		{
			printf("Failed to get handler\n");
			return;
		}
	}
	~handler() = default;
protected:
	typedef __int64(__fastcall* thandler)(void* a1);
	bool get()
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
	void send_request(_comm_data* data)
	{
		data->magic = 0x1337;
		data->target_pid = target_pid;
		handlerobj(data);
	}
private:
	thandler handlerobj;
	unsigned int target_pid;
};

class driver_handler : public handler
{
public:
	driver_handler(unsigned int target_pid) : handler(target_pid) {}
	~driver_handler() = default;

	void read(void* dst, void* src, size_t size)
	{
		_comm_data data;
		data.type = _comm_type::read;
		data.size = size;
		data.dst_address = reinterpret_cast<unsigned __int64>(dst);
		data.src_address = reinterpret_cast<unsigned __int64>(src);
		send_request(&data);
	}

	void write(void* dst, void* src, size_t size)
	{
		_comm_data data;
		data.type = _comm_type::write;
		data.size = size;
		data.dst_address = reinterpret_cast<unsigned __int64>(dst);
		data.src_address = reinterpret_cast<unsigned __int64>(src);
		send_request(&data);
	}


};

int main() 
{

	return 0;
}