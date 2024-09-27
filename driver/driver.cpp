#include <include.hpp>

NTSTATUS entry(void* a1, void* a2)
{
	const auto win32k = modules::get_module("win32k.sys");
	if (!win32k)
	{
		printf("Failed to get win32k.sys module\n");
		return STATUS_UNSUCCESSFUL;
	}

	if (!mouse::open())
	{
		printf("Failed to open mouse\n");
		return STATUS_UNSUCCESSFUL;
	}

	if (!hook::setup(win32k, hook::hooked))
	{
		printf("Failed to setup hook\n");
		return STATUS_UNSUCCESSFUL;
	}
	
	return STATUS_SUCCESS;
}