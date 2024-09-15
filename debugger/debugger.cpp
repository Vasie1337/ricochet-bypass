#include <include.hpp>

//#define _DEBUG_ANTICHEAT // If disabled, no bsod

static void on_image_load(PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo)
{
	if (wcsstr(FullImageName->Buffer, L"\\Device\\HarddiskVolume5\\SteamLibrary\\steamapps\\common\\Call of Duty HQ\\Randgrid.sys"))
	{
		printf("Randgrid.sys loaded\n");
		printf("Image base: 0x%llx\n", ImageInfo->ImageBase);
		printf("Image size: 0x%llx\n", ImageInfo->ImageSize);

		printf("Hooking IAT...\n");

		iat::hook("randgrid.sys", "PsCreateSystemThread", hooks::hooked_PsCreateSystemThread);
		iat::hook("randgrid.sys", "IoCreateSymbolicLink", hooks::hooked_IoCreateSymbolicLink);
		iat::hook("randgrid.sys", "IoDeleteDevice", hooks::hooked_IoDeleteDevice);
		iat::hook("randgrid.sys", "ObUnRegisterCallbacks", hooks::hooked_ObUnRegisterCallbacks);
		iat::hook("randgrid.sys", "PsSetCreateThreadNotifyRoutine", hooks::hooked_PsSetCreateThreadNotifyRoutine);
		iat::hook("randgrid.sys", "PsSetCreateProcessNotifyRoutine", hooks::hooked_PsSetCreateProcessNotifyRoutine);
		iat::hook("randgrid.sys", "PsSetLoadImageNotifyRoutine", hooks::hooked_PsSetLoadImageNotifyRoutine);
		iat::hook("randgrid.sys", "PsSetCreateProcessNotifyRoutineEx", hooks::hooked_PsSetCreateProcessNotifyRoutineEx);
		iat::hook("randgrid.sys", "IoCreateDevice", hooks::hooked_IoCreateDevice);
		iat::hook("randgrid.sys", "IofCompleteRequest", hooks::hooked_IofCompleteRequest);

		printf("IAT hooked\n");
	}
	return;
}

NTSTATUS entry(uint64 p1, uint64 p2)
{
#ifdef _DEBUG_ANTICHEAT
	auto driver = modules::get_kernel_module("WdFilter.sys");
	if (!driver)
	{
		printf("Failed to get driver\n");
		return STATUS_NOT_FOUND;
	}

	auto code_cave = scanner::find_pattern(driver.base, "\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC", "xxxxxxxxxxxxx");
	if (!code_cave)
	{
		printf("Failed to find code cave\n");
		return STATUS_NOT_FOUND;
	}

	printf("Code cave found at 0x%llx\n", code_cave);

	static const uint8 shellcode[] = {
		0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // mov rax, xxxxxxxx
		0xFF, 0xE0,                                                  // jmp rax
		0xC3 			   											 // ret
	};
	*(uint64*)&shellcode[2] = (uint64)on_image_load;

	uintptr_t cr0 = __readcr0();
	__writecr0(cr0 & ~(1 << 16));

	crt::memcpy((void*)code_cave, shellcode, sizeof(shellcode));

	__writecr0(cr0);

	NTSTATUS status = PsSetLoadImageNotifyRoutine(reinterpret_cast<PLOAD_IMAGE_NOTIFY_ROUTINE>(code_cave));
	if (!NT_SUCCESS(status))
	{
		printf("Failed to set load image notify routine\n");
		printf("Status: 0x%lx\n", status);
		return status;
	}
#else
	ricochet::disable_callback();
	iat::hook("randgrid.sys", "IofCompleteRequest", hooks::hooked_IofCompleteRequest);
	printf("Disabled callback\n");
#endif

	return STATUS_SUCCESS;
}