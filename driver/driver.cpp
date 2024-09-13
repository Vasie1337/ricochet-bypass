#include <include.hpp>

namespace callbacks
{
	typedef UINT64 QWORD;
	typedef unsigned short      WORD;

	QWORD GetCallbackListOffset(void) {
		POBJECT_TYPE procType = *PsProcessType;

		if (procType && MmIsAddressValid((void*)procType))
		{
			for (int i = 0xF8; i > 0; i -= 8)
			{
				QWORD first = *(QWORD*)((QWORD)procType + i), second = *(QWORD*)((QWORD)procType + (i + 8));
				if (first && MmIsAddressValid((void*)first) && second && MmIsAddressValid((void*)second))
				{
					QWORD test1First = *(QWORD*)(first + 0x0), test1Second = *(QWORD*)(first + 0x8);
					if (test1First && MmIsAddressValid((void*)test1First) && test1Second && MmIsAddressValid((void*)test1Second))
					{
						QWORD testObjectType = *(QWORD*)(first + 0x20);
						if (testObjectType == (QWORD)procType)
							return((QWORD)i);
					}
				}
			}
		}
	}

	OB_PREOP_CALLBACK_STATUS DummyObjectPreCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION OperationInformation) {
		return(OB_PREOP_SUCCESS);
	}

	typedef struct _CALLBACK_ENTRY {
		WORD Version; // 0x0
		WORD OperationRegistrationCount; // 0x2
		DWORD unk1; // 0x4
		PVOID RegistrationContext; // 0x8
		UNICODE_STRING Altitude; // 0x10
	} CALLBACK_ENTRY, * PCALLBACK_ENTRY; // header size: 0x20 (0x6C if you count the array afterwards - this is only the header. The array of CALLBACK_ENTRY_ITEMs is useless.)

	typedef struct _CALLBACK_ENTRY_ITEM {
		LIST_ENTRY CallbackList; // 0x0
		OB_OPERATION Operations; // 0x10
		DWORD Active; // 0x14
		CALLBACK_ENTRY* CallbackEntry; // 0x18
		PVOID ObjectType; // 0x20
		POB_PRE_OPERATION_CALLBACK PreOperation; // 0x28
		POB_POST_OPERATION_CALLBACK PostOperation; // 0x30
		QWORD unk1; // 0x38
	} CALLBACK_ENTRY_ITEM, * PCALLBACK_ENTRY_ITEM; // size: 0x40

	void disable_callback()
	{
		POBJECT_TYPE procType = *PsProcessType;
		if (procType && MmIsAddressValid((void*)procType))
		{
			QWORD callbackListOffset = GetCallbackListOffset();
			if (callbackListOffset && MmIsAddressValid((void*)((QWORD)procType + callbackListOffset)))
			{
				LIST_ENTRY* callbackList = (LIST_ENTRY*)((QWORD)procType + callbackListOffset);
				if (callbackList->Flink && MmIsAddressValid((void*)callbackList->Flink))
				{
					CALLBACK_ENTRY_ITEM* firstCallback = (CALLBACK_ENTRY_ITEM*)callbackList->Flink;
					CALLBACK_ENTRY_ITEM* curCallback = firstCallback;

					do {
						// Make sure the callback is valid.
						if (curCallback && MmIsAddressValid((void*)curCallback) && MmIsAddressValid((void*)curCallback->CallbackEntry))
						{
							ANSI_STRING altitudeAnsi = { 0 };
							UNICODE_STRING altitudeUni = curCallback->CallbackEntry->Altitude;
							RtlUnicodeStringToAnsiString(&altitudeAnsi, &altitudeUni, 1);


							if (!strcmp(altitudeAnsi.Buffer, "329400.017"))
							{
								printf("Altitude: %ws\n", altitudeUni.Buffer);

								if (curCallback->PreOperation) {
									curCallback->PreOperation = DummyObjectPreCallback;
								}
								RtlFreeAnsiString(&altitudeAnsi);
								break;
							}

							RtlFreeAnsiString(&altitudeAnsi);
						}

						// Get the next callback.
						curCallback = (CALLBACK_ENTRY_ITEM*)curCallback->CallbackList.Flink;
					} while (curCallback != firstCallback);
				}
			}
		}
	}
}

PRTL_PROCESS_MODULES mapped_modules = nullptr;

RTL_PROCESS_MODULE_INFORMATION get_module_from_address(UINT64 address)
{
	for (ULONG i = 0; i < mapped_modules->NumberOfModules; i++)
	{
		RTL_PROCESS_MODULE_INFORMATION module = mapped_modules->Modules[i];
		if (address >= (UINT64)module.ImageBase && address < (UINT64)module.ImageBase + module.ImageSize)
		{
			return module;
		}
	}

	RTL_PROCESS_MODULE_INFORMATION module = { 0 };
	return module;
}

NTSTATUS __stdcall hooked_PsCreateSystemThread(PHANDLE ThreadHandle, ULONG DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, HANDLE ProcessHandle, PCLIENT_ID ClientId, PKSTART_ROUTINE StartRoutine, PVOID StartContext)
{
	printf("HOOKED PsCreateSystemThread\n");
	return PsCreateSystemThread(ThreadHandle, DesiredAccess, ObjectAttributes, ProcessHandle, ClientId, StartRoutine, StartContext);
}

