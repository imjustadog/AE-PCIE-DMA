/*
==========================================================================================
                          "Copyright © 2006 Avnet, Inc. All Rights Reserved.
============================================================================================
THIS SOFTWARE IS provided “AS IS” WITHOUT WARRANTY OF ANY KIND.  AVNET MAKES NO WARRANTIES, 
EITHER EXPRESS OR IMPLIED, WITH RESPECT TO THIS SOFTWARE.  AVNET SPECIFICALLY DISCLAIMS THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE AND ANY WARRANTY 
AGAINST INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHT OF ANY THIRD PARTY WITH REGARD TO 
THIS SOFTWARE. AVNET RESERVES THE RIGHT, WITHOUT NOTICE, TO MAKE CHANGES TO THIS SOFTWARE. 
 
IN NO EVENT SHALL AVNET BE LIABLE FOR ANY INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES OF ANY KIND OR NATURE, INCLUDING, WITHOUT LIMITATION, BUSINESS INTERRUPTION COSTS, 
LOSS OF PROFIT OR REVENUE, LOSS OF DATA, PROMOTIONAL OR MANUFACTURING EXPENSES, OVERHEAD, 
INJURY TO REPUTATION OR LOSS OF CUSTOMERS ARISING OUT OF THE USE OR INSTALLATION OF THIS 
SOFTWARE."
============================================================================================ 
*/
// Bus Master DMA driver to demonstrate performance with the 
// Avnet/Xilinx Spartan3-1000 PCIe Starter Board
#include "s3_1000.h"
#include "msglog.h"
#include "wchar.h"

#define	FPGA						0				// BAR 0 contains mapped device registers

VOID
ReadAdapterControl(
  IN PDEVICE_OBJECT pDevObj,
  IN PIRP pIrp,
  IN PSCATTER_GATHER_LIST pSGL,
  IN PVOID pContext);

VOID
WriteAdapterControl(
  IN PDEVICE_OBJECT pDevObj,
  IN PIRP pIrp,
  IN PSCATTER_GATHER_LIST pSGL,
  IN PVOID pContext);
  
NTSTATUS 
OnRequestComplete(
  PDEVICE_OBJECT junk, 
  PIRP Irp, 
  PKEVENT pev);

//==========================================================================//
// DriverEntry 
//   Purpose: The initial entry point for a WDM kernel device driver,
//            called at IRQL PASSIVE_LEVEL by the I/O Manager. 
//
//   Parameters:
//      IN DriverObject:  pointer to the driver object for the device
//      IN RegistryPath:  UniCode string with the driver's registry path name
//
//   Returns:
//      Status of the operation (STATUS_SUCCESS or failure code)
//==========================================================================//
NTSTATUS
DriverEntry (
  IN PDRIVER_OBJECT     DriverObject,
  IN PUNICODE_STRING    RegistryPath
  )
{
	NTSTATUS  status = STATUS_SUCCESS;
	PDEVICE_OBJECT        deviceObject = NULL;

	KdPrint (("s3_1000.sys: DriverEntry\n"));

	// initialize the driver dispatch table
	DriverObject->DriverExtension->AddDevice			= AddDevice;
	DriverObject->DriverUnload							= Unload;
	DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]	= DispatchPnP;
	DriverObject->MajorFunction[IRP_MJ_PNP]				= DispatchPnP;
	DriverObject->MajorFunction[IRP_MJ_POWER]			= DispatchPower;
	DriverObject->MajorFunction[IRP_MJ_CREATE]			= Dispatch;
	DriverObject->MajorFunction[IRP_MJ_CLOSE]			= Dispatch;
	DriverObject->MajorFunction[IRP_MJ_READ]			= Dispatch;
	DriverObject->MajorFunction[IRP_MJ_WRITE]			= Dispatch;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = Control;

	// This driver uses Driver Queuing, so we do not specify the StartIo
	// function as a Dispatch Entry point.  Driver queuing is required
	// since we can perform simultaneous Read/Write operations, and the
	// System queuing method allows only a SINGLE IRP to be active at
	// once.  Two IRPs must be active for a simultaneous read/write.
	//DriverObject->DriverStartIo							= StartIo;
  
	KdPrint (("s3_1000.sys: DriverEntry complete\n"));

	return status;
} // DriverEntry


