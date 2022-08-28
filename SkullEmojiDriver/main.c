#include <wdm.h>

UNICODE_STRING DEVICE_NAME = RTL_CONSTANT_STRING(L"\\Device\\SkullEmoji");
UNICODE_STRING DEVICE_SYMBOLIC_NAME = RTL_CONSTANT_STRING(L"\\??\\SkullEmoji");

#define IOCTL_SKULLCODE CTL_CODE(FILE_DEVICE_UNKNOWN, 0X2049, METHOD_BUFFERED, FILE_ANY_ACCESS)

void DriverUnload(PDRIVER_OBJECT dob);
NTSTATUS MajorFunctions(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS HandleCustomIOCTL(PDEVICE_OBJECT DeviceObject, PIRP Irp);

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);

	NTSTATUS status = 0;

	DriverObject->DriverUnload = DriverUnload;

	DriverObject->MajorFunction[IRP_MJ_CREATE] = MajorFunctions;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = MajorFunctions;

	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HandleCustomIOCTL;

	status = IoCreateDevice(DriverObject, 0, &DEVICE_NAME, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DriverObject->DeviceObject);
	if (!NT_SUCCESS(status)) {
		KdPrint(("Could Not Create Device"));
		return status;
	}

	status = IoCreateSymbolicLink(&DEVICE_SYMBOLIC_NAME, &DEVICE_NAME);
	if (!NT_SUCCESS(status)) {
		IoDeleteDevice(DriverObject->DeviceObject);
		KdPrint(("Could Not Create Symlink"));
		return status;
	}

	DbgPrint("Driver loaded");

	return STATUS_SUCCESS;
}

void DriverUnload(PDRIVER_OBJECT dob) {
	DbgPrint("Driver unloaded");

	IoDeleteSymbolicLink(&DEVICE_SYMBOLIC_NAME);
	IoDeleteDevice(dob->DeviceObject);
}

NTSTATUS MajorFunctions(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);

	PIO_STACK_LOCATION stackLocation = IoGetCurrentIrpStackLocation(Irp);

	switch (stackLocation->MajorFunction) {
	case IRP_MJ_CREATE:
		DbgPrint("Handle to Symlink created");
		break;
	case IRP_MJ_CLOSE:
		DbgPrint("Handle to Symlink closed");
		break;
	}

	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS HandleCustomIOCTL(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);

	PIO_STACK_LOCATION stackLocation = IoGetCurrentIrpStackLocation(Irp);

	CHAR* hello = "Hello from kernel";

	// Receive data
	if (stackLocation->Parameters.DeviceIoControl.IoControlCode == IOCTL_SKULLCODE) {
		DbgPrint("Received %s", (char*)Irp->AssociatedIrp.SystemBuffer);
	}

	// Send data
	Irp->IoStatus.Information = strlen(hello);
	Irp->IoStatus.Status = STATUS_SUCCESS;

	RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, hello, strlen(hello));

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}
