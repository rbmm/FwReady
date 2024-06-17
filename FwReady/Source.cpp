#include "stdafx.h"

typedef LONG NTSTATUS;
//
// Logical Data Type - These are 32-bit logical values.
//
typedef ULONG LOGICAL;
typedef ULONG* PLOGICAL;

#ifndef _DEFINED__WNF_STATE_NAME
#define _DEFINED__WNF_STATE_NAME

typedef struct WNF_STATE_NAME {
    ULONG Data[2];
} *PWNF_STATE_NAME;

typedef const WNF_STATE_NAME* PCWNF_STATE_NAME;
#endif

enum WNF_DATA_SCOPE
{
    WnfDataScopeSystem,
    WnfDataScopeSession,
    WnfDataScopeUser,
    WnfDataScopeProcess,
    WnfDataScopeMachine, // REDSTONE3
    WnfDataScopePhysicalMachine, // WIN11
};

typedef struct WNF_TYPE_ID
{
    GUID TypeId;
} * PWNF_TYPE_ID;

typedef const WNF_TYPE_ID* PCWNF_TYPE_ID;

// rev
typedef ULONG WNF_CHANGE_STAMP, * PWNF_CHANGE_STAMP;

EXTERN_C
NTSYSCALLAPI
NTSTATUS
NTAPI
NtUpdateWnfStateData(
    _In_ PCWNF_STATE_NAME StateName,
    _In_reads_bytes_opt_(Length) const VOID* Buffer,
    _In_opt_ ULONG Length,
    _In_opt_ PCWNF_TYPE_ID TypeId,
    _In_opt_ const VOID* ExplicitScope,
    _In_ WNF_CHANGE_STAMP MatchingChangeStamp,
    _In_ LOGICAL CheckStamp
);

EXTERN_C
NTSYSCALLAPI
NTSTATUS
NTAPI
RtlPublishWnfStateData(
	_In_ WNF_STATE_NAME StateName,
	_In_opt_ PCWNF_TYPE_ID TypeId,
	_In_reads_bytes_opt_(Length) const VOID* Buffer,
	_In_opt_ ULONG Length,
	_In_opt_ const VOID* ExplicitScope
);

EXTERN_C
NTSYSAPI
ULONG
NTAPI
RtlNtStatusToDosErrorNoTeb(_In_ NTSTATUS Status);

// from MPSSVC.DLL
NTSTATUS FwDynDataPublishNetworkChangeReadyState(BOOL bReady)
{
    static const WNF_STATE_NAME WNF_WFAS_FIREWALL_NETWORK_CHANGE_READY = { 0xa3bc0875, 0x1287083a };
    return RtlPublishWnfStateData(WNF_WFAS_FIREWALL_NETWORK_CHANGE_READY, 0, &bReady, sizeof(bReady), 0);
    //return NtUpdateWnfStateData(&WNF_WFAS_FIREWALL_NETWORK_CHANGE_READY, &bReady, sizeof(bReady), 0, 0, 0, 0);
}

DWORD WINAPI HandlerEx(
	DWORD /*dwControl*/,
	DWORD /*dwEventType*/,
	PVOID /*lpEventData*/,
	PVOID /*lpContext*/
)
{
	return NOERROR;
}

void NTAPI ServiceMain(DWORD argc, PWSTR argv[])
{
	if (argc)
	{
		if (SERVICE_STATUS_HANDLE hServiceStatus = RegisterServiceCtrlHandlerExW(argv[0], HandlerEx, 0))
		{
			NTSTATUS status = FwDynDataPublishNetworkChangeReadyState(TRUE);
			
			SERVICE_STATUS ss = {
				SERVICE_WIN32_OWN_PROCESS,
				SERVICE_STOPPED,
				0,
				RtlNtStatusToDosErrorNoTeb(status),
				(ULONG)status
			};

			SetServiceStatus(hServiceStatus, &ss);
		}
	}
}

void NTAPI ep(void*)
{
	const static SERVICE_TABLE_ENTRY ste[] = {
		{ const_cast<PWSTR>(L"FwReady"), ServiceMain }, {}
	};

	ExitProcess(StartServiceCtrlDispatcher(ste));
}