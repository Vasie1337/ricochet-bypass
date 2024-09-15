#include <include.hpp>

auto entry(void* a1, void* a2) -> NTSTATUS
{
	const auto win32k = modules::get_module("win32k.sys");
	if (!win32k)
	{
		printf("Failed to get win32k.sys module\n");
		return false;
	}

	const auto code_cave = codecave::find(win32k.base, win32k.base + win32k.size);
	if (!code_cave)
	{
		printf("Failed to find code cave\n");
		return false;
	}

	if (!codecave::setup(code_cave, hook::handler))
	{
		printf("Failed to setup code cave\n");
		return false;
	}

	if (!hook::setup(win32k, code_cave))
	{
		return STATUS_UNSUCCESSFUL;
	}
	
	return STATUS_SUCCESS;
}