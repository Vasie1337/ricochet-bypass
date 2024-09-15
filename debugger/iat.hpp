#pragma once
#include <include.hpp>

namespace iat
{
	uintptr_t find_import_address(const char* module_name, const char* function_name)
	{
		auto found_module = modules::get_kernel_module(module_name);
		if (!found_module)
		{
			printf("Failed to get module\n");
			return 0;
		}
				
		PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)found_module.base;
		PIMAGE_NT_HEADERS64 nt_headers = (PIMAGE_NT_HEADERS64)((uintptr_t)dos_header + dos_header->e_lfanew);
		
		PIMAGE_IMPORT_DESCRIPTOR import_descriptor = (PIMAGE_IMPORT_DESCRIPTOR)((uintptr_t)dos_header + nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		
		for (size_t i = 0; import_descriptor[i].Name; i++)
		{
			const char* import_module_name = (const char*)((uintptr_t)dos_header + import_descriptor[i].Name);
		
			PIMAGE_THUNK_DATA64 original_thunk = (PIMAGE_THUNK_DATA64)((uintptr_t)dos_header + import_descriptor[i].OriginalFirstThunk);
		
			PIMAGE_THUNK_DATA64 first_thunk = (PIMAGE_THUNK_DATA64)((uintptr_t)dos_header + import_descriptor[i].FirstThunk);
		
			for (size_t j = 0; original_thunk[j].u1.AddressOfData; j++)
			{
				PIMAGE_IMPORT_BY_NAME import = (PIMAGE_IMPORT_BY_NAME)((uintptr_t)dos_header + original_thunk[j].u1.AddressOfData);
		
				if (!strcmp((const char*)import->Name, function_name))
				{
					return (uintptr_t)&first_thunk[j].u1.Function;
				}
			}
		}

		return 0;
	}

	bool hook(const char* module_name, const char* function_name, void* hook)
	{
		uintptr_t import_address = find_import_address(module_name, function_name);
		if (!import_address)
		{
			printf("Failed to find import address for %s\n", function_name);
			return false;
		}

		auto cr0 = __readcr0();
		__writecr0(cr0 & ~(1 << 16));
		
		crt::memcpy((void*)import_address, &hook, sizeof(uintptr_t));
		
		__writecr0(cr0);

		return true;
	}
}