NTSTATUS __stdcall hooked_IoCreateSymbolicLink(PUNICODE_STRING SymbolicLinkName, PUNICODE_STRING DeviceName)
{
	printf("HOOKED IoCreateSymbolicLink\n");
	printf("SymbolicLinkName: %wZ\n", SymbolicLinkName);
	printf("DeviceName: %wZ\n", DeviceName);
	return IoCreateSymbolicLink(SymbolicLinkName, DeviceName);
}

VOID __stdcall hooked_IoDeleteDevice(PDEVICE_OBJECT DeviceObject)
{
	printf("HOOKED IoDeleteDevice\n");
	IoDeleteDevice(DeviceObject);
}

VOID __stdcall hooked_ObUnRegisterCallbacks(PVOID RegistrationHandle)
{
	printf("HOOKED ObUnRegisterCallbacks\n");
	ObUnRegisterCallbacks(RegistrationHandle);
}

NTSTATUS __stdcall hooked_PsSetCreateThreadNotifyRoutine(PCREATE_THREAD_NOTIFY_ROUTINE NotifyRoutine)
{
	printf("HOOKED PsSetCreateThreadNotifyRoutine\n");
	return PsSetCreateThreadNotifyRoutine(NotifyRoutine);
}

NTSTATUS __stdcall hooked_PsSetCreateProcessNotifyRoutine(PCREATE_PROCESS_NOTIFY_ROUTINE NotifyRoutine, BOOLEAN Remove)
{
	printf("HOOKED PsSetCreateProcessNotifyRoutine\n");
	return PsSetCreateProcessNotifyRoutine(NotifyRoutine, Remove);
}

NTSTATUS __stdcall hooked_PsSetLoadImageNotifyRoutine(PLOAD_IMAGE_NOTIFY_ROUTINE NotifyRoutine)
{
	printf("HOOKED PsSetLoadImageNotifyRoutine\n");
	return PsSetLoadImageNotifyRoutine(NotifyRoutine);
}

NTSTATUS __stdcall hooked_PsSetCreateProcessNotifyRoutineEx(PCREATE_PROCESS_NOTIFY_ROUTINE_EX NotifyRoutine, BOOLEAN Remove)
{
	printf("HOOKED PsSetCreateProcessNotifyRoutineEx\n");
	return PsSetCreateProcessNotifyRoutineEx(NotifyRoutine, Remove);
}

NTSTATUS __stdcall hooked_IoCreateDevice(PDRIVER_OBJECT DriverObject, ULONG DeviceExtensionSize, PUNICODE_STRING DeviceName, DEVICE_TYPE DeviceType, ULONG DeviceCharacteristics, BOOLEAN Exclusive, PDEVICE_OBJECT* DeviceObject)
{
	printf("HOOKED IoCreateDevice\n");
	return IoCreateDevice(DriverObject, DeviceExtensionSize, DeviceName, DeviceType, DeviceCharacteristics, Exclusive, DeviceObject);
}

VOID __stdcall hooked_IofCompleteRequest(PIRP Irp, CCHAR PriorityBoost)
{
	printf("HOOKED IofCompleteRequest\n");

	ULONG RequestorProcessId = IoGetRequestorProcessId(Irp);
	printf("Request came from process ID: %lu\n", RequestorProcessId);

	__debugbreak();

	IofCompleteRequest(Irp, PriorityBoost);
}

void on_image_load(PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo)
{
	if (wcsstr(FullImageName->Buffer, L"\\Device\\HarddiskVolume5\\SteamLibrary\\steamapps\\common\\Call of Duty HQ\\Randgrid.sys")) 
	{
		printf("Randgrid.sys loaded\n");
		printf("Image base: 0x%llx\n", ImageInfo->ImageBase);
		printf("Image size: 0x%llx\n", ImageInfo->ImageSize);

		printf("Hooking IAT...\n");

		iat::hook("randgrid.sys", "PsCreateSystemThread", hooked_PsCreateSystemThread);
		iat::hook("randgrid.sys", "IoCreateSymbolicLink", hooked_IoCreateSymbolicLink);
		iat::hook("randgrid.sys", "IoDeleteDevice", hooked_IoDeleteDevice);
		iat::hook("randgrid.sys", "ObUnRegisterCallbacks", hooked_ObUnRegisterCallbacks);
		iat::hook("randgrid.sys", "PsSetCreateThreadNotifyRoutine", hooked_PsSetCreateThreadNotifyRoutine);
		iat::hook("randgrid.sys", "PsSetCreateProcessNotifyRoutine", hooked_PsSetCreateProcessNotifyRoutine);
		iat::hook("randgrid.sys", "PsSetLoadImageNotifyRoutine", hooked_PsSetLoadImageNotifyRoutine);
		iat::hook("randgrid.sys", "PsSetCreateProcessNotifyRoutineEx", hooked_PsSetCreateProcessNotifyRoutineEx);
		iat::hook("randgrid.sys", "IoCreateDevice", hooked_IoCreateDevice);
		iat::hook("randgrid.sys", "IofCompleteRequest", hooked_IofCompleteRequest);

			
		printf("IAT hooked\n");


		
	}

}

NTSTATUS DriverEntry(uint64 p1, uint64 p2)
{
	mapped_modules = modules::get_modules();
	if (!mapped_modules)
	{
		printf("Failed to get modules\n");
		return STATUS_UNSUCCESSFUL;
	}

	NTSTATUS status = PsSetLoadImageNotifyRoutine(on_image_load);
	if (!NT_SUCCESS(status))
	{
		printf("Failed to set load image notify routine\n");
		printf("Status: 0x%lx\n", status);
		return status;
	}

	printf("Driver loaded\n");

	return STATUS_SUCCESS;
}