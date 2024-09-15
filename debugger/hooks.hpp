#pragma once
#include <include.hpp>

namespace hooks
{
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

		IofCompleteRequest(Irp, PriorityBoost);
	}
}