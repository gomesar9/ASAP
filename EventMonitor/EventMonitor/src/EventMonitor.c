/*++
Module Name:
    EventMonitor.c
Abstract:
    This is the main module of the EventMonitor miniFilter driver.
Environment:
    Kernel mode
--*/

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")
#include "dvc/device.h"
#include "bfr/buffer.h"

PFLT_FILTER gFilterHandle;
ULONG_PTR OperationStatusCtx = 1;

// Init
EXTERN_C_START

DRIVER_INITIALIZE DriverEntry;
NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    );

NTSTATUS
EventMonitorUnload(
	PDRIVER_OBJECT DriverObject
);

EXTERN_C_END


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, EventMonitorUnload)
#endif


NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER( RegistryPath );

	debug("DriverEntry: Entered");

	DriverObject->DriverUnload = EventMonitorUnload;

	/* Set up device */
	status = CreateDevice(DriverObject);
	//if ((status = CreateDevice(DriverObject)) != STATUS_SUCCESS)
	//{
	//	debug("Error Creating Device");
	//}
	//DriverObject->DriverUnload = EventMonitorUnload;

    //
    //  Register with FltMgr to tell it our callback routines
    //

    //status = FltRegisterFilter( DriverObject,
    //                            &FilterRegistration,
    //                            &gFilterHandle );

    //FLT_ASSERT( NT_SUCCESS( status ) );

    //if (NT_SUCCESS( status )) {

        //
        //  Start filtering i/o
        //

        //status = FltStartFiltering( gFilterHandle );

        //if (!NT_SUCCESS( status )) {

        //    FltUnregisterFilter( gFilterHandle );
        //}
    //}

	/* create simply buffer */
	if (bfr_create() == 0) {
		debug("Buffer created.");
	}
	//debug("Entry Point: Out");

    return status;
}

NTSTATUS
EventMonitorUnload (
    PDRIVER_OBJECT DriverObject
    )
{
	UNICODE_STRING path;
    //UNREFERENCED_PARAMETER( Flags );

    //PAGED_CODE();

    debug("EventMonitorUnload: Entered");

	//PT_DBG_PRINT(PTDBG_TRACE_ROUTINES, ("Removing Device"));

	RtlInitUnicodeString(&path, DOSDRIVERNAME);
	IoDeleteSymbolicLink(&path);
	IoDeleteDevice(DriverObject->DeviceObject);

	debug("Destroying BFR");
	bfr_destroy();
    //FltUnregisterFilter( gFilterHandle );

    return STATUS_SUCCESS;
}

