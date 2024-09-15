#include <../.shared/shared.hpp>

typedef __int64(__fastcall* Handler_type)(__int64 a1);

Handler_type get_handler()
{
	LoadLibraryW(L"user32.dll");

	HMODULE win32u = LoadLibraryW(L"win32u.dll");
	if (!win32u) {
		printf("Failed to get win32u.dll handle");
		return nullptr;
	}

	uint64_t handler_address = (uint64_t)GetProcAddress(win32u, "NtUserGetWindowFeedbackSetting");
	if (!handler_address) {
		printf("Failed to get handler address");
		return nullptr;
	}

	return (Handler_type)handler_address;
}

int main() 
{
	Handler_type handler = get_handler();
	if (!handler) 
	{
		return 1;
	}

	handler(0x1337);

	return 0;
}