#pragma once
#include <include.hpp>

namespace handler
{
	void pass(_comm_data data)
	{
		PEPROCESS target_process = nullptr;
		if (!NT_SUCCESS(PsLookupProcessByProcessId(reinterpret_cast<HANDLE>(data.target_pid), &target_process)))
		{
			printf("Failed to get target process\n");
			return;
		}

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
				&bytes);

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
				&bytes);

			break;
		}
		case _comm_type::base:
		{
			void* base = PsGetProcessSectionBaseAddress(target_process);
			crt::memcpy(data.src_address, &base, sizeof(base));

			break;
		}
		case _comm_type::cr3:
		{
			void* base = PsGetProcessSectionBaseAddress(target_process);
			physical::cr3::StoredCr3 = physical::cr3::GetFromBase(reinterpret_cast<uint64>(base));
			crt::memcpy(data.src_address, &physical::cr3::StoredCr3, sizeof(physical::cr3::StoredCr3));
			break;
		}
		case _comm_type::peb:
		{
			void* peb = PsGetProcessPeb(target_process);
			crt::memcpy(data.src_address, &peb, sizeof(peb));
			break;
		}
		default:
			printf("Invalid req type\n");
			break;
		}
	}
}