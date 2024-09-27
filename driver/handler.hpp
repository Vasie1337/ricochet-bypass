#pragma once
#include <include.hpp>

namespace handler
{
	void wcharToChar(const wchar_t* s1, char* out_char, size_t max_len) 
	{
		size_t i = 0;
		while (s1[i] != L'\0' && i < max_len - 1) {
			out_char[i] = static_cast<char>(s1[i]);
			i++;
		}
		out_char[i] = '\0';
	}

	void pass(_comm_data data)
	{
		PEPROCESS target_process = reinterpret_cast<PEPROCESS>(data.target_proc);

		switch (data.type)
		{
		case _comm_type::read:
		{
			size_t bytes = 0;

			physical::ReadMemory(
				physical::cr3::StoredCr3,
				data.src_address,
				data.dst_address,
				data.size,
				&bytes
			);

			break;
		}
		case _comm_type::write:
		{
			size_t bytes = 0;

			physical::WriteMemory(
				physical::cr3::StoredCr3,
				data.src_address,
				data.dst_address,
				data.size,
				&bytes
			);

			break;
		}
		case _comm_type::base:
		{
			KAPC_STATE apc_state;
			KeStackAttachProcess(target_process, &apc_state);

			PPEB pPeb = (PPEB)PsGetProcessPeb(target_process);
			if (!pPeb)
			{
				return;
			}

			LARGE_INTEGER time = { 0 };
			time.QuadPart = -250ll * 10 * 1000;

			for (INT i = 0; !pPeb->Ldr && i < 10; i++)
			{
				KeDelayExecutionThread(KernelMode, TRUE, &time);
			}

			if (!pPeb->Ldr)
			{
				return;
			}

			for (int i = 0; i < sizeof(data.str_buffer); i++)
			{
				data.str_buffer[i] = tolower(data.str_buffer[i]);
			}

			PVOID result = nullptr;
			for (PLIST_ENTRY pListEntry = pPeb->Ldr->InLoadOrderModuleList.Flink;
				pListEntry != &pPeb->Ldr->InLoadOrderModuleList;
				pListEntry = pListEntry->Flink)
			{
				PLDR_DATA_TABLE_ENTRY pEntry = CONTAINING_RECORD(pListEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

				WCHAR buffer[1024] = { 0 };
				crt::memcpy(buffer, pEntry->BaseDllName.Buffer, pEntry->BaseDllName.Length);

				char out_char[1024] = { 0 };
				wcharToChar(buffer, out_char, 1024);

				for (int i = 0; i < sizeof(out_char); i++)
				{
					out_char[i] = tolower(out_char[i]);
				}

				if (crt::strcmp(out_char, data.str_buffer) == 0)
				{
					result = pEntry->DllBase;
					break;
				}
			}

			KeUnstackDetachProcess(&apc_state);
			crt::memcpy(data.src_address, &result, sizeof(result));
			break;
		}
		case _comm_type::cr3:
		{
			void* base = PsGetProcessSectionBaseAddress(target_process);
			if (!base)
			{
				printf("Failed to get base\n");
				return;
			}
			physical::cr3::StoredCr3 = physical::cr3::GetFromBase(reinterpret_cast<uint64>(base));
			if (!physical::cr3::StoredCr3)
			{
				printf("Failed to get cr3\n");
				return;
			}
			crt::memcpy(data.src_address, &physical::cr3::StoredCr3, sizeof(physical::cr3::StoredCr3));
			break;
		}
		case _comm_type::peb:
		{
			void* peb = PsGetProcessPeb(target_process);
			if (!peb)
			{
				printf("Failed to get peb\n");
				return;
			}
			crt::memcpy(data.src_address, &peb, sizeof(peb));
			break;
		}
		case _comm_type::proc:
		{
			PEPROCESS target = modules::get_eprocess(data.str_buffer);
			if (!target)
			{
				printf("Failed to get target process\n");
				return;
			}
			crt::memcpy(data.src_address, &target, sizeof(void*));
			break;
		}
		case _comm_type::pid:
		{
			HANDLE pid = PsGetProcessId(target_process);
			if (!pid)
			{
				printf("Failed to get pid\n");
				return;
			}
			crt::memcpy(data.src_address, &pid, sizeof(pid));
			break;
		}
		case _comm_type::mouse:
		{
			mouse::set(data.mouse_data.flags, data.mouse_data.x, data.mouse_data.y);
			break;
		}
		default:
			printf("Invalid req type\n");
			break;
		}
	}
}