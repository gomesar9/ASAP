/* 
* Event Monitor (extends Branch Monitor)
* Marcus Botacin, Alexandre Gomes
* 2018
*/

#include "IO.h"
#include "../dbg/debug.h"
#include "../bfr/buffer.h"
#include "../ems/EMS.h"


/* Write data from the userland to driver stack */
NTSTATUS Write(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	//TBTS_BUFFER bdata;
	PVOID userbuffer;
	PIO_STACK_LOCATION PIO_STACK_IRP;
	UINT32 datasize, sizerequired= 0;
	CHAR msg[EMS_BUFFER_MAX_LENGHT+32];
	NTSTATUS NtStatus = STATUS_SUCCESS;
	// --+-- EMC use --+--
	CHAR _cmdBfr[EMS_BUFFER_MAX_LENGHT + 1];
	memset(_cmdBfr, '\0', EMS_BUFFER_MAX_LENGHT * sizeof(char)); // For sure..
	
	PIO_STACK_IRP = IoGetCurrentIrpStackLocation(Irp);

	userbuffer = Irp->AssociatedIrp.SystemBuffer;
	datasize = PIO_STACK_IRP->Parameters.Write.Length;

	// Reading
	if (datasize < EMS_BUFFER_MAX_LENGHT) {
		memcpy(_cmdBfr, userbuffer, datasize);
		_cmdBfr[EMS_BUFFER_MAX_LENGHT] = '\0'; // Assurance

		sprintf(msg, "IO: msg received: %s (%u).", _cmdBfr, datasize);
		debug(msg);

		NtStatus = execute(_cmdBfr, (UINT16) datasize);
		
		Irp->IoStatus.Status = NtStatus;
	} else {
		debug("[!] CMD buffer overflow");
		return STATUS_BUFFER_OVERFLOW;
	}
	
	Irp->IoStatus.Status = NtStatus;
	Irp->IoStatus.Information = sizerequired;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return NtStatus;

}

/* Write data from driver to the userland stack */
NTSTATUS Read(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
	//TBTS_BUFFER bdata;
	PVOID userbuffer;
	PIO_STACK_LOCATION PIO_STACK_IRP;
	UINT32 datasize;//, sizerequired;
	size_t sizerequired;

#ifdef REFAC
	CHAR samples[IO_MAX_OUT_BUFFER];
	//CHAR buff[BFR_SIZE];
	INT return_code;
#else
	char buff[BFR_SIZE];
#endif

	userbuffer = Irp->AssociatedIrp.SystemBuffer;
	PIO_STACK_IRP = IoGetCurrentIrpStackLocation(Irp);
	datasize = PIO_STACK_IRP->Parameters.Read.Length;

	// --+-- BAND-AID --+--
	UINT32 CORE_TMP = 0;
	return_code = get_samples(samples, CORE_TMP);
	if (return_code != 0) {
//		sprintf(buff, "%llu", samples);
//	} else {
		sprintf(samples, "%d", -1);
	}

#ifdef REFAC
	sizerequired = strlen(samples);
	memcpy(userbuffer, samples, sizerequired);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = sizerequired;
#else
	if (resp == 0) {
		sizerequired = strlen(buff);
		if (datasize >= sizerequired) {
#ifdef DEBUG
			debug("Copying data to userbuffer");
#endif
			memcpy(userbuffer, buff, sizerequired);

			Irp->IoStatus.Status = STATUS_SUCCESS;
			Irp->IoStatus.Information = sizerequired;
		} else {
			debug("Insufficient IRP size.");
			Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
			Irp->IoStatus.Information = 0;
		}
	}
#endif


	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
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
	char msg[128];
	sprintf(msg, "Not supported I/O operation (Flags: %lu)", Irp->Flags);
	debug(msg);

	Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	status = STATUS_NOT_SUPPORTED;

	return status;
}
