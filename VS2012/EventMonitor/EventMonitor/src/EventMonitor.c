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

PFLT_FILTER gFilterHandle;
ULONG_PTR OperationStatusCtx = 1;



/*************************************************************************
    Prototypes
*************************************************************************/

//EXTERN_C_START

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

//EXTERN_C_END

//
//  Assign text sections for each routine.
//

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, EventMonitorUnload)
#endif

//
//  operation registration
//

/*************************************************************************
    MiniFilter initialization and unload routines.
*************************************************************************/

NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:

    This is the initialization routine for this miniFilter driver.  This
    registers with FltMgr and initializes all global data structures.

Arguments:

    DriverObject - Pointer to driver object created by the system to
        represent this driver.

    RegistryPath - Unicode string identifying where the parameters for this
        driver are located in the registry.

Return Value:

    Routine can return non success error codes.

--*/
{
    NTSTATUS status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER( RegistryPath );

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("EventMonitor!DriverEntry: Entered\n") );

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
	debug("Entry Point: Out");

    return status;
}

NTSTATUS
EventMonitorUnload (
    PDRIVER_OBJECT DriverObject
    )
/*++

Routine Description:

    This is the unload routine for this miniFilter driver. This is called
    when the minifilter is about to be unloaded. We can fail this unload
    request if this is not a mandatory unload indicated by the Flags
    parameter.

Arguments:

    Flags - Indicating if this is a mandatory unload.

Return Value:

    Returns STATUS_SUCCESS.

--*/
{
	UNICODE_STRING path;
    //UNREFERENCED_PARAMETER( Flags );

    //PAGED_CODE();

    debug("EventMonitorUnload: Entered");

	//PT_DBG_PRINT(PTDBG_TRACE_ROUTINES, ("Removing Device"));

	RtlInitUnicodeString(&path, DOSDRIVERNAME);
	IoDeleteSymbolicLink(&path);
	IoDeleteDevice(DriverObject->DeviceObject);

    //FltUnregisterFilter( gFilterHandle );

    return STATUS_SUCCESS;
}


/*************************************************************************
    MiniFilter callback routines.
*************************************************************************/
