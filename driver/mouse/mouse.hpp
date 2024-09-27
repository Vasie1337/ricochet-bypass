#pragma once
#include <include.hpp>

EXTERN_C VOID MouseClassServiceCallback(PDEVICE_OBJECT DeviceObject, PMOUSE_INPUT_DATA InputDataStart, PMOUSE_INPUT_DATA InputDataEnd, PULONG InputDataConsumed);

namespace mouse
{
	bool open()
	{
        _KeAcquireSpinLockAtDpcLevel = reinterpret_cast<UINT64>(KeAcquireSpinLockAtDpcLevel);
        _KeReleaseSpinLockFromDpcLevel = reinterpret_cast<UINT64>(KeReleaseSpinLockFromDpcLevel);
        _IofCompleteRequest = reinterpret_cast<UINT64>(IofCompleteRequest);
        _IoReleaseRemoveLockEx = reinterpret_cast<UINT64>(IoReleaseRemoveLockEx);

		UNICODE_STRING class_string;
		RtlInitUnicodeString(&class_string, L"\\Driver\\MouClass");

		PDRIVER_OBJECT class_driver_object = NULL;
		NTSTATUS status = ObReferenceObjectByName(&class_string, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&class_driver_object);
		if (!NT_SUCCESS(status)) 
		{
			printf("Failed to get class driver object\n");
			return false;
		}

		UNICODE_STRING hid_string;
		RtlInitUnicodeString(&hid_string, L"\\Driver\\MouHID");

		PDRIVER_OBJECT hid_driver_object = NULL;

		status = ObReferenceObjectByName(&hid_string, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&hid_driver_object);
		if (!NT_SUCCESS(status)) 
		{
			if (class_driver_object) ObfDereferenceObject(class_driver_object);
			printf("Failed to get hid driver object\n");
			return false;
		}

		PDEVICE_OBJECT hid_device_object = hid_driver_object->DeviceObject;

        while (hid_device_object && !gMouseObject.service_callback) 
        {
			PDEVICE_OBJECT class_device_object = class_driver_object->DeviceObject;

			while (class_device_object && !gMouseObject.service_callback) 
			{
				if (!class_device_object->NextDevice && !gMouseObject.mouse_device)
					gMouseObject.mouse_device = class_device_object;

				PULONG_PTR device_extension = reinterpret_cast<PULONG_PTR>(hid_device_object->DeviceExtension);
				ULONG_PTR device_ext_size = ((reinterpret_cast<ULONG_PTR>(hid_device_object->DeviceObjectExtension) - reinterpret_cast<ULONG_PTR>(hid_device_object->DeviceExtension)) / 4);

				for (ULONG_PTR i = 0; i < device_ext_size; i++) 
				{
					if (device_extension[i] == reinterpret_cast<ULONG_PTR>(class_device_object) && device_extension[i + 1] > reinterpret_cast<ULONG_PTR>(class_driver_object)) 
					{
						gMouseObject.service_callback = reinterpret_cast<MouseClassServiceCallbackFn>(device_extension[i + 1]);
						break;
					}
				}
				class_device_object = class_device_object->NextDevice;
			}
			hid_device_object = hid_device_object->AttachedDevice;
        }

		if (!gMouseObject.mouse_device) 
		{
			PDEVICE_OBJECT target_device_object = class_driver_object->DeviceObject;
			while (target_device_object) 
			{
				if (!target_device_object->NextDevice) 
				{
					gMouseObject.mouse_device = target_device_object;
					break;
				}
				target_device_object = target_device_object->NextDevice;
			}
		}

		ObfDereferenceObject(class_driver_object);
		ObfDereferenceObject(hid_driver_object);

		return gMouseObject.mouse_device && gMouseObject.service_callback;
	}

	void set(int x, int y, unsigned short flags, unsigned short button_flags)
	{
		KIRQL irql = { 0 };
		MOUSE_INPUT_DATA mid = { 0 };
		ULONG input_data;

		mid.LastX = x;
		mid.LastY = y;
		mid.ButtonFlags = button_flags;
		mid.Flags = flags;
		mid.UnitId = 1;

		KeRaiseIrql(DISPATCH_LEVEL, &irql);
		MouseClassServiceCallback(gMouseObject.mouse_device, &mid, (PMOUSE_INPUT_DATA)&mid + 1, &input_data);
		KeLowerIrql(irql);
	}
}