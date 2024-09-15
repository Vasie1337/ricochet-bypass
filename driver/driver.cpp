#include <include.hpp>

static inline __int64(__fastcall* original_ptr)(__int64 a1);
__int64 hooked_ptr(__int64 a1)
{
	if (a1 != 0x1337)
		return original_ptr(a1);

	printf("Hooked that nigga: %llx\n", a1);

	return 0;
}

bool init_data_ptr(modules::DATA_ENTRY module, const char* pattern, const char* mask, void* target)
{
	auto winlogon = modules::get_eprocess("winlogon.exe");
	if (!winlogon)
	{
		printf("Failed to get winlogon.exe process\n");
		return false;
	}

	KAPC_STATE apc = { 0 };
	KeStackAttachProcess(winlogon, &apc);

	const uint64 function = scanner::find_pattern(module.base, module.size, pattern, mask);
	if (!function)
	{
		printf("Failed to find pattern\n");
		return false;
	}

	printf("Function found at: %p\n", reinterpret_cast<void*>(function));

	int* displacement_ptr = (int*)(function + 7);
	uint64 target_address = function + 7 + 4 + *displacement_ptr;

	*(void**)&original_ptr = _InterlockedExchangePointer((void**)target_address, target);
	printf("Original function at: %p\n", original_ptr);
	printf("Target address: %p\n", target_address);

	KeUnstackDetachProcess(&apc);

	return true;
}

void* find_code_cave(uint64 start, uint64 end, size_t size)
{
	uint8* ptr = (uint8*)start;
	while (ptr < (uint8*)end)
	{
		if (ptr[0] == 0xCC)
		{
			uint8* code_cave = ptr;
			for (size_t i = 0; i < size; i++)
			{
				if (code_cave[i] != 0xCC)
				{
					code_cave = nullptr;
					break;
				}
			}

			if (code_cave)
			{
				return code_cave;
			}
		}

		ptr++;
	}

	return nullptr;
}

uint8 shellcode[] = { 
	0x48, 0x89, 0xC0, 
	0x90, 
	0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x90, 
	0xFF, 0xE0 
};

bool setup_code_cave(void* code_cave, void* target)
{
	*(void**)&shellcode[6] = target;

	auto cr0 = __readcr0();
	__writecr0(cr0 & ~(1 << 16));

	crt::memcpy(code_cave, shellcode, sizeof(shellcode));
	printf("Cave at: %p\n", code_cave);

	__writecr0(cr0);

	return true;
}

NTSTATUS entry(uint64 p1, uint64 p2)
{
	auto win32k = modules::get_module("win32k.sys");
	if (!win32k)
	{
		printf("Failed to get win32k.sys module\n");
		return false;
	}

	auto code_cave = find_code_cave(win32k.base, win32k.base + win32k.size, sizeof(shellcode));
	if (!code_cave)
	{
		printf("Failed to find code cave\n");
		return false;
	}

	if (!setup_code_cave(code_cave, hooked_ptr))
	{
		printf("Failed to setup code cave\n");
		return false;
	}

	const char pattern[] = "\x48\x83\xEC\x38\x48\x8B\x05\x01\xFB";
	const char mask[] = "xxxxxxxxx";

	if (!init_data_ptr(win32k, pattern, mask, code_cave))
	{
		return STATUS_UNSUCCESSFUL;
	}
	
	return STATUS_SUCCESS;
}