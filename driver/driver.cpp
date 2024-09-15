#include <include.hpp>

auto entry(void* a1, void* a2) -> NTSTATUS
{
	const auto win32k = modules::get_module("win32k.sys");
	if (!win32k)
	{
		printf("Failed to get win32k.sys module\n");
		return false;
	}

	if (!hook::setup(win32k, hook::hooked))
	{
		return STATUS_UNSUCCESSFUL;
	}
	
	return STATUS_SUCCESS;
}