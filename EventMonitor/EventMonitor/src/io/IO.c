/* Branch Monitor
* Marcus Botacin
* 2017
*/

#include "IO.h"
#include "../dbg/debug.h"
//#include "Threading\thread.h"
//#include "list\list.h"
//#include "BTS\BTS.h"


/* Write data from the userland to driver stack */
NTSTATUS Write(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	//TBTS_BUFFER bdata;
	PVOID userbuffer;
	PIO_STACK_LOCATION PIO_STACK_IRP;
	UINT32 _datasize, sizerequired= 0;
	NTSTATUS NtStatus = STATUS_SUCCESS;
	UNREFERENCED_PARAMETER(DeviceObject);
	NtStatus = STATUS_SUCCESS;
	
	PIO_STACK_IRP = IoGetCurrentIrpStackLocation(Irp);

	userbuffer = Irp->AssociatedIrp.SystemBuffer;
	_datasize = PIO_STACK_IRP->Parameters.Write.Length;
	/* Reading */
	debug("Reading user data");
	//char entry[] = malloc ?
	//memcpy(userbuffer, &entry, sizerequired);
	
	Irp->IoStatus.Status = NtStatus;
	Irp->IoStatus.Information = sizerequired;


	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return NtStatus;

}

/* Write data from driver to the userland stack */
NTSTATUS Read(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	//TBTS_BUFFER bdata;
	PVOID userbuffer;
	PIO_STACK_LOCATION PIO_STACK_IRP;
	UINT32 datasize, sizerequired;
	NTSTATUS NtStatus = STATUS_SUCCESS;
	UNREFERENCED_PARAMETER(DeviceObject);
	NtStatus = STATUS_SUCCESS;

	userbuffer = Irp->AssociatedIrp.SystemBuffer;
	PIO_STACK_IRP = IoGetCurrentIrpStackLocation(Irp);

	/* Greatings */
	char hello[6] = "Hello";
	sizerequired = sizeof(hello);
	datasize = PIO_STACK_IRP->Parameters.Read.Length;

	if (datasize >= sizerequired) {
		debug("Copying data to userbuffer");
		memcpy(userbuffer, &hello, sizerequired);
		Irp->IoStatus.Status = NtStatus;
		Irp->IoStatus.Information = sizerequired;
	}
	else {
		Irp->IoStatus.Status = NtStatus;
		Irp->IoStatus.Information = 0;
	}


	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return NtStatus;

}

/* Create File -- start capture mechanism */
NTSTATUS Create(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	NTSTATUS status;
	UNREFERENCED_PARAMETER(DeviceObject);
	debug("Create I/O operation");

	/* I don't know if launching threads inside an I/O routine is OK
	* Anyway, an IOCTL would be more suitable
	* The idea here is Create/CLose work as Start/Stop
	*/

	/* Launch setup threads */
	//control_thread(LOAD_BTS, BTS_CORE);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	status = STATUS_SUCCESS;
	return status;
}


/* CLoseFile/CloseHandle -- stop routine */
NTSTATUS Close(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	NTSTATUS status;
	UNREFERENCED_PARAMETER(DeviceObject);
	debug("Close I/O operation");

	//control_thread(UNLOAD_BTS, BTS_CORE);

	/* On a multicore scenario, you can do something like:
	* n_proc=KeNumberProcessors;
	* for(i=0;i<n_proc;i++)
	* control_thread(action,i);
	*/

	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	status = STATUS_SUCCESS;
	return status;
}

/* generic routine to support non-implemented I/O */
NTSTATUS NotSupported(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	NTSTATUS status;
	UNREFERENCED_PARAMETER(DeviceObject);
	debug("Not Supported I/O operation");
	Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	status = STATUS_NOT_SUPPORTED;
	return status;
}