//==========================================================================//
// Control 
//   Purpose: Called by the I/O Manager when
//            the IRP_MJ_DEVICE_CONTROL function code is encountered.
//            Implements all the device-specific PIO commands.
//
//   Parameters:
//      IN DeviceObject:  pointer to the device information structure
//      IN Irp:  pointer to the I/O Request Packet
//
//   Returns:
//      STATUS_SUCCESS or failure code
//==========================================================================//
NTSTATUS
Control (
  PDEVICE_OBJECT DeviceObject,
  PIRP Irp
  )
{
	ULONG				address;			// Register address
	PDEVICE_EXTENSION	deviceExtension;	// Device characteristics
	ULONG				index;				// LED Bank index
	PIO_STACK_LOCATION	ioStack;			// Pointer to the current IRP stack
	ULONG				length;				// Length of the data buffer
	ULONG				loopLimit;			// Prevents endless loops
	PULONG				pBuffer;			// Pointer to the unsigned long parameter value
	ULONG				regValue;			// 32 bit register value
	NTSTATUS			status;				// return code

	length = 0;

	status = STATUS_SUCCESS;
	deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

	// Get the current stack and data buffer locations.
	ioStack = IoGetCurrentIrpStackLocation (Irp);
	pBuffer = (PULONG) Irp->AssociatedIrp.SystemBuffer;

	// Set the Base Address Register block.
    switch (ioStack->Parameters.DeviceIoControl.IoControlCode)  
	{
	case IOCTL_INITIATOR_RESET:
		// Perform a reset of the Initiator device by setting the low bit of the Device
		// Control Register (DCR) high.
		KdPrint(("s3_1000.sys: IOCTL_INITIATOR_RESET\n"));
		address = (ULONG) deviceExtension->MemoryStart[FPGA] + DCR_OFFSET;
		length = 1;
		WRITE_REGISTER_ULONG ((PULONG) address, 0x1);

		// A reset happens prior to setting up any DMA transfer, so this is a good time
		// to also initialize the transfer length values in our device extension. 
		deviceExtension->ReadLength = 0;
		deviceExtension->WriteLength = 0;

		// Reset the interrupt monitor variables.
		deviceExtension->ReadDone = FALSE;
		deviceExtension->WriteDone = FALSE;

		break;

	case IOCTL_CLEAR_INITIATOR_RESET:
		// Clears a reset of the Initiator device by setting the low bit of the Device
		// Control Register (DCR) low, then setting it low again.
		KdPrint(("s3_1000.sys: IOCTL_CLEAR_INITIATOR_RESET\n"));
		address = (ULONG) deviceExtension->MemoryStart[FPGA] + DCR_OFFSET;
		length = 1;
		WRITE_REGISTER_ULONG ((PULONG) address, 0x0);
		break;

	case IOCTL_SET_DMA_WRITE_ADDR:
		// Writes the target address on the host for a DMA transfer to the Write DMA TLP Address Register.
		// The target host address is received in pBuffer.
		address = (ULONG) deviceExtension->MemoryStart[FPGA] + WRITE_ADDR_OFFSET;
		length = 1;
		
		// Set the DMA target address, ensuring that the value is aligned on a 4-byte boundary.
		WRITE_REGISTER_ULONG ((PULONG) address, *pBuffer & 0xFFFFFFFC);
		KdPrint(("s3_1000.sys: IOCTL_SET_DMA_WRITE_ADDR"));
		KdPrint(("   dma write addr = 0x%x\n", *(pBuffer) & 0xFFFFFFFC));
		break;

	case IOCTL_SET_DMA_WRITE_SIZE:
		// Sets the size in bytes of each TLP by writing the value to the Write DMA TLP Size Register.
		// The size is received in pBuffer, and is limited to the lower 12 bytes of the register.
		address = (ULONG) deviceExtension->MemoryStart[FPGA] + WRITE_SIZE_OFFSET;
		length = 1;

		// Set the new Write TLP size.
		regValue = READ_REGISTER_ULONG ((PULONG) address);		// Read the entire register to preserve the upper bits
		regValue &= 0xFFFFF000;									// Clear the lower 12 bits for the new size value
		regValue |= *(pBuffer) & 0xFFF;							// Set the new size
		WRITE_REGISTER_ULONG ((PULONG) address, *(pBuffer));	// Write the new register value
		KdPrint(("s3_1000.sys: IOCTL_SET_DMA_WRITE_SIZE"));
		KdPrint(("   dma write size = %d\n", regValue));
		break;

	case IOCTL_SET_DMA_WRITE_COUNT:
		// Sets the number of TLPs to transfer by writing the value to the Write DMA TLP Count Register.
		// The count is received in pBuffer, and is limited to the lower 16 bits of the register.
		address = (ULONG) deviceExtension->MemoryStart[FPGA] + WRITE_COUNT_OFFSET;
		length = 1;

		// Set the new Write TLP count.
		regValue = READ_REGISTER_ULONG ((PULONG) address);		// Read the entire register to preserve the upper bits
		regValue &= 0xFFFF0000;									// Clear the lower 16 bits for the new count value
		regValue |= *(pBuffer) & 0xFFFF;						// Set the new count
		WRITE_REGISTER_ULONG ((PULONG) address, *(pBuffer));	// Write the new register value
		KdPrint(("s3_1000.sys: IOCTL_SET_DMA_WRITE_COUNT"));
		KdPrint(("   dma write count = %d\n", regValue));
		break;

	case IOCTL_SET_DMA_WRITE_PATTERN:
		// Sets the data pattern to use in each TLP for subsequent verification on the host.
		// The hex pattern is received in pBuffer.
		address = (ULONG) deviceExtension->MemoryStart[FPGA] + WRITE_PATTERN_OFFSET;
		length = 1;
		WRITE_REGISTER_ULONG ((PULONG) address, *(pBuffer));	// Write the new register value from pBuffer
		KdPrint(("s3_1000.sys: IOCTL_SET_DMA_WRITE_PATTERN"));
		KdPrint(("   dma write pattern = 0x%x\n", *(pBuffer)));
		break;

	case IOCTL_SET_DMA_READ_ADDR:
		// Writes the target address on the host for a DMA transfer to the Write DMA TLP Address Register.
		// The target host address is received in pBuffer.
		address = (ULONG) deviceExtension->MemoryStart[FPGA] + READ_ADDR_OFFSET;
		length = 1;

		// Set the DMA target address, ensuring that the value is aligned on a 4-byte boundary.
		WRITE_REGISTER_ULONG ((PULONG) address, *(pBuffer) & 0xFFFFFFFC);
		KdPrint(("s3_1000.sys: IOCTL_SET_DMA_READ_ADDR"));
		KdPrint(("   dma read addr = 0x%x\n", *(pBuffer) & 0xFFFFFFFC));
		break;

	case IOCTL_SET_DMA_READ_SIZE:
		// Sets the size in bytes of each TLP by writing the value to the Write DMA TLP Size Register.
		// The size is received in pBuffer, and is limited to the lower 12 bytes of the register.
		address = (ULONG) deviceExtension->MemoryStart[FPGA] + READ_SIZE_OFFSET;
		length = 1;

		// Set the new Read TLP size.
		regValue = READ_REGISTER_ULONG ((PULONG) address);		// Read the entire register to preserve the upper bits
		regValue &= 0xFFFFF000;									// Clear the lower 12 bits for the new size value
		regValue |= *(pBuffer) & 0xFFF;							// Set the new size
		WRITE_REGISTER_ULONG ((PULONG) address, regValue);		// Write the new register value
		KdPrint(("s3_1000.sys: IOCTL_SET_DMA_READ_SIZE"));
		KdPrint(("   dma read size = %d\n", regValue));
		break;

	case IOCTL_SET_DMA_READ_COUNT:
		// Sets the number of TLPs to transfer by writing the value to the Write DMA TLP Count Register.
		// The count is received in pBuffer, and is limited to the lower 16 bits of the register.
		address = (ULONG) deviceExtension->MemoryStart[FPGA] + READ_COUNT_OFFSET;
		length = 1;

		// Set the new Read TLP count.
		regValue = READ_REGISTER_ULONG ((PULONG) address);		// Read the entire register to preserve the upper bits
		regValue &= 0xFFFF0000;									// Clear the lower 16 bits for the new count value
		regValue |= *(pBuffer) & 0xFFFF;						// Set the new count
		WRITE_REGISTER_ULONG ((PULONG) address, regValue);		// Write the new register value
		KdPrint(("s3_1000.sys: IOCTL_SET_DMA_READ_COUNT"));
		KdPrint(("   dma read count = %d\n", regValue));
		break;

	case IOCTL_SET_DMA_READ_PATTERN:
		// Sets the data pattern to use in each TLP for subsequent verification on the host.
		// The hex pattern is received in pBuffer.
		address = (ULONG) deviceExtension->MemoryStart[FPGA] + READ_PATTERN_OFFSET;
		length = 1;
		WRITE_REGISTER_ULONG ((PULONG) address, *(pBuffer));	// Write the new register value from pBuffer
		KdPrint(("s3_1000.sys: IOCTL_SET_DMA_READ_PATTERN"));
		KdPrint(("   dma read pattern = 0x%x\n", *(pBuffer)));
		break;

	case IOCTL_DMA_START:
		// Initiate any DMA transfers that have been set up in the Read/Write registers.
		status = STATUS_PENDING;
		LogMsg(DeviceObject, L"Control: PCIe BMDMA Transfer Starting");
		address = (ULONG) deviceExtension->MemoryStart[FPGA] + DCSR_OFFSET;
		length = 0;												// Nothing returned
		regValue = READ_REGISTER_ULONG ((PULONG) address);		// Read the entire register to preserve bits
		regValue |= *pBuffer;									// Set the new register bits passed in
		KdPrint(("s3_1000.sys: IOCTL_DMA_START"));
		KdPrint(("   dma start: pBuffer = 0x%x, New DCSR = 0x%x\n", *(pBuffer), regValue));
		WRITE_REGISTER_ULONG ((PULONG) address, regValue);		// Write the new register value
		break;

	case IOCTL_GET_DMA_STATUS:
		// Read the DMA status register and return the results.
		address = (ULONG) deviceExtension->MemoryStart[FPGA] + DCSR_OFFSET;
		length = 1;
		*pBuffer = READ_REGISTER_ULONG ((PULONG) address);
		KdPrint(("s3_1000.sys: IOCTL_GET_DMA_STATUS"));
		KdPrint(("   dma status = 0x%x\n", *(pBuffer)));
		break;

	case IOCTL_GET_DMA_WRITE_PERF:
		// Read the DMA Write Performance register and return the results.
		address = (ULONG) deviceExtension->MemoryStart[FPGA] + WRITE_DMA_PERF_OFFSET;
		length = 1;
		*pBuffer = READ_REGISTER_ULONG ((PULONG) address);
		KdPrint(("s3_1000.sys: IOCTL_GET_DMA_WRITE_PERF"));
		KdPrint(("   dma write performance = 0x%x\n", *(pBuffer)));
		break;

	case IOCTL_GET_DMA_READ_PERF:
		// Read the DMA Read Performance register and return the results.
		address = (ULONG) deviceExtension->MemoryStart[FPGA] + READ_DMA_PERF_OFFSET;
		length = 1;
		*pBuffer = READ_REGISTER_ULONG ((PULONG) address);
		KdPrint(("s3_1000.sys: IOCTL_GET_DMA_READ_PERF"));
		KdPrint(("   dma read performance = 0x%x\n", *(pBuffer)));
		break;

	case IOCTL_GET_DMA_WRITE_ADDR:
		// Read the DMA Write Address register and return the results.
		address = (ULONG) deviceExtension->MemoryStart[FPGA] + WRITE_ADDR_OFFSET;
		length = 1;
		*pBuffer = READ_REGISTER_ULONG ((PULONG) address) & 0xFFFFFFFC;		// 4-byte boundary
		KdPrint(("s3_1000.sys: IOCTL_GET_DMA_WRITE_ADDR"));
		KdPrint(("   dma write addr = 0x%x\n", *(pBuffer) ));
		break;
     
	case IOCTL_GET_DMA_WRITE_SIZE:
		// Read the DMA Write TLP Size register and return the results.
		address = (ULONG) deviceExtension->MemoryStart[FPGA] + WRITE_SIZE_OFFSET;
		length = 1;
		*pBuffer = READ_REGISTER_ULONG ((PULONG) address) & 0xFFF;			// Lower 12 bits only
		KdPrint(("s3_1000.sys: IOCTL_GET_DMA_WRITE_SIZE"));
		KdPrint(("   dma write size = %d\n", *(pBuffer)));
		break;
     
	case IOCTL_GET_DMA_WRITE_COUNT:
		// Read the DMA Write TLP Count register and return the results.
		address = (ULONG) deviceExtension->MemoryStart[FPGA] + WRITE_COUNT_OFFSET;
		length = 1;
		*pBuffer = READ_REGISTER_ULONG ((PULONG) address) & 0xFFFF;			// Lower 16 bits only
		KdPrint(("s3_1000.sys: IOCTL_GET_DMA_WRITE_COUNT"));
		KdPrint(("   dma write count = %d\n", *(pBuffer)));
		break;
     
	case IOCTL_GET_DMA_WRITE_PATTERN:
		// Read the DMA Write Pattern register and return the results.
		address = (ULONG) deviceExtension->MemoryStart[FPGA] + WRITE_PATTERN_OFFSET;
		length = 1;
		*pBuffer = READ_REGISTER_ULONG ((PULONG) address);	
		KdPrint(("s3_1000.sys: IOCTL_GET_DMA_WRITE_PATTERN"));
		KdPrint(("   dma write pattern = 0x%x\n", *(pBuffer)));	
		break;
     
	case IOCTL_GET_DMA_READ_ADDR:
		// Read the DMA Read Address register and return the results.
		address = (ULONG) deviceExtension->MemoryStart[FPGA] + READ_ADDR_OFFSET;
		length = 1;
		*pBuffer = READ_REGISTER_ULONG ((PULONG) address) & 0xFFFFFFFC;		// 4-byte boundary
		KdPrint(("s3_1000.sys: IOCTL_GET_DMA_READ_ADDR"));
		KdPrint(("   dma read addr = 0x%x\n", *(pBuffer) ));
		break;
     
	case IOCTL_GET_DMA_READ_SIZE:
		// Read the DMA Read TLP Size register and return the results.
		address = (ULONG) deviceExtension->MemoryStart[FPGA] + READ_SIZE_OFFSET;
		length = 1;
		*pBuffer = READ_REGISTER_ULONG ((PULONG) address) & 0xFFF;			// Lower 12 bits only
		KdPrint(("s3_1000.sys: IOCTL_GET_DMA_READ_SIZE"));
		KdPrint(("   dma read size = %d\n", *(pBuffer)));
		break;
     
	case IOCTL_GET_DMA_READ_COUNT:
		// Read the DMA Read TLP Count register and return the results.
		address = (ULONG) deviceExtension->MemoryStart[FPGA] + READ_COUNT_OFFSET;
		length = 1;
		*pBuffer = READ_REGISTER_ULONG ((PULONG) address) & 0xFFFF;			// Lower 16 bits only
		KdPrint(("s3_1000.sys: IOCTL_GET_DMA_READ_COUNT"));
		KdPrint(("   dma read count = %d\n", *(pBuffer)));
		break;
     
	case IOCTL_GET_DMA_READ_PATTERN:
		// Read the DMA Read Pattern register and return the results.
		address = (ULONG) deviceExtension->MemoryStart[FPGA] + READ_PATTERN_OFFSET;
		length = 1;
		*pBuffer = READ_REGISTER_ULONG ((PULONG) address);		
		KdPrint(("s3_1000.sys: IOCTL_GET_DMA_READ_PATTERN"));
		KdPrint(("   dma read pattern = 0x%x\n", *(pBuffer)));
		break;
     
	case IOCTL_GET_DMA_REGISTER:
		// Read the DMA register by using the index to calculate its byte offset.
		address = (ULONG) deviceExtension->MemoryStart[FPGA] + (*pBuffer * 4);
		length = 1;
		*pBuffer = READ_REGISTER_ULONG ((PULONG) address);
		KdPrint(("s3_1000.sys: IOCTL_GET_DMA_REGISTER"));
		KdPrint(("   register[0x%x] = 0x%x\n", address & 0x000000FF, *(pBuffer)));
		break;
          
	case IOCTL_CLEANUP:
		// RRW - this function is a work-around for the failure to recognize interrupts.  It
		// is called from the application when the app determines by reading the status register
		// that a transfer has completed.  In essence, the app is "interrupting".
		// This control code should be removed when interrupts start working, as BDMA_Isr has
		// already been registered with the OS as the interrupt handler.
        // HJK - This function now does nothing since the interrupts are recognized.
		KdPrint(("s3_1000.sys: IOCTL_CLEANUP\n"));
		length = 1;
		break;

	case IOCTL_DISABLE_INTERRUPTS:
		// Turn off write/read completion interrupt generation in the PCIe HW.
		address = (ULONG) deviceExtension->MemoryStart[FPGA] + DCSR_OFFSET;
		length = 1;

		// Disable interrupts while preserving all other bits.
		regValue = READ_REGISTER_ULONG ((PULONG) address);		// Read the entire register to preserve the bits
		regValue |= 0x800080;									// Set the interrupt disable bits.
		WRITE_REGISTER_ULONG ((PULONG) address, regValue);		// Write the new register value
		KdPrint(("s3_1000.sys: IOCTL_DISABLE_INTERRUPTS\n"));
		break;

	case IOCTL_ENABLE_INTERRUPTS:
		// Turn on write/read completion interrupt generation in the PCIe HW.  This is the default
		// setting in the hardware.
		address = (ULONG) deviceExtension->MemoryStart[FPGA] + DCSR_OFFSET;
		length = 1;

		// Enable interrupts while preserving all other bits.
		regValue = READ_REGISTER_ULONG ((PULONG) address);		// Read the entire register to preserve the bits
		regValue &= 0xFF7FFF7F;									// Clear the interrupt disable bits.
		WRITE_REGISTER_ULONG ((PULONG) address, regValue);		// Write the new register value
		KdPrint(("s3_1000.sys: IOCTL_ENABLE_INTERRUPTS\n"));
		break;

   	default:
		// Set failure status for unrecognized control codes.
		KdPrint(("s3_1000.sys: Received invalid I/O Control code.\n"));
		status = STATUS_NOT_IMPLEMENTED;
	}
	
    if(status != STATUS_PENDING)
    {
	   Irp->IoStatus.Status = status;
	   Irp->IoStatus.Information = length * sizeof(ULONG);		// This MUST be set to the size of the return value.

	   //	Tell the I/0 Manager that everything has been done.
       IoCompleteRequest( Irp, IO_NO_INCREMENT );
    }

	return status;
}

