#pragma once
#include <include.hpp>

namespace modules
{
	typedef class DATA_ENTRY
	{
	public:
		bool operator!() const
		{
			return !base || !size;
		}

		uint64 base;
		uint64 size;
	};

	PRTL_PROCESS_MODULES get_modules()
	{
		NTSTATUS Status = STATUS_SUCCESS;
		ULONG Bytes = 0;
		PRTL_PROCESS_MODULES Modules = NULL;

		Status = ZwQuerySystemInformation(SystemModuleInformation, 0, Bytes, &Bytes);
		if (Status != STATUS_INFO_LENGTH_MISMATCH)
			return NULL;

		Modules = (PRTL_PROCESS_MODULES)ExAllocatePool(NonPagedPool, Bytes);
		if (!Modules)
			return NULL;

		Status = ZwQuerySystemInformation(SystemModuleInformation, Modules, Bytes, &Bytes);
		if (!NT_SUCCESS(Status))
		{
			ExFreePool(Modules);
			return NULL;
		}

		return Modules;
	}

	DATA_ENTRY get_module(const char* module_name)
	{
		DATA_ENTRY entry = { 0 };

		PRTL_PROCESS_MODULES Modules = get_modules();
		if (!Modules)
			return entry;

		for (ULONG i = 0; i < Modules->NumberOfModules; i++)
		{
			const char* module = strrchr((const char*)Modules->Modules[i].FullPathName, '\\');
			if (!module)
				continue;

			module += 1;

			if (!crt::strcmp(module, module_name))
			{
				entry.base = (uint64)Modules->Modules[i].ImageBase;
				entry.size = Modules->Modules[i].ImageSize;
				break;
			}
		}

		ExFreePool(Modules);
		return entry;
	}

	DATA_ENTRY get_module_section(DATA_ENTRY module, const char* section_name)
	{
		PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)module.base;
		PIMAGE_NT_HEADERS nt_headers = (PIMAGE_NT_HEADERS)(module.base + dos_header->e_lfanew);
		PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt_headers);

		for (unsigned i = 0; i < nt_headers->FileHeader.NumberOfSections; i++)
		{
			if (crt::strstr((const char*)section->Name, section_name))
			{
				DATA_ENTRY entry = { 0 };
				entry.base = module.base + section->VirtualAddress;
				entry.size = section->Misc.VirtualSize;
				return entry;
			}

			section++;
		}

		return { 0 };
	}

	bool dump_driver(modules::DATA_ENTRY entry)
	{
		uint8* buffer = (uint8*)ExAllocatePool(NonPagedPool, entry.size);
		if (!buffer)
		{
			printf("Failed to allocate memory\n");
			return false;
		}

		RtlZeroMemory(buffer, entry.size);
		RtlCopyMemory(buffer, (void*)entry.base, entry.size);

		PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)buffer;
		PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(buffer + dos->e_lfanew);
		PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt);

		for (uint32 i = 0; i < nt->FileHeader.NumberOfSections; i++)
		{
			section->PointerToRawData = section->VirtualAddress;
			section++;
		}

		HANDLE file = NULL;
		OBJECT_ATTRIBUTES oa = { 0 };
		UNICODE_STRING us;
		IO_STATUS_BLOCK iosb;

		RtlInitUnicodeString(&us, L"\\??\\C:\\dumped_drv.sys");
		InitializeObjectAttributes(&oa, &us, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

		NTSTATUS status = ZwCreateFile(&file, GENERIC_WRITE, &oa, &iosb, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OVERWRITE_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
		if (!NT_SUCCESS(status))
		{
			printf("Failed to create file\n");
			printf("Status: %x\n", status);
			ExFreePool(buffer);
			return false;
		}

		ULONG bytes = 0;
		status = ZwWriteFile(file, NULL, NULL, NULL, &iosb, buffer, entry.size, NULL, NULL);
		if (!NT_SUCCESS(status))
		{
			printf("Failed to write file\n");
			ZwClose(file);
			ExFreePool(buffer);
			return false;
		}

		ZwClose(file);
		ExFreePool(buffer);

		printf("Driver dumped\n");

		return true;
	}

	PEPROCESS get_eprocess(const char* process_name)
	{
		PEPROCESS sys_process = PsInitialSystemProcess;
		PEPROCESS curr_entry = sys_process;

		char image_name[15];

		do
		{
			crt::memcpy((void*)(&image_name), (void*)((uintptr_t)curr_entry + 0x5a8), sizeof(image_name));

			if (crt::strcmp(image_name, process_name) == 0)
			{
				uint32 active_threads;

				crt::memcpy((void*)&active_threads, (void*)((uintptr_t)curr_entry + 0x5f0), sizeof(active_threads));

				if (active_threads)
				{
					return curr_entry;
				}
			}

			PLIST_ENTRY list = (PLIST_ENTRY)((uintptr_t)(curr_entry)+0x448);
			curr_entry = (PEPROCESS)((uintptr_t)list->Flink - 0x448);

		} while (curr_entry != sys_process);

		return 0;
	}
}