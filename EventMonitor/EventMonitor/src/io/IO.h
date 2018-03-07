#pragma once

#include "../config.h"
#include <stdlib.h>
#include <stdio.h>


/* I/O routines */
NTSTATUS NotSupported(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS Read(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS Write(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS Create(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS Close(PDEVICE_OBJECT DeviceObject, PIRP Irp);
