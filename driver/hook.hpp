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

		if (data.magic != 0x1337)
			return original_ptr(a1);

		handler::pass(data);

		return 0;
	}

	constexpr char pattern[] = "\x48\x83\xEC\x38\x48\x8B\x05\x01\xFB";
	constexpr char mask[] = "xxxxxxxxx";

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
		printf("Target address: %llx\n", target_address);

		KeUnstackDetachProcess(&apc);

		return true;
	}
}