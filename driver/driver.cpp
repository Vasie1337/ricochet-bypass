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
}

NTSTATUS DriverEntry(uint64 p1, uint64 p2)
{
#ifdef _DEBUG_ANTICHEAT
	NTSTATUS status = PsSetLoadImageNotifyRoutine(on_image_load); // Will bsod; need to find a gadget to bypass PG
	if (!NT_SUCCESS(status))
	{
		printf("Failed to set load image notify routine\n");
		printf("Status: 0x%lx\n", status);
		return status;
	}
#else
	ricochet::disable_callback();
	printf("Disabled callback\n");
#endif

	return STATUS_SUCCESS;
}