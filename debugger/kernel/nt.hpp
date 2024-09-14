#pragma once
#include <include.hpp>

typedef unsigned long long uint64;
typedef unsigned long uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
	HANDLE Section;
	void* MappedBase;
	void* ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES
{
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;

typedef struct _CALLBACK_ENTRY {
	unsigned short Version; // 0x0
	unsigned short OperationRegistrationCount; // 0x2
	DWORD unk1; // 0x4
	PVOID RegistrationContext; // 0x8
	UNICODE_STRING Altitude; // 0x10
} CALLBACK_ENTRY, * PCALLBACK_ENTRY;

typedef struct _CALLBACK_ENTRY_ITEM {
	LIST_ENTRY CallbackList; // 0x0
	OB_OPERATION Operations; // 0x10
	DWORD Active; // 0x14
	CALLBACK_ENTRY* CallbackEntry; // 0x18
	PVOID ObjectType; // 0x20
	POB_PRE_OPERATION_CALLBACK PreOperation; // 0x28
	POB_POST_OPERATION_CALLBACK PostOperation; // 0x30
	UINT64 unk1; // 0x38
} CALLBACK_ENTRY_ITEM, * PCALLBACK_ENTRY_ITEM; // size: 0x40

typedef enum _SYSTEM_INFORMATION_CLASS
{
	SystemBasicInformation,
	SystemProcessorInformation,
	SystemPerformanceInformation,
	SystemTimeOfDayInformation,
	SystemPathInformation,
	SystemProcessInformation,
	SystemCallCountInformation,
	SystemDeviceInformation,
	SystemProcessorPerformanceInformation,
	SystemFlagsInformation,
	SystemCallTimeInformation,
	SystemModuleInformation,
	SystemLocksInformation,
	SystemStackTraceInformation,
	SystemPagedPoolInformation,
	SystemNonPagedPoolInformation,
	SystemHandleInformation,
	SystemObjectInformation,
	SystemPageFileInformation,
	SystemVdmInstemulInformation,
	SystemVdmBopInformation,
	SystemFileCacheInformation,
	SystemPoolTagInformation,
	SystemInterruptInformation,
	SystemDpcBehaviorInformation,
	SystemFullMemoryInformation,
	SystemLoadGdiDriverInformation,
	SystemUnloadGdiDriverInformation,
	SystemTimeAdjustmentInformation,
	SystemSummaryMemoryInformation,
	SystemNextEventIdInformation,
	SystemEventIdsInformation,
	SystemCrashDumpInformation,
	SystemExceptionInformation,
	SystemCrashDumpStateInformation,
	SystemKernelDebuggerInformation,
	SystemContextSwitchInformation,
	SystemRegistryQuotaInformation,
	SystemExtendServiceTableInformation,
	SystemPrioritySeperation,
	SystemPlugPlayBusInformation,
	SystemDockInformation,
	SystemProcessorSpeedInformation,
	SystemCurrentTimeZoneInformation,
	SystemLookasideInformation,
	SystemBigPoolInformation = 0x42
} SYSTEM_INFORMATION_CLASS, * PSYSTEM_INFORMATION_CLASS;

struct VirtualAddress {
	union {
		uint64 Value;
		struct {
			uint64 offset : 12;
			uint64 pt_index : 9;
			uint64 pd_index : 9;
			uint64 pdpt_index : 9;
			uint64 pml4_index : 9;
			uint64 reserved : 16;
		};
	};
};

struct _MMPTE_HARDWARE
{
	ULONGLONG Valid : 1;              
	ULONGLONG Dirty1 : 1;             
	ULONGLONG Owner : 1;              
	ULONGLONG WriteThrough : 1;       
	ULONGLONG CacheDisable : 1;       
	ULONGLONG Accessed : 1;           
	ULONGLONG Dirty : 1;              
	ULONGLONG LargePage : 1;          
	ULONGLONG Global : 1;             
	ULONGLONG CopyOnWrite : 1;        
	ULONGLONG Unused : 1;             
	ULONGLONG Write : 1;              
	ULONGLONG PageFrameNumber : 40;   
	ULONGLONG ReservedForSoftware : 4;
	ULONGLONG WsleAge : 4;            
	ULONGLONG WsleProtection : 3;     
	ULONGLONG NoExecute : 1;          
};

struct _MMPTE
{
	union
	{
		struct _MMPTE_HARDWARE Hard;
	} u;
};


EXTERN_C_START

NTSTATUS NTAPI ZwQuerySystemInformation(ULONG SystemInformationClass, void* SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);

NTSTATUS NTAPI MmCopyVirtualMemory(PEPROCESS SourceProcess, void* SourceAddress, PEPROCESS TargetProcess, void* TargetAddress, SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode, PSIZE_T ReturnSize);

NTSTATUS NTAPI IoCreateDriver(PUNICODE_STRING DriverName, PDRIVER_INITIALIZE InitializationFunction);

void* NTAPI PsGetProcessSectionBaseAddress(PEPROCESS Process);

EXTERN_C_END