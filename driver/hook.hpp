#pragma once
#include <include.hpp>

namespace hook
{
	static inline __int64(__fastcall* original_ptr)(void* a1);
	__int64 hooked(void* a1)
	{
		if (!a1 || !MmIsAddressValid(a1))
			return original_ptr(a1);

		_comm_data data = { 0 };
		crt::memcpy(&data, a1, sizeof(_comm_data));
		xor_comm_data(&data);

		if (data.magic != 0x1337)
			return original_ptr(a1);

		handler::pass(data);

		return 0;
	}

	const char* pattern_list[] = {
		"\x48\x83\xEC\x38\x48\x8B\x05\x01\xFB", // 22H2
		"\x48\x83\xEC\x38\x48\x8B\x05\x89\x8A", // 23H2
		"\x48\x83\xEC\x38\x48\x8B\x05\xC1\xFA"  // 22H2 19045.3803
		"\x48\x83\xEC\x38\x48\x8B\x05\x01\xFB"  // 22H2 19045.4894
	};
	const char mask[] = "xxxxxxxxx";

	bool setup(modules::DATA_ENTRY module, void* target)
	{
		PEPROCESS winlogon = modules::get_eprocess("winlogon.exe");
		if (!winlogon)
		{
			printf("Failed to get winlogon.exe process\n");
			return false;
		}

		KAPC_STATE apc = { 0 };
		KeStackAttachProcess(winlogon, &apc);

		uint64 function = 0;
		for (auto pattern : pattern_list)
		{
			function = scanner::find_pattern(module.base, module.size, pattern, mask);
			if (function)
				break;
		}

		if (!function)
		{
			printf("Failed to find pattern\n");
			KeUnstackDetachProcess(&apc);
			return false;
		}

		printf("Function found at: %p\n", reinterpret_cast<void*>(function));

		int* displacement_ptr = (int*)(function + 7);
		uint64 target_address = function + 7 + 4 + *displacement_ptr;

		*(void**)&original_ptr = _InterlockedExchangePointer((void**)target_address, target);

		printf("Original function at: %p\n", original_ptr);
		printf("Target address: %llx\n", target_address);

		KeUnstackDetachProcess(&apc);

		return true;
	}
}