#pragma once
#include <include.hpp>

namespace hook
{
	static inline __int64(__fastcall* original_ptr)(__int64 a1);
	__int64 handler(__int64 a1)
	{
		if (a1 != 0x1337)
			return original_ptr(a1);

		printf("Hooked that nigga: %llx\n", a1);

		return 0;
	}

	const char pattern[] = "\x48\x83\xEC\x38\x48\x8B\x05\x01\xFB";
	const char mask[] = "xxxxxxxxx";

	bool setup(modules::DATA_ENTRY module, void* target)
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
}