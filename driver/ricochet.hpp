#pragma once
#include <include.hpp>

namespace ricochet
{
	UINT64 GetCallbackListOffset() // Pasted lol
	{
		POBJECT_TYPE procType = *PsProcessType;

		if (procType && MmIsAddressValid((void*)procType))
		{
			for (int i = 0xF8; i > 0; i -= 8)
			{
				UINT64 first = *(UINT64*)((UINT64)procType + i), second = *(UINT64*)((UINT64)procType + (i + 8));
				if (first && MmIsAddressValid((void*)first) && second && MmIsAddressValid((void*)second))
				{
					UINT64 test1First = *(UINT64*)(first + 0x0), test1Second = *(UINT64*)(first + 0x8);
					if (test1First && MmIsAddressValid((void*)test1First) && test1Second && MmIsAddressValid((void*)test1Second))
					{
						UINT64 testObjectType = *(UINT64*)(first + 0x20);
						if (testObjectType == (UINT64)procType)
							return((UINT64)i);
					}
				}
			}
		}
	}

	OB_PREOP_CALLBACK_STATUS DummyObjectPreCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION OperationInformation) {
		return(OB_PREOP_SUCCESS);
	}

	void disable_callback()
	{
		UINT64 callbackListOffset = GetCallbackListOffset();
		if (!callbackListOffset || !MmIsAddressValid((void*)((UINT64)*PsProcessType + callbackListOffset)))
		{
			printf("Failed to get callback list offset\n");
			return;
		}

		LIST_ENTRY* callbackList = (LIST_ENTRY*)((UINT64)*PsProcessType + callbackListOffset);
		if (!callbackList->Flink || !MmIsAddressValid((void*)callbackList->Flink))
		{
			printf("Failed to get callback list\n");
			return;
		}

		CALLBACK_ENTRY_ITEM* firstCallback = (CALLBACK_ENTRY_ITEM*)callbackList->Flink;
		CALLBACK_ENTRY_ITEM* curCallback = firstCallback;

		do
		{
			if (curCallback && MmIsAddressValid((void*)curCallback) && MmIsAddressValid((void*)curCallback->CallbackEntry))
			{
				ANSI_STRING altitudeAnsi = { 0 };
				UNICODE_STRING altitudeUni = curCallback->CallbackEntry->Altitude;
				RtlUnicodeStringToAnsiString(&altitudeAnsi, &altitudeUni, 1);

				if (!strcmp(altitudeAnsi.Buffer, "329400.017")) // Altitude of Ricochet driver
				{
					if (curCallback->PreOperation)
					{
						curCallback->PreOperation = DummyObjectPreCallback;
					}
					RtlFreeAnsiString(&altitudeAnsi);
					break;
				}

				RtlFreeAnsiString(&altitudeAnsi);
			}

			curCallback = (CALLBACK_ENTRY_ITEM*)curCallback->CallbackList.Flink;
		} while (curCallback != firstCallback);
	}
}