//==========================================================================//
// Dispatch 
//   Purpose: Called by the I/O Manager when a dispatch function code
//            is encountered.
//            Implements Create, Close, Read and Write.
//
//   Parameters:
//      IN DeviceObject:  pointer to the device information structure
//      IN Irp:  pointer to the I/O Request Packet
//
//   Returns:
//      STATUS_SUCCESS or failure code
//==========================================================================//
// IRQL PASSIVE_LEVEL
NTSTATUS
Dispatch (
  IN PDEVICE_OBJECT     DeviceObject,
  IN PIRP               Irp
  )
{
	PDEVICE_EXTENSION   deviceExtension;	// device characteristics
	PIO_STACK_LOCATION	ioStack;			// Pointer to the current IRP stack
	ULONG				regAddr;			// Address of the DMA Address register
	NTSTATUS			status;				// completion code
	KIRQL				oldIrql;
	
	PDRIVER_CANCEL rcancel = ReadCancelRoutine;
	PDRIVER_CANCEL wcancel = WriteCancelRoutine;
	NTSTATUS abortstatus;

	// Start by assuming success and deferred processing.
	status = STATUS_PENDING;

	// Retrieve our IRP and device characteristics.
	ioStack = IoGetCurrentIrpStackLocation(Irp);
	deviceExtension = DeviceObject->DeviceExtension;

	// Select the operation and set the transfer parameters.
	switch(ioStack->MajorFunction)
	{
	case IRP_MJ_CREATE:
		LogMsg(DeviceObject, L"Dispatch: PCIe DMA function v.0.7.0 created");
		KdPrint(("s3_1000.sys: IRP_MJ_CREATE\n"));
        
        // Start the IO timer to prevent hang if interrupt
        // not received from the PCIe HW			
		deviceExtension->timer = -1;
		IoStartTimer(DeviceObject);				
        
		Irp->IoStatus.Status = STATUS_SUCCESS;
		Irp->IoStatus.Information = 0;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
		return STATUS_SUCCESS;

	case IRP_MJ_CLOSE:
		LogMsg(DeviceObject, L"Dispatch: PCIe DMA function closed");
		KdPrint(("s3_1000.sys IRP_MJ_CLOSE\n"));
        
        // Stop IO timer when exiting from task
		IoStopTimer(DeviceObject);
        
		Irp->IoStatus.Status = STATUS_SUCCESS;
		Irp->IoStatus.Information = 0;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
		return STATUS_SUCCESS;

	case	IRP_MJ_READ:
		LogMsg(DeviceObject, L"Dispatch: PCIe BMDMA Read Request received.");
		KdPrint(("s3_1000.sys: IRP_MJ_READ\n"));
		deviceExtension->ReadTotalLength = ioStack->Parameters.Read.Length;

		// Is the transfer size within our prescribed limits?
		if (deviceExtension->ReadTotalLength > MAX_TRANSFER_SIZE)
		{	
			status = STATUS_BUFFER_TOO_SMALL;
		}

		else if (deviceExtension->ReadTotalLength == 0)
		{
			status = STATUS_SUCCESS;
			LogMsg(DeviceObject, L"Dispatch: Exiting, nothing to read");
		}

		// Has the OS allocated mapped our user buffer?
		else if (Irp->MdlAddress == NULL)
		{
			status = STATUS_NOT_IMPLEMENTED;

		}

		// Exit if we found any errors.
		if (status != STATUS_PENDING)
		{
			// Complete the I/O Request.
			Irp->IoStatus.Status = status;
			Irp->IoStatus.Information = 0;
			IoCompleteRequest(Irp, IO_NO_INCREMENT);

			return status;
		}

		// Initialize the offset value.
		deviceExtension->ReadOffset = 0;
		
		// Take out the Read list lock, since we'll be inserting this request on our
		// queue.  We need to protect any queue changes since the queue can be accessed
		// by a DPCforISR, which could take priority over a Dispatch function.
		KeAcquireSpinLock(&deviceExtension->ReadQueueLock, &oldIrql);
		
        ASSERT(rcancel);				// must supply a cancel routine
	    ASSERT(Irp->CancelRoutine == NULL); // maybe left over from a higher level?

	   // If the device has been removed by surprise, complete IRP immediately.
	   abortstatus = deviceExtension->readabortstatus;
	   if (abortstatus)
	   {					
           // aborting all requests now
		   status = abortstatus;
		   KeReleaseSpinLock(&deviceExtension->ReadQueueLock, oldIrql);
		   Irp->IoStatus.Status = abortstatus;
		   IoCompleteRequest(Irp, IO_NO_INCREMENT);
	   } 

	   // If the device is busy with another request, or if the queue has
	   // been stalled due to some PnP or power event, just put the new IRP
	   // onto the queue and set a cancel routine pointer.
	   else if (deviceExtension->ReadIrp || deviceExtension->readstallcount)
	   {					
		   // See if this IRP was cancelled before it got to us. If so,
		   // make sure either we or the cancel routine completes it
		   IoSetCancelRoutine(Irp, rcancel);
		   if (Irp->Cancel && IoSetCancelRoutine(Irp, NULL))
		   {					
               // IRP has already been cancelled so complete the request
			   status = STATUS_CANCELLED;
			   KeReleaseSpinLock(&deviceExtension->ReadQueueLock, oldIrql);
			   Irp->IoStatus.Status = STATUS_CANCELLED;
			   IoCompleteRequest(Irp, IO_NO_INCREMENT);
		   }				
		   else
		   {					
            // queue IRP
			   InsertTailList(&deviceExtension->ReadQueue, &Irp->Tail.Overlay.ListEntry);
			   LogMsg(DeviceObject, L"Dispatch: New read request queued.");
		   }					
	   }					

	   // If the device is idle and not stalled, pass the IRP to the StartDmaRead
	   // routine associated with this queue
       else
	   {						
	       // start this irp
		   status = StartDMARead(DeviceObject, Irp);
	   }						

	   // Release the Interlock.
	   KeReleaseSpinLock(&deviceExtension->ReadQueueLock, oldIrql);

	   // Mark the Irp for deferred processing, if there were no problems in StartDMARead.
	   if (status == STATUS_PENDING)
	   {
		  Irp->IoStatus.Status = status;
		  Irp->IoStatus.Information = 0;
		  IoMarkIrpPending(Irp);
	   }
	   break;

	case	IRP_MJ_WRITE:
		LogMsg(DeviceObject, L"Dispatch: PCIe BMDMA Write Request received.");
		KdPrint(("s3_1000.sys: IRP_MJ_WRITE\n"));
		deviceExtension->WriteTotalLength = ioStack->Parameters.Write.Length;

		// Is the transfer size within our prescribed limits?
		if (deviceExtension->WriteTotalLength > MAX_TRANSFER_SIZE)
		{	
			status = STATUS_BUFFER_TOO_SMALL;
		}

		else if (deviceExtension->WriteTotalLength == 0)
		{
			status = STATUS_SUCCESS;
			LogMsg(DeviceObject, L"Dispatch: Exiting, nothing to write");
		}

		// Has the OS allocated mapped our user buffer?
		else if (Irp->MdlAddress == NULL)
		{
			status = STATUS_NOT_IMPLEMENTED;
		}

		// Exit if we found any errors.
		if (status != STATUS_PENDING)
		{
			// Complete the I/O Request.
			Irp->IoStatus.Status = status;
			Irp->IoStatus.Information = 0;
			IoCompleteRequest(Irp, IO_NO_INCREMENT);

			return status;
		}

		// Initialize the offset value.
		deviceExtension->WriteOffset = 0;
		
		// Take out the Write list lock, since we'll be inserting this request on our
		// queue.  We need to protect any queue changes since the queue can be accessed
		// by a DPCforISR, which could take priority over a Dispatch function.
		KeAcquireSpinLock(&deviceExtension->WriteQueueLock, &oldIrql);
		
        ASSERT(wcancel);				// must supply a cancel routine
	    ASSERT(Irp->CancelRoutine == NULL); // maybe left over from a higher level?

	    // If the device has been removed by surprise, complete IRP immediately.
	    abortstatus = deviceExtension->writeabortstatus;
	    if (abortstatus)
		{					
           // aborting all requests now
		   status = abortstatus;
		   KeReleaseSpinLock(&deviceExtension->WriteQueueLock, oldIrql);
		   Irp->IoStatus.Status = abortstatus;
		   IoCompleteRequest(Irp, IO_NO_INCREMENT);
		} 

	    // If the device is busy with another request, or if the queue has
	    // been stalled due to some PnP or power event, just put the new IRP
	    // onto the queue and set a cancel routine pointer.
	    else if (deviceExtension->WriteIrp || deviceExtension->writestallcount)
		{					
		   // See if this IRP was cancelled before it got to us. If so,
		   // make sure either we or the cancel routine completes it
		   IoSetCancelRoutine(Irp, wcancel);
		   if (Irp->Cancel && IoSetCancelRoutine(Irp, NULL))
		   {					
              // IRP has already been cancelled
			  status = STATUS_CANCELLED;
			  KeReleaseSpinLock(&deviceExtension->WriteQueueLock, oldIrql);
			  Irp->IoStatus.Status = STATUS_CANCELLED;
			  IoCompleteRequest(Irp, IO_NO_INCREMENT);
		   }				
		   else
		   {					
              // queue IRP
			  InsertTailList(&deviceExtension->WriteQueue, &Irp->Tail.Overlay.ListEntry);
			  LogMsg(DeviceObject, L"Dispatch: New write request queued.");
		   }					
		}					

	    // If the device is idle and not stalled, pass the IRP to the StartDMAWrite
	    // routine associated with this queue
        else
		{						
           // start this irp
		   status = StartDMAWrite(DeviceObject, Irp);
		}						// start this irp
 
		// Release the Interlock.
		KeReleaseSpinLock(&deviceExtension->WriteQueueLock, oldIrql);

		// Mark the Irp for deferred processing, if there were no problems in StartDMAWrite.
		if (status == STATUS_PENDING)
		{
			Irp->IoStatus.Status = status;
			Irp->IoStatus.Information = 0;
			IoMarkIrpPending(Irp);
		}
		break;

	default:	// Invalid 
	    KdPrint(("s3_1000.sys IRP_MJ_NOT_IMPLEMENTED\n"));
		status = STATUS_NOT_IMPLEMENTED;
		Irp->IoStatus.Status = status;
		Irp->IoStatus.Information = 0;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
		break;
	}
 
	return status;
} // Dispatch

 
//==========================================================================
// The read cancel routine
//==========================================================================
VOID ReadCancelRoutine (
   PDEVICE_OBJECT DeviceObject,
   PIRP Irp)
   {
       PDEVICE_EXTENSION devExt =  
          (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
       CancelReadRequest(devExt, Irp);
   }
   
//==========================================================================
// The write cancel routine
//==========================================================================
VOID WriteCancelRoutine (
   PDEVICE_OBJECT DeviceObject,
   PIRP Irp)
   {
       PDEVICE_EXTENSION devExt =  
          (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
       CancelWriteRequest(devExt, Irp);
   }
  
//=========================================================================
// Cancel routines
//=========================================================================
VOID CancelReadRequest(
   PDEVICE_EXTENSION pdevice_extension, 
   PIRP Irp)
	{						
	   KIRQL oldirql = Irp->CancelIrql;

	   // Release the global cancel spin lock as soon as possible
	   IoReleaseCancelSpinLock(DISPATCH_LEVEL);

	   // Acquire our queue-specific queue lock. Note that we stayed at DISPATCH_LEVEL
	   // when we released the cancel spin lock
	   KeAcquireSpinLockAtDpcLevel(&pdevice_extension->ReadQueueLock);

	   // The IRP is guaranteed to be on *some* queue (maybe a degenerate one),
	   // so we unconditionally remove it and complete it.
	   RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
	   KeReleaseSpinLock(&pdevice_extension->ReadQueueLock, oldirql);

	   Irp->IoStatus.Status = STATUS_CANCELLED;
	   IoCompleteRequest(Irp, IO_NO_INCREMENT);
	} // end of CancelReadRequest()
   
   
VOID CancelWriteRequest(
   PDEVICE_EXTENSION pdevice_extension, 
   PIRP Irp)
	{						
	   KIRQL oldirql = Irp->CancelIrql;

	   // Release the global cancel spin lock as soon as possible
	   IoReleaseCancelSpinLock(DISPATCH_LEVEL);

	   // Acquire our queue-specific queue lock. Note that we stayed at DISPATCH_LEVEL
	   // when we released the cancel spin lock
	   KeAcquireSpinLockAtDpcLevel(&pdevice_extension->WriteQueueLock);

	   // The IRP is guaranteed to be on *some* queue (maybe a degenerate one),
	   // so we unconditionally remove it and complete it.
	   RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
	   KeReleaseSpinLock(&pdevice_extension->WriteQueueLock, oldirql);

	   Irp->IoStatus.Status = STATUS_CANCELLED;
	   IoCompleteRequest(Irp, IO_NO_INCREMENT);
	} // end of CancelWriteRequest()
 
//==========================================================================
// Code to handle completion of active IRPs
//==========================================================================
 
VOID WaitForCurrentIrp(
   PDEVICE_EXTENSION DeviceExtension,
   BOOLEAN read)
	{							
       KIRQL oldirql;
       BOOLEAN mustwait;

       if(read == TRUE)
	   {
          // First reset the event that DpcForIsr sets each time
	      KeClearEvent(&DeviceExtension->readevStop);

	      // Under protection of our spin lock, check to see if there's a current IRP.
	      // Since whoever called us should also have stalled requests, no-one can sneak
	      // in after we release the spin lock and start a new request behind our back.
	      ASSERT(DeviceExtension->readstallcount != 0);	// should be stalled now!
	
	      KeAcquireSpinLock(&DeviceExtension->ReadQueueLock, &oldirql);
	      mustwait = DeviceExtension->ReadIrp != NULL;
	      KeReleaseSpinLock(&DeviceExtension->ReadQueueLock, oldirql);

	      if (mustwait)
		  {
		      KeWaitForSingleObject(&DeviceExtension->readevStop, Executive, KernelMode, FALSE, NULL);
		  }
	   }
	   else
	   {
          // First reset the event that DpcForIsr sets each time
	      KeClearEvent(&DeviceExtension->writeevStop);

	      // Under protection of our spin lock, check to see if there's a current IRP.
	      // Since whoever called us should also have stalled requests, no-one can sneak
	      // in after we release the spin lock and start a new request behind our back.
	      ASSERT(DeviceExtension->writestallcount != 0);	// should be stalled now!
	
	      KeAcquireSpinLock(&DeviceExtension->WriteQueueLock, &oldirql);
	      mustwait = DeviceExtension->WriteIrp != NULL;
	      KeReleaseSpinLock(&DeviceExtension->WriteQueueLock, oldirql);

	      if (mustwait)
		  {
		      KeWaitForSingleObject(&DeviceExtension->writeevStop, Executive, KernelMode, FALSE, NULL);
		  }
	   }
	} // end of WaitForCurrentIrp()
 
//==========================================================================
// code to handle stalling requests
//==========================================================================
VOID StallRequests(
   PDEVICE_EXTENSION deviceExtension,
   BOOLEAN read)
	{			
       if (read == TRUE)			
	   InterlockedIncrement(&deviceExtension->readstallcount);
       else
	   InterlockedIncrement(&deviceExtension->writestallcount);
	} // end of StallRequests()
 
//==========================================================================
// restart requests that were stalled
//==========================================================================

VOID RestartRequests(
   PDEVICE_EXTENSION deviceExtension, 
   PDEVICE_OBJECT pDeviceObject,
   BOOLEAN read)
   {		
	   KIRQL oldirql;
	   if(read == TRUE)
	   { 
	      KIRQL oldirql;
	      KeAcquireSpinLock(&deviceExtension->ReadQueueLock, &oldirql);

	      if (InterlockedDecrement(&deviceExtension->readstallcount) > 0)
		  {						// still stalled
		     KeReleaseSpinLock(&deviceExtension->ReadQueueLock, oldirql);
		     return;
		  }						// still stalled

	      // Dequeue and start the IRP at the head of the list.  
	      while (!deviceExtension->readstallcount && !deviceExtension->ReadIrp &&
		         !deviceExtension->readabortstatus && !IsListEmpty(&deviceExtension->ReadQueue))
	      {	 
			 // start first queued IRP
		     PLIST_ENTRY next = RemoveHeadList(&deviceExtension->ReadQueue);
		     PIRP Irp = CONTAINING_RECORD(next, IRP, Tail.Overlay.ListEntry);

		     if (!IoSetCancelRoutine(Irp, NULL))
			 {					
				 // IRP being cancelled right now
			     ASSERT(Irp->Cancel);	// else CancelRoutine shouldn't be NULL!
			     InitializeListHead(&Irp->Tail.Overlay.ListEntry);
			     continue;		// with "start first queued IRP"
			 }	// IRP being cancelled right now

             // Ahh - found one not being canceled
		     deviceExtension->ReadIrp = Irp;
		     KeReleaseSpinLockFromDpcLevel(&deviceExtension->ReadQueueLock);
             
			 // start the IRP
		     (VOID)StartDMARead(pDeviceObject, Irp);
		     KeLowerIrql(oldirql);
		     return;
	      }						// start first queued IRP

	      // No read IRPs need to be started (or else all queued IRPs were being cancelled)
	      KeReleaseSpinLock(&deviceExtension->ReadQueueLock, oldirql);
	   }
	   else
	   {
	      KIRQL oldirql;
	      KeAcquireSpinLock(&deviceExtension->WriteQueueLock, &oldirql);

	      if (InterlockedDecrement(&deviceExtension->writestallcount) > 0)
		  {						// still stalled
		     KeReleaseSpinLock(&deviceExtension->WriteQueueLock, oldirql);
		     return;
		  }						// still stalled

	      // Dequeue and start the IRP at the head of the list.  
	      while (!deviceExtension->writestallcount && !deviceExtension->WriteIrp &&
		         !deviceExtension->writeabortstatus && !IsListEmpty(&deviceExtension->WriteQueue))
	      {	 
			 // start first queued IRP
		     PLIST_ENTRY next = RemoveHeadList(&deviceExtension->WriteQueue);
		     PIRP Irp = CONTAINING_RECORD(next, IRP, Tail.Overlay.ListEntry);

		     if (!IoSetCancelRoutine(Irp, NULL))
			 {					
				 // IRP being cancelled right now
			     ASSERT(Irp->Cancel);	// else CancelRoutine shouldn't be NULL!
			     InitializeListHead(&Irp->Tail.Overlay.ListEntry);
			     continue;		// with "start first queued IRP"
			 }	// IRP being cancelled right now

             // Ahh - found one not being canceled
		     deviceExtension->WriteIrp = Irp;
		     KeReleaseSpinLockFromDpcLevel(&deviceExtension->WriteQueueLock);
             
			 // start the IRP
		     (VOID)StartDMAWrite(pDeviceObject, Irp);
		     KeLowerIrql(oldirql);
		     return;
	      }						// start first queued IRP

	      // No write IRPs need to be started (or else all queued IRPs were being cancelled)
	      KeReleaseSpinLock(&deviceExtension->WriteQueueLock, oldirql);
	   }
   } // end of RestartRequests()
 

//==========================================================================
// Clean up requests
//==========================================================================

VOID CleanupRequests(
   PDEVICE_EXTENSION deviceExtension, 
   PFILE_OBJECT fop, 
   NTSTATUS status,
   BOOLEAN read)
{							
   LIST_ENTRY cancellist;
   KIRQL oldirql;
   PLIST_ENTRY first;
   PLIST_ENTRY next;
   PIRP Irp;
   PIO_STACK_LOCATION stack;
   PLIST_ENTRY current;
   InitializeListHead(&cancellist);
   if(read == TRUE)
   {
	   // Create a list of IRPs that belong to the same file object
       // Note: fop is not used in this driver
	
	   KeAcquireSpinLock(&deviceExtension->ReadQueueLock, &oldirql);

	   first = &deviceExtension->ReadQueue;
	
	   for (next = first->Flink; next != first; )
	   {						
          // for each queued IRP
		  Irp = CONTAINING_RECORD(next, IRP, Tail.Overlay.ListEntry);
		  stack = IoGetCurrentIrpStackLocation(Irp);

		  // Follow the chain to the next IRP now (so that the next iteration of
		  // the loop is properly setup whether we dequeue this IRP or not)
		  current = next;
		  next = next->Flink;

		  // Skip this IRP if it's not for the same file object as the
		  // current IRP_MJ_CLEANUP.
		  if (fop && stack->FileObject != fop)
          {
		     continue;			// not for same file object
          }

		 // Set the CancelRoutine pointer to NULL. If it was already NULL, 
         // someone is trying to cancel this IRP right now, so just leave it on
         // the queue and let them do it as soon as we release the spin lock.
		  if (!IoSetCancelRoutine(Irp, NULL))
          {
		     continue;
          }
		  RemoveEntryList(current);
		  InsertTailList(&cancellist, current);
	   }	// for each queued IRP

	   // Release the spin lock. We're about to undertake a potentially time-consuming
	   // operation that might conceivably result in a deadlock if we keep the lock.
	   KeReleaseSpinLock(&deviceExtension->ReadQueueLock, oldirql);

	   // Complete the selected requests.
	   while (!IsListEmpty(&cancellist))
	   {						
          // cancel selected requests
		  next = RemoveHeadList(&cancellist);
		  Irp = CONTAINING_RECORD(next, IRP, Tail.Overlay.ListEntry);
		  Irp->IoStatus.Status = status;
		  IoCompleteRequest(Irp, IO_NO_INCREMENT);
	   }	
   }
   else // write requests
   {
	   // Create a list of IRPs that belong to the same file object
       // Note: fop is not used in this driver
	
	   KeAcquireSpinLock(&deviceExtension->WriteQueueLock, &oldirql);

	   first = &deviceExtension->WriteQueue;
	
	   for (next = first->Flink; next != first; )
	   {						
          // for each queued IRP
		  Irp = CONTAINING_RECORD(next, IRP, Tail.Overlay.ListEntry);
		  stack = IoGetCurrentIrpStackLocation(Irp);

		  // Follow the chain to the next IRP now (so that the next iteration of
		  // the loop is properly setup whether we dequeue this IRP or not)
		  current = next;
		  next = next->Flink;

		  // Skip this IRP if it's not for the same file object as the
		  // current IRP_MJ_CLEANUP.
		  if (fop && stack->FileObject != fop)
          {
		     continue;			// not for same file object
          }

		 // Set the CancelRoutine pointer to NULL. If it was already NULL, 
         // someone is trying to cancel this IRP right now, so just leave it on
         // the queue and let them do it as soon as we release the spin lock.
		  if (!IoSetCancelRoutine(Irp, NULL))
          {
		     continue;
          }
		  RemoveEntryList(current);
		  InsertTailList(&cancellist, current);
	   }	// for each queued IRP

	   // Release the spin lock. We're about to undertake a potentially time-consuming
	   // operation that might conceivably result in a deadlock if we keep the lock.
	   KeReleaseSpinLock(&deviceExtension->WriteQueueLock, oldirql);

	   // Complete the selected requests.
	   while (!IsListEmpty(&cancellist))
	   {						
          // cancel selected requests
		  next = RemoveHeadList(&cancellist);
		  Irp = CONTAINING_RECORD(next, IRP, Tail.Overlay.ListEntry);
		  Irp->IoStatus.Status = status;
		  IoCompleteRequest(Irp, IO_NO_INCREMENT);
	   }	
   }					
} // end of CleanupRequests()

//==========================================================================
// Dispatch cleanup requests
//==========================================================================

VOID AbortRequests(
   PDEVICE_EXTENSION deviceExtension, 
   NTSTATUS status,
   BOOLEAN read)
{		
   if(read == TRUE)				
   {
	   deviceExtension->readabortstatus = status;
	   CleanupRequests(deviceExtension, NULL, status, TRUE);
   }
   else
   {
	   deviceExtension->writeabortstatus = status;
	   CleanupRequests(deviceExtension, NULL, status, FALSE);
   }
   
} // end of AbortRequests()

//===========================================================================
// Request helper routines
//===========================================================================

VOID AllowRequests(
   PDEVICE_EXTENSION deviceExtension)
	{							
	   deviceExtension->readabortstatus = STATUS_SUCCESS;
	   deviceExtension->writeabortstatus = STATUS_SUCCESS;
	} // end of AllowRequests()

 

BOOLEAN 
OkToStop(
        PDEVICE_EXTENSIONdeviceExtension)
		{
			// There should be no problem stopping device
			return TRUE;
		}// end of OkToStop
		
BOOLEAN 
OkToRemove(
        PDEVICE_EXTENSIONdeviceExtension)
		{
			// There should be no problem removing device
			return TRUE;
		}


//===========================================================================
// LogMsg
//   Purpose:  Sends a message to the System Event Log.  Note that callers
//             of this function must be running at IRQL <= DISPATCH_LEVEL,
//             due to the constraints of "IoWriteErrorLogEntry".
//
//   Parameters:
//       IN deviceObject: pointer to the device information structure
//       IN logMsg: pointer to a wide char array containing the message.
//
//   Returns:
//       None.
//===========================================================================
VOID
LogMsg(
	IN PDEVICE_OBJECT deviceObject,		//  Device Object
	IN const wchar_t *logMsg			//  Message for event log
	)
{
	PIO_ERROR_LOG_PACKET	errorLogEntry;
	ULONG					len;

	// Verify that we can handle the current IRQL level.  If not, 
	// do nothing and return, as recommended in the IoWriteErrorLogEntry
	// documentation.
	if (KeGetCurrentIrql() > DISPATCH_LEVEL)
		return;

	// Verify that we have a device object.  If not, just return. 
	if (deviceObject == NULL)
		return;

	// Calculate the message length.  These should be kept short, as there is
	// typically only room for about 52 wchar_t characters (104 bytes) for both
	// DumpData and insertion strings.  This example does not supply any dumpdata,
	// so we can use the available space for a message.  Anything beyond the 
	// allocated space will be truncated in the log.
	len = wcslen(logMsg)*sizeof(WCHAR) + sizeof(WCHAR);

		// Ensure this message is not too large;  truncate it if it is.  I'm not
	// sure why the ERROR_LOG_MAXIMUM_SIZE is so small, but on my development
	// system it turns out to be only 152 bytes.  Given that the IO_ERROR_LOG_PACKET
	// is 48 bytes long, that leaves only 104 bytes for dump data and insertion strings.
	if (sizeof(IO_ERROR_LOG_PACKET) + len > ERROR_LOG_MAXIMUM_SIZE)
		len = ERROR_LOG_MAXIMUM_SIZE - sizeof(IO_ERROR_LOG_PACKET) - 1;

	// Allocate the log packet and fill in the required information.
	errorLogEntry  = (PIO_ERROR_LOG_PACKET)
		IoAllocateErrorLogEntry(deviceObject, 
			(UCHAR)(sizeof(IO_ERROR_LOG_PACKET) + len));

	if (errorLogEntry == NULL)		// Continue as per WDM spec.  Maybe next time 
		return;						// the logmsg for this will work.

	errorLogEntry->ErrorCode = (ULONG) MSGLOG_INFORMATION;
	errorLogEntry->NumberOfStrings = 1;
	errorLogEntry->StringOffset = 
		FIELD_OFFSET(IO_ERROR_LOG_PACKET, DumpData);

	// Copy the insertion string to the packet.  The way the DumpData field works is that
	// it is declared as an array of 1 as the last field in the IO_ERROR_LOG_PACKET structure. 
	// IoAllocateErrorLogEntry is used to allocate n bytes of space for the packet, which includes
	// extra space for dumpdata and insertion strings at the end (up to the limit specified by
	// ERROR_LOG_MAXIMUM_SIZE).  So in our case we have no dump data, but we allocated space beginning
	// at the DumpData field for our insertion string where we now copy it in.  This is why it may
	// appear at first glance as if we are copying to memory we didn't allocate, but in fact the allocation
	// occurs in IoAllocateErrorLogEntry above.
	RtlCopyMemory(&errorLogEntry->DumpData[0], logMsg, len);

	// Write the packet to the event log.  This also frees memory allocated for
	// DumpData and Insertion strings.
	IoWriteErrorLogEntry(errorLogEntry);
}

//==========================================================================//
// StartDMARead
//   Purpose: Start a Bus Master DMA Read request.  This is a read from the 
//			  perspective of the device, so data is flowing from system main 
//            memory to the device.
//
//   Parameters:
//      IN DeviceObject:  pointer to the device information structure
//      IN Irp:  pointer to the I/O Request Packet
//
//   Returns:
//      If function succeeds, the I/O Request Packet will be queued while 
//      the device handles the data transfer.  The return code for this
//      will be STATUS_PENDING.  A failure code is returned for any errors.
//==========================================================================//
NTSTATUS
StartDMARead(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )
{    
	PDEVICE_EXTENSION   pDevExt;			// device characteristics
	NTSTATUS			status;				// return code
	ULONG				mapRegsNeeded;		// Number of map registers needed for the transfer
	KIRQL				oldIrql;
	ULONG	*			baseVa;
 
	// Retrieve our device characteristics.
	pDevExt = pDeviceObject->DeviceExtension;

	LogMsg(pDeviceObject, L"StartDMARead: Processing Read Irp.");
	KdPrint(("s3_1000.sys: StartDMARead\n"));
    
    // Start the IO timer. The timer will time out after 3 seconds if no
    // interrupt has been received. This must be an interlocked operation.
    KeSynchronizeExecution(pDevExt->InterruptObject, 
                          (PKSYNCHRONIZE_ROUTINE) SetIoTimerValue, pDevExt);
	
	// Make sure that the buffers in main memory are coherent with the cache before 
	// beginning a DMA transfer.  Note that the READ/WRITE IRPs actually refer to 
	// the perspective of the Bus Master device, so here we reverse the 2nd cache flush
	// parameter to be from the perspective of the OS in order to match up correctly.
	KeFlushIoBuffers(pIrp->MdlAddress, 
					 FALSE,								// Write DMA (from the OS perspective)
					 TRUE);								// DMA Transfer indicated	
	
	// Determine the number of map registers required.  This driver currently does not support
	// a DMA transfer that exceeds the available map registers, so if that happens we treat it 
	// as an error and punt the transfer.  
	// In order to handle this with a non S/G DMA device, the driver must split up
	// a single large transaction into N smaller transactions (that fit in the map register space), and
	// then program & start the DMA device once for each of the N transfers.  The driver keeps track of
	// how much data is transfered each time.
	mapRegsNeeded = ADDRESS_AND_SIZE_TO_SPAN_PAGES(MmGetMdlVirtualAddress(pIrp->MdlAddress), 
												   pDevExt->ReadTotalLength - pDevExt->ReadOffset);
	
	if (mapRegsNeeded <= pDevExt->ReadMapRegMax)
	{
		pDevExt->ReadMapRegNeeded = mapRegsNeeded;
		pDevExt->ReadLength = pDevExt->ReadTotalLength - pDevExt->ReadOffset;
	}

	else
	{
		pDevExt->ReadMapRegNeeded = pDevExt->ReadMapRegMax;
		pDevExt->ReadLength = pDevExt->ReadMapRegMax * PAGE_SIZE;
	}

	// Set the Irp as current, where it will remain until the device has completed
	// the data transfer.  The device will be started from an I/O Control call, since it may still
	// need to be programmed for a simultaneous write transfer first.  
	pDevExt->ReadIrp = pIrp; 

	// Request a channel for the Read Adapter.  This must be done at DISPATCH level.
	KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
	baseVa = MmGetMdlVirtualAddress(pIrp->MdlAddress + pDevExt->ReadOffset);

	if(
		status = pDevExt->pReadAdapter->DmaOperations->GetScatterGatherList(pDevExt->pReadAdapter, 
																   pDeviceObject,
																   pIrp->MdlAddress,
																   baseVa,
																   pDevExt->ReadLength,
																   ReadAdapterControl,
																   pIrp,
																   TRUE)// transfer to device from buffer
		!= STATUS_SUCCESS)
	{
			LogMsg(pDeviceObject, L"StartDMARead: (Read) GetScatterGatherList failed");
			KdPrint(("s3_1000.sys: StartDMARead - GetScatterGatherList failed\n"));
			
			pIrp->IoStatus.Status = status;
            pIrp->IoStatus.Information = 0;
            
            // Complete the request now
            IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			return status;
	}
	else
		status = STATUS_PENDING;

	KeLowerIrql(oldIrql);
	KdPrint(("s3_1000.sys: StartDMARead completed\n"));
	
    return status; 
}// StartDMARead

//==========================================================================//
// StartDMAWrite
//   Purpose: Start a Bus Master DMA Write request.  This is a write from the 
//			  perspective of the device, so data is flowing from the device to 
//            system main memory.
//
//   Parameters:
//      IN DeviceObject:  pointer to the device information structure
//      IN Irp:  pointer to the I/O Request Packet
//
//   Returns:
//      If function succeeds, the I/O Request Packet will be queued while 
//      the device handles the data transfer.  The return code for this
//      will be STATUS_PENDING.  A failure code is returned for any errors.
//==========================================================================//
NTSTATUS
StartDMAWrite(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp 
    )
{    
	PDEVICE_EXTENSION   pDevExt;			// device characteristics
	NTSTATUS			status;				// return code
	ULONG				mapRegsNeeded;		// Number of map registers needed for the transfer
	KIRQL				oldIrql;
	ULONG	*			baseVa;


	// Retrieve our device characteristics.
	pDevExt = pDeviceObject->DeviceExtension;

	LogMsg(pDeviceObject, L"StartDMAWrite: Processing Write Irp.");
	KdPrint(("s3_1000.sys: StartDMAWrite\n"));
    
    // Start the IO timer. The timer will time out after 3 seconds if no
    // interrupt has been received. This must be an interlocked operation.
    KeSynchronizeExecution(pDevExt->InterruptObject, 
                          (PKSYNCHRONIZE_ROUTINE) SetIoTimerValue, pDevExt);

	// Make sure that the buffers in main memory are coherent with the cache before 
	// beginning a DMA transfer.  Note that the READ/WRITE IRPs actually refer to 
	// the perspective of the Bus Master device, so here we reverse the 2nd cache flush
	// parameter to be from the perspective of the OS in order to match up correctly.
	KeFlushIoBuffers(Irp->MdlAddress, 
					 TRUE,								// Read DMA (from the OS perspective)
					 TRUE);								// DMA Transfer indicated	
	
	// Determine the number of map registers required.  This driver currently does not support
	// a DMA transfer that exceeds the available map registers, so if that happens we treat it 
	// as an error and punt the transfer.  
	// In order to handle this with a non S/G DMA device, the driver must split up
	// a single large transaction into N smaller transactions (that fit in the map register space), and
	// then program & start the DMA device once for each of the N transfers.  The driver keeps track of
	// how much data is transfered each time.
	mapRegsNeeded = ADDRESS_AND_SIZE_TO_SPAN_PAGES(MmGetMdlVirtualAddress(pIrp->MdlAddress), 
												   pDevExt->WriteTotalLength - pDevExt->WriteOffset);
		if (mapRegsNeeded <= pDevExt->WriteMapRegMax)
	{
		pDevExt->WriteMapRegNeeded = mapRegsNeeded;
		pDevExt->WriteLength = pDevExt->WriteTotalLength - pDevExt->WriteOffset;
	}

	else
	{
		pDevExt->WriteMapRegNeeded = pDevExt->WriteMapRegMax;
		pDevExt->WriteLength = pDevExt->WriteMapRegMax * PAGE_SIZE;
	}
 
	// Set the Irp as current, where it will remain until the device has completed
	// the data transfer.  The device will be started from an I/O Control call, since it may still
	// need to be programmed for a simultaneous read transfer first.  
	pDevExt->WriteIrp = pIrp;

	// Request a channel for the Write Adapter.  This must be done at DISPATCH level.
	KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
	baseVa = MmGetMdlVirtualAddress(pIrp->MdlAddress + pDevExt->WriteOffset);
	
	if(
		status = pDevExt->pWriteAdapter->DmaOperations->GetScatterGatherList(pDevExt->pWriteAdapter, 
																   pDeviceObject,
																   pIrp->MdlAddress,
																   baseVa,
																   pDevExt->WriteLength,
																   WriteAdapterControl,
																   pIrp,
																   FALSE)// transfer from device to buffer
																   
		!= STATUS_SUCCESS)
	{
			LogMsg(pDeviceObject, L"StartDMAWrite: (Write) GetScatterGatherList failed");
			KdPrint(("s3_1000.sys: StartDMAWrite - GetScatterGatherList failed\n"));
		   
			pIrp->IoStatus.Status = status;
            pIrp->IoStatus.Information = 0;
            
            // Complete the request now
            IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			return status;
			
	}
	else
		status = STATUS_PENDING;

	KeLowerIrql(oldIrql);
    KdPrint(("s3_1000.sys: StartDMAWrite completed\n"));
    														  
    return status; 
}// StartDMAWrite

//==========================================================================//
// ReadAdapterControl
//   Purpose: Program the Bus Master DMA Read operation in the device.
//
//   Parameters:
//      IN pDevObj:  pointer to the device information structure
//      IN Unsed_with_Driver_Queuing:  normally the IRP, but unused with DQ
//      IN pSGL:     pointer to the Scatter/Gather list allocated by the OS
//      IN pContext: normally the IRP for DQ.  We don't use it, but its available.
//
//   Returns:
//      None.
//
//	 Comment:
//		Normally this is where the DMA operation would be started.  The 
//      existing design allows for the application to program a second
//      operation, so starting the transfer is deferred to application
//      control via a PIO command.
//==========================================================================//
VOID
ReadAdapterControl(
  IN PDEVICE_OBJECT pDevObj,
  IN PIRP Unsed_with_Driver_Queuing,
  IN PSCATTER_GATHER_LIST pSGL,
  IN PVOID pContext)
{
	PDEVICE_EXTENSION	pDevExt = (PDEVICE_EXTENSION) pDevObj->DeviceExtension;
	PIRP				pIrp = (PIRP) pContext;
	ULONG				regAddr;				// Memory-mapped Device register for the DMA transfer address

	LogMsg(pDevObj, L"ReadAdapterControl: BMDMA Read using System S/G");
	KdPrint(("s3_1000.sys: ReadAdapterControl - BMDMA Read using System S/G\n"));

	// Save the Scatter/Gather list pointer for release later.
	pDevExt->pReadSGL = pSGL;

	// Program the logical transfer address into our Bus Master device.
	regAddr = (ULONG) pDevExt->MemoryStart[FPGA] + READ_ADDR_OFFSET;
	WRITE_REGISTER_ULONG ((PULONG) regAddr, pSGL->Elements[0].Address.LowPart);

	// LIMITATION:  The driver relies on the application to provide the programming information for the device
	// in one step, and then kick off the DMA engine via PIO when all operations have been programmed.  This
	// means that the driver cannot know at this point if it should wait for a simultaneous write transfer to
	// complete before starting the engine again to continue with the read operations.  This effectively
	// restricts simultaneous read/write operations to a size that can be performed with a single set of map
	// registers.  Without a redesign of the driver/application interaction, transfers requiring multiple
	// DMA passes can only be Read or Write, but not both.  This restriction must be enforced by the 
	// application, since the driver cannot check for this scenario.
	if (pDevExt->ReadOffset > 0)
	{
		// Maintain statistics for current transfer - storage for these needs to be added to the device
		// extension.
		// Perform Initiator Reset
		// Release Initiator Reset
		// Start DMA Engine
	}

	// We're done with the adapter channel object, but we must keep the map registers around for the 
	// device to use for the transfer.  The return code signals this intent to the I/O Manager
	KdPrint(("s3_1000.sys: ReadAdapterControl completed\n"));
	return;

}

//==========================================================================//
// WriteAdapterControl
//   Purpose: Program the Bus Master DMA Write operation in the device.
//
//   Parameters:
//      IN pDevObj:  pointer to the device information structure
//      IN Unsed_with_Driver_Queuing:  normally the IRP, but unused with DQ
//      IN pSGL:     pointer to the Scatter/Gather list allocated by the OS
//      IN pContext: normally the IRP for DQ.  We don't use it, but its available.
//
//   Returns:
//      None.
//
//	 Comment:
//		Normally this is where the DMA operation would be started.  The 
//      existing design allows for the application to program a second
//      operation, so starting the transfer is deferred to application
//      control via a PIO command.
//==========================================================================//
VOID
WriteAdapterControl(
  IN PDEVICE_OBJECT pDevObj,
  IN PIRP Unsed_with_Driver_Queuing,
  IN PSCATTER_GATHER_LIST pSGL,
  IN PVOID pContext)
{
	PDEVICE_EXTENSION	pDevExt = (PDEVICE_EXTENSION) pDevObj->DeviceExtension;
	PIRP				pIrp = (PIRP) pContext;
	ULONG				regAddr;				// Memory-mapped Device register for the DMA transfer address

	LogMsg(pDevObj, L"WriteAdapterControl: BMDMA Write using System S/G");
	KdPrint(("s3_1000.sys: WriteAdapterControl\n"));

	// Save the Scatter/Gather list pointer for release later.
	pDevExt->pWriteSGL = pSGL;

	// Program the logical transfer address into our Bus Master device.
	regAddr = (ULONG) pDevExt->MemoryStart[FPGA] + WRITE_ADDR_OFFSET;
	WRITE_REGISTER_ULONG ((PULONG) regAddr, pSGL->Elements[0].Address.LowPart);

	// LIMITATION:  The driver relies on the application to provide the programming information for the device
	// in one step, and then kick off the DMA engine via PIO when all operations have been programmed.  This
	// means that the driver cannot know at this point if it should wait for a simultaneous write transfer to
	// complete before starting the engine again to continue with the read operations.  This effectively
	// restricts simultaneous read/write operations to a size that can be performed with a single set of map
	// registers.  Without a redesign of the driver/application interaction, transfers requiring multiple
	// DMA passes can only be Read or Write, but not both.  This restriction must be enforced by the 
	// application, since the driver cannot check for this scenario.
	if (pDevExt->WriteOffset > 0)
	{
		// Maintain statistics for current transfer - storage for these needs to be added to the device
		// extension.
		// Perform Initiator Reset
		// Release Initiator Reset
		// Start DMA Engine
	}

	// We're done with the adapter channel object, but we must keep the map registers around for the 
	// device to use for the transfer.  The return code signals this intent to the I/O Manager.
	KdPrint(("s3_1000.sys: WriteAdapterControl completed\n"));
	return;
//	return DeallocateObjectKeepRegisters;
}


//==========================================================================//
// Unload 
//   Purpose: Called by the I/O Manager when a driver unload request is
//			  received.
//
//   Parameters:
//      IN DriverObject:  pointer to the driver object.
//
//   Returns:
//      None.
//
//	 Comment:
//		There is no special clean-up required for our device, so this 
//      function is essentially a no-op.  It is required by the WDM.
//==========================================================================//
// IRQL PASSIVE_LEVEL
VOID
Unload (
  IN PDRIVER_OBJECT     DriverObject
  )
{
	PAGED_CODE ();

	KdPrint (("s3_1000.sys: DriverUnload\n"));

	return;
} // Unload

//==========================================================================//
// OnTimer 
//   Purpose: Designated interrupt handler for the device timer.
//
//   Parameters:
//      IN PDEVICE_OBJECT:    Pointer to the device object 
//      IN PDEVICE_EXTENSION: Pointer to the device extension
//
//   Returns:
//      none
//
//	 Comment:
//		This function is called by a device timer timeout interrupt. It calls
//      a function that determins if a timeout should terminate the IRP with an
//      error. This must be synchronized with the BDMA_Isr function
//==========================================================================//
VOID OnTimer(  
      IN PDEVICE_OBJECT     DeviceObject, 
      IN PDEVICE_EXTENSION  DeviceExtension )
{						
   KeSynchronizeExecution(DeviceExtension->InterruptObject, 
                         (PKSYNCHRONIZE_ROUTINE) CheckTimer, DeviceExtension);
}					

#pragma LOCKEDCODE

//==========================================================================//
// CheckTimer 
//   Purpose: Designated interrupt handler for the device timer.
//
//   Parameters:
//      IN PDEVICE_EXTENSION: Pointer to the device extension
//
//   Returns:
//      none
//
//	 Comment:
//		The CheckTimer function will terminate an IRP if the timeout has 
//      been reached
//==========================================================================//
VOID CheckTimer(
           IN PDEVICE_EXTENSION DeviceExtension )
{							
    
    
	if (DeviceExtension->timer <= 0 || --DeviceExtension->timer > 0)
		return;					// not counting or hasn't timed out yet
		
	if (!DeviceExtension->busy)
		return;					// no request active
    KdPrint(("s3_1000.sys: CheckTimer\n"));
	DeviceExtension->busy = FALSE;
    DeviceExtension->iotimeout = TRUE;
	IoRequestDpc(DeviceExtension->FunctionDeviceObject, NULL, DeviceExtension);
}   // CheckTimer				

//==========================================================================//
// SetIoTimerValue 
//   Purpose: Initializes the timer value to the start value
//
//   Parameters:
//      IN PDEVICE_EXTENSION: Pointer to the device extension
//
//   Returns:
//      none
//
//	 Comment:
//		The timer value is set to an initial value of 1 second. Busy
//      is set to TRUE because we are starting an operation.
//==========================================================================//
VOID SetIoTimerValue (
  IN PDEVICE_EXTENSION  DeviceExtension )
{
    DeviceExtension->timer = 1;			 
	DeviceExtension->busy = TRUE;
} // SetIoTimerValue

//==========================================================================//
// BDMA_Isr 
//   Purpose: Designated interrupt handler for the DMA hardware.
//
//   Parameters:
//      IN  pIntObj:		 pointer to the interrupt object.
//		IN  pServiceContext: pointer to the device object.
//
//   Returns:
//      TRUE if this is an interrupt for our device, FALSE if it is not.
//      (PCIe devices must share interrupt vectors)
//
//	 Comment:
//		Normally this function would clear an interrupt register in the device,
//      but our interrupts are transmitted as PCIe frames using MSI.  Clearing
//      of the interrupt is handled by a subsequent MSI packet, outside of 
//      driver control.
//      As per normal, all of the processing required to service the interrupt
//      is passed to a DPC, running at a lower priority.
//==========================================================================//
BOOLEAN BDMA_Isr (
			IN PKINTERRUPT pIntObj,
			IN PVOID pServiceContext		) 
{
	ULONG							address;
	ULONG							regValue;
	PDEVICE_EXTENSION				pDevExt;
	BOOLEAN							myInt = FALSE;		// Assume its not our interrupt
	PDEVICE_OBJECT					pDevObj = (PDEVICE_OBJECT) pServiceContext;

	pDevExt = (PDEVICE_EXTENSION) pDevObj->DeviceExtension;
    
    if (!pDevExt->busy)
		return TRUE;			// spurious interrupt

	// An interrupt has occurred, but since interrupt vectors may be shared by
	// multiple PCIe devices, we must determine if our device is the initiator.
	// Since our device only interrupts on either a read or write completion,
	// we can simply look at the status register to see if a done bit is set.  If
	// neither (read/write) bit is set, we punt the interrupt since it isn't meant for us.

    KdPrint(("s3_1000.sys: BDMA_Isr\n"));

	address = (ULONG) pDevExt->MemoryStart[FPGA] + DCSR_OFFSET;
	regValue = READ_REGISTER_ULONG ((PULONG) address); // Read the register
    

	// The following should be combined
	if (~pDevExt->ReadDone && (regValue & 0x1000000))	// Read Complete
	{
        pDevExt->busy = FALSE; // prevents race with CheckTimer
        pDevExt->timer = -1;
		IoRequestDpc(pDevObj, pDevExt->ReadIrp, (PVOID)pDevExt);
		pDevExt->ReadDone = TRUE;						// One read operation only per run
		myInt = TRUE;
	}

	if (~pDevExt->WriteDone && (regValue & 0x100))		// Write Complete
	{
        pDevExt->busy = FALSE; // prevents race with CheckTimer
        pDevExt->timer = -1;
		IoRequestDpc(pDevObj, pDevExt->WriteIrp, (PVOID)pDevExt);
		pDevExt->WriteDone = TRUE;						// One write operation only per run
		myInt = TRUE;
	}

	KdPrint(("s3_1000.sys: BDMA_Isr completed\n"));

	return (myInt);
}

//==========================================================================//
// DpcForIsr 
//   Purpose: Performs any processing associated with interrupt service.
//
//   Parameters:
//      IN  pDpc:		pointer to the DPC structure assigned by the OS
//		IN  pDevObj:	pointer to the device object.
//		IN  pIrp:		pointer to the interrupting IRP.
//		IN  pContext:	pointer to the device extension.
//
//   Returns:
//      None.
//
//	 Comment:
//		Closes out the current IRP if the transfer is complete.  Although
//      not currently supported by the device, the driver queue supports
//      additional requests for transfers received while operations are
//      currently active.
//      Also includes code for transfers larger than the map register limit,
//      although this is not supported by the application/driver combination.
//==========================================================================//
VOID
DpcForIsr(
	IN PKDPC pDpc,
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp,
	IN PVOID pContext
	)
{
	PDEVICE_EXTENSION	pDevExt = (PDEVICE_EXTENSION) pContext;
	KIRQL				oldIrql;
	PLIST_ENTRY			pNextIrp;
	NTSTATUS			newStatus;
    PIRP                CurrentReadIrp = NULL;
	PIRP                CurrentWriteIrp = NULL;
	
	KdPrint(("s3_1000.sys: DpcForIsr\n"));
    
	if (pDevExt->WriteLength > 0 )
	{

		// Copy the data in the map registers back to the original (non-contiguous) user
		// buffer space. 
		pDevExt->pWriteAdapter->DmaOperations->PutScatterGatherList(
							pDevExt->pWriteAdapter,
							pDevExt->pWriteSGL,
						    FALSE);	// Write-to-device is False for BMDMA Write
							 

		// Update the offset with the data write.
		pDevExt->WriteOffset += pDevExt->WriteLength;

		// If the transfer is done, clean up and complete the original IRP.
		if (pDevExt->WriteOffset >= pDevExt->WriteTotalLength)
		{
			// Set the IRP return values.
            // If we entered due to an IO timeout, the return parameters
            // have already been set.
            if(pDevExt->iotimeout == FALSE)
            {
			   pDevExt->WriteIrp->IoStatus.Status = STATUS_SUCCESS;
			   pDevExt->WriteIrp->IoStatus.Information = pDevExt->WriteLength;
               
            }
            else
            {
               LogMsg(pDevObj, L"DpcForIsr: PCIe Write - NO INTERUPT FROM HW");
               pDevExt->iotimeout = FALSE;
               pDevExt->WriteIrp->IoStatus.Status = STATUS_IO_TIMEOUT;
	           pDevExt->WriteIrp->IoStatus.Information = 0;
            }

	        // Nullify the current IRP pointer after remembering the current one.
	        // We'll return the current IRP pointer as our return value so that
	        // the DPC routine has a way to know whether an active request got
	        // aborted.
	        CurrentWriteIrp = (PIRP) InterlockedExchangePointer((PVOID*) &pDevExt->WriteIrp, NULL);

	        // If we just finished processing a request, set the event on which
	        // WaitForCurrentIrp may be waiting in some other thread.
	        if (CurrentWriteIrp)
		        KeSetEvent(&pDevExt->writeevStop, 0, FALSE);

			//Close out the IRP and propagate execution for further requests.
			IoCompleteRequest(CurrentWriteIrp, IO_NO_INCREMENT );
			LogMsg(pDevObj, L"DpcForIsr: PCIe Write BMDMA Transfer Complete");
			KdPrint(("s3_1000.sys: DpcForIsr - PCIe Write BMDMA Transfer Complete\n"));

			// Reset our device variables.
			pDevExt->WriteLength = 0;
			pDevExt->WriteMapRegNeeded = 0;
			pDevExt->WriteIrp = NULL;
			pDevExt->WriteMapRegisterBase = NULL;

			// Lets see if any write requests were queued up by the Dispatch routine while
			// we've been busy.  If not, we just continue on.
			if (!IsListEmpty(&pDevExt->WriteQueue))
			{
				KeAcquireSpinLock(&pDevExt->WriteQueueLock, &oldIrql);

	            // Start the next IRP.
	            while (!pDevExt->writestallcount && !pDevExt->writeabortstatus 
			 	      && !IsListEmpty(&pDevExt->WriteQueue))
		        {						
                   // start next packet
		           PLIST_ENTRY next = RemoveHeadList(&pDevExt->WriteQueue);
		           PIRP Irp = CONTAINING_RECORD(next, IRP, Tail.Overlay.ListEntry);

		           // Nullify the cancel pointer in this IRP. If it was
		           // already NULL, someone is trying to cancel this IRP right now. Reinitialize
		           // the link pointers so the cancel routine's call to RemoveEntryList won't
		           // do anything harmful and look for another IRP. The cancel routine will
		           // take over as soon as we release the spin lock
		           if (!IoSetCancelRoutine(Irp, NULL))
			       {					
                      // IRP being cancelled right now
			          ASSERT(Irp->Cancel);	// else CancelRoutine shouldn't be NULL!
			          InitializeListHead(&Irp->Tail.Overlay.ListEntry);
			          continue;			// with "start next Irp"
			       }				

		           pDevExt->WriteIrp = Irp;
		   
		        } // start the next IRP
				
				KeReleaseSpinLock(&pDevExt->WriteQueueLock, oldIrql);
                
                KdPrint(("s3_1000.sys: DpcForIsr - new write IRP started\n"));
	            newStatus = StartDMAWrite(pDevObj, pDevExt->WriteIrp);
			    if (newStatus == STATUS_PENDING)
			    {
		           pDevExt->WriteIrp->IoStatus.Status = newStatus;
			       pDevExt->WriteIrp->IoStatus.Information = 0;
			       IoMarkIrpPending(pDevExt->WriteIrp);
			    }
			}// any more write requests ?
		}// transfer done

		// There is more data to be transferred, so update the data pointer and initiate another DMA operation.
		else
		{
			KdPrint(("s3_1000.sys: DpcForIsr - more to do on this write transfer\n"));
			(void) StartDMAWrite(pDevObj, pDevExt->WriteIrp);
		}
	}

	if (pDevExt->ReadLength > 0)
	{
		// Copy the data in the map registers back to the original (non-contiguous) user
		// buffer space.
		pDevExt->pReadAdapter->DmaOperations->PutScatterGatherList(
							pDevExt->pReadAdapter,
							pDevExt->pReadSGL,
							TRUE);	 // Write-to-device is True for BMDMA Read

		// Update the offset with the data read.
		pDevExt->ReadOffset += pDevExt->ReadLength;

		// If the transfer is done, clean up and complete the original IRP.
		if (pDevExt->ReadOffset >= pDevExt->ReadTotalLength)
		{
			// Set the IRP return values.
            // If we entered due to an IO timeout, the return parameters
            // have already been set.
            if(pDevExt->iotimeout == FALSE)
            {
			   pDevExt->ReadIrp->IoStatus.Status = STATUS_SUCCESS;
			   pDevExt->ReadIrp->IoStatus.Information = pDevExt->ReadLength;
            }
            else
            {
               LogMsg(pDevObj, L"DpcForIsr: PCIe Read - NO INTERUPT FROM HW");
               pDevExt->iotimeout = FALSE;
               pDevExt->ReadIrp->IoStatus.Status = STATUS_IO_TIMEOUT;
	           pDevExt->ReadIrp->IoStatus.Information = 0;
            }


	        // Nullify the current IRP pointer after remembering the current one.
	        // We'll return the current IRP pointer as our return value so that
	        // the DPC routine has a way to know whether an active request got
	        // aborted.
	        CurrentReadIrp = (PIRP) InterlockedExchangePointer((PVOID*) &pDevExt->ReadIrp, NULL);

	        // If we just finished processing a request, set the event on which
	        // WaitForCurrentIrp may be waiting in some other thread.
	        if (CurrentReadIrp)
		    KeSetEvent(&pDevExt->readevStop, 0, FALSE);
            
			//Close out the just finished IRP and propagate execution for further requests.
			IoCompleteRequest(CurrentReadIrp, IO_NO_INCREMENT );
            
			LogMsg(pDevObj, L"DpcForIsr: PCIe Read BMDMA Transfer Complete");
			KdPrint(("s3_1000.sys: DpcForIsr - PCIe Read BMDMA Transfer Complete\n"));

			// Reset our device variables.
			pDevExt->ReadLength = 0;
			pDevExt->ReadMapRegNeeded = 0;
			pDevExt->ReadIrp = NULL;
			pDevExt->ReadMapRegisterBase = NULL;

			// Lets see if any read requests were queued up by the Dispatch routine while
			// we've been busy.  If not, we just continue on.
			if (!IsListEmpty(&pDevExt->ReadQueue))
			{
				KeAcquireSpinLock(&pDevExt->ReadQueueLock, &oldIrql);

	            // Start the next IRP.
	            while (!pDevExt->readstallcount && !pDevExt->readabortstatus 
			 	       && !IsListEmpty(&pDevExt->ReadQueue))
		        {						
                    // start next packet
		            PLIST_ENTRY next = RemoveHeadList(&pDevExt->ReadQueue);
		            PIRP Irp = CONTAINING_RECORD(next, IRP, Tail.Overlay.ListEntry);

		            // Nullify the cancel pointer in this IRP. If it was
		            // already NULL, someone is trying to cancel this IRP right now. Reinitialize
		            // the link pointers so the cancel routine's call to RemoveEntryList won't
		            // do anything harmful and look for another IRP. The cancel routine will
		            // take over as soon as we release the spin lock
		            if (!IoSetCancelRoutine(Irp, NULL))
			        {					
                       // IRP being cancelled right now
			           ASSERT(Irp->Cancel);	// else CancelRoutine shouldn't be NULL!
			           InitializeListHead(&Irp->Tail.Overlay.ListEntry);
			           continue;			// with "start next Irp"
			        }				

		            pDevExt->ReadIrp = Irp;
		   
		       } // start the next IRP
			  
			   KeReleaseSpinLock(&pDevExt->ReadQueueLock, oldIrql);
               
               KdPrint(("s3_1000.sys: DpcForIsr - new read IRP started\n"));
			   newStatus = StartDMARead(pDevObj, pDevExt->ReadIrp);
 
			   // Mark the Irp for deferred processing, if there were no problems in StartDMARead.
			   if (newStatus == STATUS_PENDING)
			   {
				    pDevExt->ReadIrp->IoStatus.Status = newStatus;
				    pDevExt->ReadIrp->IoStatus.Information = 0;
				    IoMarkIrpPending(pDevExt->ReadIrp);
			   }
			}// read requests queued up ? 
		}// transfer done

		// There is more data to be transferred, so update the data pointer and initiate another DMA operation.
		else
		{
			 KdPrint(("s3_1000.sys: DpcForIsr - more to do on this read transfer\n"));
			(void) StartDMARead(pDevObj, pDevExt->ReadIrp);
		}
	}
	 KdPrint(("s3_1000.sys: DpcForIsr completed\n"));
}
