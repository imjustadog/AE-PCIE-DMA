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
#include "s3_1000.h"

CHAR* statenames[9] = {
	        "NOTSTARTED",          
            "DELETED",                 
            "SURPRISEREMOVED",   
            "STOPPED",                 
            "REMOVEPENDING",           
            "FAILHARDWARE",            
            "STOPPENDING",             
            "FAILSOFTWARE",            
            "STARTED"   
};               


NTSTATUS
PnPStartDevice (
  IN PDEVICE_OBJECT     DeviceObject,
  IN PIRP               Irp
  );
  
NTSTATUS
PnPQueryStopDevice (
  IN PDEVICE_OBJECT     DeviceObject,
  IN PIRP               Irp
  );
  
NTSTATUS
PnPCancelStopDevice (
  IN PDEVICE_OBJECT     DeviceObject,
  IN PIRP               Irp
  );
  
VOID 
AbortRequests(
   IN PDEVICE_EXTENSION deviceExtension, 
   IN NTSTATUS          status,
   IN BOOLEAN           read
   );

NTSTATUS
PnPStopDevice (
  IN PDEVICE_OBJECT     DeviceObject,
  IN PIRP               Irp
  );

NTSTATUS
PnPQueryRemoveDevice (
  IN PDEVICE_OBJECT     DeviceObject,
  IN PIRP               Irp
  );
  
NTSTATUS
PnPRemoveDevice (
  IN PDEVICE_OBJECT     DeviceObject,
  IN PIRP               Irp
  );

NTSTATUS
PnPCancelRemoveDevice(
  IN PDEVICE_OBJECT     DeviceObject, 
  IN PIRP               Irp
  );

NTSTATUS
PnPSurpriseRemoval(
  IN PDEVICE_OBJECT     DeviceObject,
  IN PIRP               Irp
  );

NTSTATUS
CompletionRoutine (
  IN PDEVICE_OBJECT     DeviceObject,
  IN PIRP               Irp,
  IN PVOID              Context
  );

NTSTATUS
SendIrpSync (
  IN PDEVICE_EXTENSION deviceExtension,
  IN PIRP               Irp
  );

NTSTATUS 
PnPPassDown( 
  IN PDEVICE_OBJECT		DeviceObject,
  IN PIRP				Irp 
  );

NTSTATUS
GetDMAInfo (
  IN PDEVICE_OBJECT     pDeviceObject,
  IN PIRP               Irp
  );
  
VOID 
WaitForCurrentIrp(
   IN PDEVICE_EXTENSION DeviceExtension,
   IN BOOLEAN read
   );
   
VOID 
StallRequests(
   IN PDEVICE_EXTENSION DeviceExtension,
   IN BOOLEAN read
   );
   
VOID
RestartRequests(
   IN PDEVICE_EXTENSION pdq, 
   IN PDEVICE_OBJECT pDeviceObject,
   IN BOOLEAN read
   );
   
VOID 
AllowRequests(
   IN PDEVICE_EXTENSION deviceExtension
   );
  
BOOLEAN 
OkToStop(
    IN PDEVICE_EXTENSION deviceExtension
	);
	
BOOLEAN 
OkToRemove(
    IN PDEVICE_EXTENSION deviceExtension
	);

#pragma alloc_text (PAGE, AddDevice)
#pragma alloc_text (PAGE, DispatchPnP)
#pragma alloc_text (PAGE, PnPStartDevice)
#pragma alloc_text (PAGE, PnPQueryStopDevice) 
#pragma alloc_text (PAGE, PnPCancelStopDevice)
#pragma alloc_text (PAGE, PnPStopDevice)
#pragma alloc_text (PAGE, PnPQueryRemoveDevice) 
#pragma alloc_text (PAGE, PnPCancelRemoveDevice) 
#pragma alloc_text (PAGE, PnPSurpriseRemoval) 
#pragma alloc_text (PAGE, PnPRemoveDevice)
#pragma alloc_text (PAGE, DispatchPower)

// IRQL PASSIVE_LEVEL - called by the PnP Manager.
// AddDevice initializes the FDO and its device extension, but does not touch the HW.
// HW initialization is performed when the driver receives the IRP_MJ_PNP function code.
NTSTATUS
AddDevice (
  IN PDRIVER_OBJECT     DriverObject,
  IN PDEVICE_OBJECT     PhysicalDeviceObject
  )
{
	NTSTATUS              status = STATUS_SUCCESS;
	PDEVICE_OBJECT        deviceObject = NULL;
	PDEVICE_EXTENSION     deviceExtension;
	DEVICE_DESCRIPTION	  deviceDescription;
	PULONG				  mapRegisterCount = NULL;

	PAGED_CODE ();

	KdPrint (("s3_1000.sys: AddDevice\n"));

  // Create our device object, which represents the Functional Device Object controlled by
  // this device driver.
	status = IoCreateDevice (
      DriverObject,
      sizeof (DEVICE_EXTENSION),
      NULL,
      FILE_DEVICE_UNKNOWN,
      0,
      FALSE,
      &deviceObject);

    // Exit AddDevice if we are unable to create the FDO.
	if (!NT_SUCCESS (status))  
	{
		KdPrint (("s3_1000.sys: IoCreateDevice exits with 0x%x\n", status));
		return status;
	}

    // Initialize our device extension, where we keep all the required persistent
    // information about this driver.
    deviceExtension = (PDEVICE_EXTENSION) deviceObject->DeviceExtension;
    deviceExtension->PhysicalDeviceObject = PhysicalDeviceObject;
    deviceExtension->FunctionDeviceObject = deviceObject;
	deviceExtension->InterruptObject = NULL;
    deviceExtension->iotimeout = FALSE;
    deviceExtension->timer = -1;

	// Initialize the byte offset to memory within each BAR.  BAR 0 is implemented as a 
	// set of memory-mapped registers for our DMA device.   Hooks for BAR 1 are provided
	// for future extension, but are unused in this driver.
	deviceExtension->baseOffset[0] = 0x0;
	deviceExtension->baseOffset[1] = 0x0;

    // Place our driver on top of the current Device Stack for this hardware.  In this
    // simple case, we have only the PDO created by the bus driver, and our FDO that
    // we are attaching.
    deviceExtension->NextStackDeviceObject = 
		IoAttachDeviceToDeviceStack (deviceObject,PhysicalDeviceObject);

    // Register the interface in the system registry and save the name assigned 
    // by the system in our device extension.
    status = IoRegisterDeviceInterface (
	  PhysicalDeviceObject,
	  (LPGUID) &GuidDriver,
	  NULL,
	  &deviceExtension->InterfaceName);

    // Exit if we are unsuccessful in registering our device.  In this unlikely event,
    // we will try to delete the device we've just added.  
    if (!NT_SUCCESS (status))  
	{
	  KdPrint (("s3_1000.sys: IoRegisterDeviceInterface exits with 0x%x\n", status));
	  IoDeleteDevice (deviceObject);
	  return status;
	}

    // Initialize our Driver state.
	INITIALIZE_PNP_STATE (deviceExtension);

    // Set up our driver to use Direct I/O for and to disable
    // device initialization, since none is required.
//    deviceObject->Flags |= DO_BUFFERED_IO;					// Buffered I/O
    deviceObject->Flags |= DO_DIRECT_IO;					// Direct I/O  (for DMA)
    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
	
	// Initialize the spin locks for the Read and Write request queues.
	KeInitializeSpinLock(&deviceExtension->ReadQueueLock);
	KeInitializeSpinLock(&deviceExtension->WriteQueueLock);

	// Initialize the Irq Lists.
	InitializeListHead(&deviceExtension->ReadQueue);
	InitializeListHead(&deviceExtension->WriteQueue);
	
	// Initialize the in-progress list (only 1 in-progress request of each type is allowed).
	deviceExtension->ReadIrp = NULL;
	deviceExtension->WriteIrp = NULL;
	
	// Initialize the queue status parameters
	deviceExtension->readstallcount = 1;
	KeInitializeEvent(&deviceExtension->readevStop, NotificationEvent, FALSE);
	deviceExtension->readabortstatus = (NTSTATUS) 0;
	deviceExtension->writestallcount = 1;
	KeInitializeEvent(&deviceExtension->writeevStop, NotificationEvent, FALSE);
	deviceExtension->writeabortstatus = (NTSTATUS) 0;
	IoInitializeRemoveLock(&deviceExtension->RemoveLock, 0, 0, 0);

	// Initialize the Interrupt tracking variables.
	deviceExtension->ReadDone = FALSE;
	deviceExtension->WriteDone = FALSE;
    
    // Initialize the device object so we can set a timeout for write operations
	IoInitializeTimer(deviceObject, (PIO_TIMER_ROUTINE) OnTimer, 
                     (PVOID) deviceExtension);

	// Register a DpcForIsr to perform processing associated with an interrupt.
	IoInitializeDpcRequest(deviceObject, DpcForIsr);

	KdPrint (("s3_1000.sys: AddDevice complete\n"));
	return status;
} // AddDevice

// IRQL PASSIVE_LEVEL
// DispatchClose completes the current IRP and returns the status of the I/O request.
NTSTATUS
DispatchClose (
  IN PIRP               pIrp,
  IN NTSTATUS			status,
  IN ULONG				information
  )
{
	PIO_STACK_LOCATION	ioStack;
	ioStack = IoGetCurrentIrpStackLocation (pIrp);
	
	// Set the IRP status and information fields.  If this is not done religiously, the
	// Driver Verifier Tool returns fatal error code 226 - driver did not complete IRP
	// or failed to pass down an unexpected I/O request code.
	pIrp->IoStatus.Status = status;
    
    // The Driver Verifier will complain if this is not done
	if(ioStack->MinorFunction != IRP_MN_QUERY_DEVICE_RELATIONS)
	       pIrp->IoStatus.Information = information;

	// Mark the IRP as completed.  Do NOT try to access the IRP after this call, as it
	// has already been freed and WILL cause a nasty BSOD (blue screen of death).
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return status;
}

// IRQL PASSIVE_LEVEL
// DispatchPnP performs driver initialization and Plug and Play operations as requested by
// the PnP Manager through the use of the IRP_MJ_PNP function code.  All PNP messages are
// routed through this single function code, so this handler performs a secondary dispatch
// using the minor subcodes in the IRP.
NTSTATUS
DispatchPnP (
  IN PDEVICE_OBJECT     DeviceObject,
  IN PIRP               Irp
  )
{
	NTSTATUS							status = STATUS_SUCCESS;
	PDEVICE_EXTENSION					deviceExtension;
	PIO_STACK_LOCATION					ioStack;

	PAGED_CODE ();

	KdPrint (("s3_1000.sys:  DispatchPnP\n"));

	deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
	
    // Check if the device is being removed prior to processing this IRP
	// If it is already being removed, complete this IRP
    // If this test fails, the driver has received an IRP_MN_REMOVE_DEVICE and
    // is waiting at an IoReleaseRemoveLockAndWait (multiple processors)
	if (status != IoAcquireRemoveLock(&deviceExtension->RemoveLock, Irp))
	{ // Someone has set the remove device lock
		KdPrint (("s3_1000.sys: DispatchPnP exits - RemoveLock set\n"));
		return DispatchClose(Irp, status, 0);
	}

	ioStack = IoGetCurrentIrpStackLocation (Irp);

    // Send all I/O request down to the Physical Driver so any HW activities can
    // be completed before we perform our Functional Driver tasks.  This function
    // blocks until the lower level driver is finished.  If any problems occur,  abort
    // the IRP processing and return the failure status.
    // Failure to do this will result in an error code of 22E from the Driver Verifier
    // Tool - IRP completed without passing down.
    status = SendIrpSync (deviceExtension, Irp);

	if (!NT_SUCCESS (status))  
	{
		KdPrint (("s3_1000.sys: SendIrp exits with 0x%x\n", status));
        IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
		return DispatchClose(Irp, status, 0);
	}

	switch (ioStack->MinorFunction)  
	{
	case IRP_MN_START_DEVICE:
		LogMsg(DeviceObject, L"DispatchPnP: Device received StartDevice!");
		KdPrint(("s3_1000.sys: IRP_MN_START_DEVICE\n"));
		status = PnPStartDevice (DeviceObject, Irp);
        IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
		status = DispatchClose(Irp, status, 0);
		break;

	case IRP_MN_QUERY_STOP_DEVICE:
		LogMsg(DeviceObject, L"DispatchPnP: Device received QueryStopDevice!");
		KdPrint(("s3_1000.sys: IRP_MN_QUERY_STOP_DEVICE\n"));
		status = PnPQueryStopDevice(DeviceObject, Irp);
		break;
		
	case IRP_MN_STOP_DEVICE:
		LogMsg(DeviceObject, L"DispatchPnP: Device received StopDevice!");
		KdPrint(("s3_1000.sys: IRP_MN_STOP_DEVICE\n"));
		status = PnPStopDevice (DeviceObject, Irp);
        IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
		status = DispatchClose(Irp, status, 0);
		break;

	case IRP_MN_CANCEL_STOP_DEVICE:
		LogMsg(DeviceObject, L"DispatchPnP: Device received CancelStopDevice!");
		KdPrint(("s3_1000.sys: IRP_MN_CANCEL_STOP_DEVICE\n"));
        status = PnPCancelStopDevice(DeviceObject, Irp);
		break;

	case IRP_MN_QUERY_REMOVE_DEVICE:
		LogMsg(DeviceObject, L"DispatchPnP: Device received QueryRemoveDevice!");
		KdPrint(("s3_1000.sys: IRP_MN_QUERY_REMOVE_DEVICE\n"));
        status = PnPQueryRemoveDevice(DeviceObject, Irp);
		break;

	case IRP_MN_REMOVE_DEVICE:
		LogMsg(DeviceObject, L"DispatchPnP: Device received RemoveDevice!");
		KdPrint(("s3_1000.sys: IRP_MN_REMOVE_DEVICE\n"));
		status = PnPRemoveDevice (DeviceObject, Irp);
		break;

	case IRP_MN_CANCEL_REMOVE_DEVICE:
		LogMsg(DeviceObject, L"DispatchPnP: Device received CancelRemoveDevice!");
		KdPrint(("s3_1000.sys: IRP_MN_CANCEL_REMOVE_DEVICE\n"));
        status = PnPCancelRemoveDevice(DeviceObject, Irp);
		break;

	case IRP_MN_SURPRISE_REMOVAL:
		LogMsg(DeviceObject, L"DispatchPnP: Device received SurpriseRemoval!");
		KdPrint(("s3_1000.sys: IRP_MN_SURPRISE_REMOVAL\n"));
        status = PnPSurpriseRemoval(DeviceObject, Irp);
		break;

	default: // If the code is not supported, pass it down the device stack.
	    KdPrint(("s3_1000.sys: IRP_MN_DEFAULT\n"));
		status = PnPPassDown(DeviceObject, Irp);
	}
	
	KdPrint (("s3_1000.sys: DispatchPnP complete\n"));
	return status;
} // DispatchPnP


// Pass a PnP request that is not handled by this functional driver to the next lower
// driver in the device stack.  This driver has no need to wait for the lower driver
// to perform its processing, as we don't do anything further with the request.
NTSTATUS 
PnPPassDown( 
  IN PDEVICE_OBJECT pDO,
  IN PIRP pIrp ) 
{
	PDEVICE_EXTENSION     pDE;			// Pointer to the Device Extension

	// Remove the current IRP stack location from participation in IRP processing.
	IoSkipCurrentIrpStackLocation( pIrp );

	// Get a pointer to our device extension from the device object so we can
	// get the address of the next lower driver in the stack.
	pDE = (PDEVICE_EXTENSION) pDO->DeviceExtension;

	// Send the IRP to the next lower driver and return the status.
	return IoCallDriver(pDE->NextStackDeviceObject, pIrp);
} // PnPPassDown


NTSTATUS
PnPStartDevice (
  IN PDEVICE_OBJECT     DeviceObject,
  IN PIRP               Irp
  )
{
	PDEVICE_EXTENSION				deviceExtension;
	ULONG							i;
	ULONG							idx;
	PIO_STACK_LOCATION				ioStack;
	PCM_PARTIAL_RESOURCE_DESCRIPTOR rawResource;
	PCM_PARTIAL_RESOURCE_LIST       rawResourceList;
	NTSTATUS						status = STATUS_SUCCESS;
	PCM_PARTIAL_RESOURCE_DESCRIPTOR translatedResource;
	PCM_PARTIAL_RESOURCE_LIST       translatedResourceList;

	PAGED_CODE ();

	// Record the action in the Event Log.
	LogMsg(DeviceObject, L"PnPStartDevice: s3_1000 PCIe driver device started");
	KdPrint (("s3_1000.sys: PnPStartDevice\n"));

	deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
	ioStack = IoGetCurrentIrpStackLocation (Irp);

	if (GetDMAInfo(DeviceObject, Irp) != STATUS_SUCCESS)
		return status;
	
	if (ioStack->Parameters.StartDevice.AllocatedResources &&
      ioStack->Parameters.StartDevice.AllocatedResourcesTranslated)  
	{
		rawResourceList = 
		  &ioStack->Parameters.StartDevice.AllocatedResources->List[0].PartialResourceList;
		rawResource = &rawResourceList->PartialDescriptors[0];

		translatedResourceList = 
		  &ioStack->Parameters.StartDevice.AllocatedResourcesTranslated->List[0].PartialResourceList;
		translatedResource = &translatedResourceList->PartialDescriptors[0];

		for (i = 0; 
			 i < rawResourceList->Count; 
			 i++, rawResource++, translatedResource++)  
		{
			switch (rawResource->Type)  
			{
			case CmResourceTypeNull:
				KdPrint (("s3_1000.sys: Resource RAW Null\n"));
				break;

			case CmResourceTypePort:
				KdPrint (("s3_1000.sys: Resource Raw Port: (0x%x) Length: (0x%x)\n",
				  rawResource->u.Port.Start.LowPart, rawResource->u.Port.Length));

				switch (translatedResource->Type)  
				{
				case CmResourceTypePort:
					KdPrint (("s3_1000.sys: Resource Translated Port: (0x%x) Length: (0x%x)\n",
					  translatedResource->u.Port.Start.LowPart, 
					  translatedResource->u.Port.Length));
					break;

				case CmResourceTypeMemory:
					KdPrint (("s3_1000.sys: Resource Translated Memory: (0x%x) Length: (0x%x)\n",
					  translatedResource->u.Memory.Start.LowPart, 
					  translatedResource->u.Memory.Length));
					break;

				default:
					KdPrint (("s3_1000.sys: Unhandled resource 0x%x\n", translatedResource->Type));
					break;
				}

				break;

			case CmResourceTypeInterrupt:
				KdPrint (("s3_1000.sys: Resource Interrupt provided\n"));
				deviceExtension->IRQL = (KIRQL) translatedResource->u.Interrupt.Level;
				deviceExtension->Vector = translatedResource->u.Interrupt.Vector;
				deviceExtension->Affinity = translatedResource->u.Interrupt.Affinity;
				deviceExtension->TriggerMode = 
					(translatedResource->Flags & CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE) ? LevelSensitive : Latched;
				break;

			case CmResourceTypeMemory:
				KdPrint (("s3_1000.sys: Resource Raw Memory: (0x%x) Length: (0x%x)\n",
				  rawResource->u.Memory.Start.LowPart, 
				  rawResource->u.Memory.Length));
				KdPrint (("s3_1000.sys: Resource Translated Memory: (0x%x) Length: (0x%x)\n",
				  translatedResource->u.Memory.Start.LowPart, 
				  translatedResource->u.Memory.Length));

				// map memory
				idx = deviceExtension->bar;
				deviceExtension->MemoryOffset[idx] = 0;
				deviceExtension->MemoryLength[idx] = translatedResource->u.Memory.Length;
				deviceExtension->MemoryStart[idx]  = MmMapIoSpace (
				  translatedResource->u.Memory.Start,
				  translatedResource->u.Memory.Length,
				  MmNonCached);
				deviceExtension->Memory[idx] = translatedResource->u.Memory.Start;
				KdPrint (("s3_1000.sys: Translated resource is memory at 0x%x\n", 
				  deviceExtension->MemoryStart));
				break;

			case CmResourceTypeDma:
				break;

			case CmResourceTypeDeviceSpecific:
				break;

			default:
				break;
			}
		}
	}

	// Make sure we got our interrupt information correctly.  If not, return an
	// error condition.
	if (deviceExtension->IRQL == 0)
		return STATUS_BIOS_FAILED_TO_CONNECT_INTERRUPT;

	status = IoConnectInterrupt(&deviceExtension->InterruptObject,
					   BDMA_Isr,
					   DeviceObject,
					   NULL,
					   deviceExtension->Vector,
					   deviceExtension->IRQL,
					   deviceExtension->IRQL,
					//   deviceExtension->TriggerMode,
                       LevelSensitive,
					   TRUE,
					   deviceExtension->Affinity,
					   FALSE);
	
	if (status != STATUS_SUCCESS)
	{
		LogMsg(DeviceObject, L"PnPStartDevice: IoConnectInterrupt failed.");
	    KdPrint (("s3_1000.sys: PnPStartDevice - IoConnectInterrupt failed\n"));
	}

	else
	{
		// enable interface
		IoSetDeviceInterfaceState (&deviceExtension->InterfaceName, TRUE);

		SET_NEW_PNP_STATE (deviceExtension, Started); 

        // Allow all IRP's to start.
        AllowRequests(deviceExtension); // in case we got a bogus STOP
		RestartRequests(deviceExtension, DeviceObject, TRUE );  // read
        RestartRequests(deviceExtension, DeviceObject, FALSE);  // write

		// Record the action in the Event Log.
	    LogMsg(DeviceObject, L"PnPStartDeviceDevice: s3_1000 PCIe driver Start Device completed");
	KdPrint (("s3_1000.sys: PnPStartDevice completed\n"));
	}
	
	return status;
} // PnPStartDevice

NTSTATUS
PnPQueryStopDevice (
  IN PDEVICE_OBJECT     DeviceObject,
  IN PIRP               Irp
  )
{
	NTSTATUS              status = STATUS_SUCCESS;
	PDEVICE_EXTENSION     deviceExtension;
	
	PAGED_CODE ();

	// Record the action in the Event Log.
	LogMsg(DeviceObject, L"PnPQueryStopDevice: s3_1000 PCIe driver received a query stop");
	KdPrint (("s3_1000.sys: PnPQueryStopDevice\n"));

	deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
	
	// Ignore a QUERY_STOP prior to initialization.
	// just pass it down the`stack
	if (deviceExtension->DevicePnPState != Started)
	{
		// Record the action in the Event Log.
	    LogMsg(DeviceObject, L"PnPQueryStopDevice: s3_1000 PCIe driver was not started");
	    KdPrint (("s3_1000.sys: PnPQueryStopDevice - device was not started\n"));
        IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        status = DispatchClose(Irp, status, 0);
		return status;
	}
	
	if(!OkToStop(deviceExtension))
	{
      // Record the action in the Event Log.
	  LogMsg(DeviceObject, L"PnPQueryStopDevice: s3_1000 PCIe driver - not OK to stop device");
	  KdPrint (("s3_1000.sys: PnPQueryStopDevice - not OK to stop device\n"));
      IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
      status = DispatchClose(Irp, STATUS_UNSUCCESSFUL, 0);
      return status;
	}
	
    // Stall all queued requests. An IRP_STOP_DEVICE may shortly occur
    StallRequests(deviceExtension, TRUE);       // read
    StallRequests(deviceExtension, FALSE);      // write
	
    // Make sure any IRP's being processed have been completed.
    WaitForCurrentIrp(deviceExtension, TRUE);   // read IRP's
	WaitForCurrentIrp(deviceExtension, FALSE);  // write IRP's
	
	KdPrint(("s3_1000.sys: To STOPPENDING from %s\n", 
		         statenames[deviceExtension->DevicePnPState]));
	
	SET_NEW_PNP_STATE (deviceExtension, StopPending);
	
	// Make sure the Remove Lock is not set prior to passing this IRP down. 
	// If it is just complete the IRP
	// If we acquire the RemoveLock it will be released at the end of DispatchPnP
    // If this test fails, the driver has received an IRP_MN_REMOVE_DEVICE and
    // is waiting at an IoReleaseRemoveLockAndWait (multiple processors)
    if (&deviceExtension->RemoveLock && 
		status != IoAcquireRemoveLock(&deviceExtension->RemoveLock, Irp))
	{ // Someone has set the remove device lock
		KdPrint (("s3_1000.sys: PnPQueryStopDevice exits - RemoveLock set\n"));
        IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
		status = DispatchClose(Irp, status, 0);
		return status;
	}
	// Pass down the stack so other drivers have a chance
	// to accept or decline
	status = PnPPassDown(DeviceObject, Irp);
    
	// Record the action in the Event Log.
	LogMsg(DeviceObject, L"PnPQueryStopDevice: s3_1000 PCIe driver - query stop completed");
	KdPrint (("s3_1000.sys: PnPQueryStopDevice completed\n"));
    
    IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
	status = DispatchClose(Irp, status, 0);
	return status;

} // PnPQueryStopDevice


NTSTATUS
PnPStopDevice (
  IN PDEVICE_OBJECT     DeviceObject,
  IN PIRP               Irp
  )
{
	NTSTATUS              status = STATUS_SUCCESS;
	PDEVICE_EXTENSION     deviceExtension;
	ULONG				  idx;
	KIRQL				  oldIrql;

	PAGED_CODE ();

	// Record the action in the Event Log.
	LogMsg(DeviceObject, L"PnPStopDevice: s3_1000 PCIe driver received a stop device");
	KdPrint (("s3_1000.sys: PnPStopDevice\n"));

	deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    
    // Make sure any IRP's presently being executed finish prior to proceeding
    WaitForCurrentIrp(deviceExtension, TRUE);   // read IRP
	WaitForCurrentIrp(deviceExtension, FALSE);  // write IRP

	// Delete our Interrupt object
	if (deviceExtension->InterruptObject)
		IoDisconnectInterrupt( deviceExtension->InterruptObject );

	// unmap memory
	for (idx = 0; idx < deviceExtension->bar; idx++)
		if (deviceExtension->MemoryStart[idx])
			MmUnmapIoSpace (deviceExtension->MemoryStart[idx],
			  deviceExtension->MemoryLength[idx]);

	// Return the DMA adapters.   
	if (deviceExtension->pReadAdapter != NULL)
		deviceExtension->pReadAdapter->DmaOperations->PutDmaAdapter(deviceExtension->pReadAdapter);

	if (deviceExtension->pWriteAdapter != NULL)
		deviceExtension->pWriteAdapter->DmaOperations->PutDmaAdapter(deviceExtension->pWriteAdapter);

	// disable interface
	IoSetDeviceInterfaceState (&deviceExtension->InterfaceName, FALSE);

	KdPrint(("s3_1000.sys: To STOPPED from %s\n", 
		         statenames[deviceExtension->DevicePnPState]));
	SET_NEW_PNP_STATE (deviceExtension, Stopped);

	// Record the action in the Event Log.
	LogMsg(DeviceObject, L"PnPStopDevice: s3_1000 PCIe driver stopped");
	KdPrint (("s3_1000.sys: PnPStopDevice complete\n"));

	return status;
} // PnPStopDevice

NTSTATUS
PnPCancelStopDevice (
  IN PDEVICE_OBJECT     DeviceObject,
  IN PIRP               Irp
  )
{
	NTSTATUS              status = STATUS_SUCCESS;
	PDEVICE_EXTENSION     deviceExtension;
	ULONG				  idx;
	KIRQL				  oldIrql;

	PAGED_CODE ();

	// Record the action in the Event Log.
	LogMsg(DeviceObject, L"PnPCancelStopDevice: s3_1000 PCIe driver received a cancel stop");
	KdPrint (("s3_1000.sys: PnPCancelStopDevice\n"));

	deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    
	if (deviceExtension->DevicePnPState == StopPending)
	{						// we succeeded earlier query

		// Lower level drivers are presumably in the pending-stop state as
		// well, so we need to tell them that the stop has been cancelled
		// before we start sending IRPs down to them.
        status = SendIrpSync (deviceExtension, Irp);// wait for lower layers

		if (status == STATUS_SUCCESS)
	    {					// completed successfully
			KdPrint(("s3_1000.sys: To STARTED from PENDINGSTOP\n"));
			deviceExtension->DevicePnPState = Started;
			// restart any read IRP's
			RestartRequests(deviceExtension, deviceExtension->PhysicalDeviceObject, TRUE);
			// restart any write IRP's
			RestartRequests(deviceExtension, deviceExtension->PhysicalDeviceObject, FALSE);
		}					// completed successfully
		else
			KdPrint(("s3_1000.sys: Status %8.8lX returned by PDO for IRP_MN_CANCEL_STOP_DEVICE", status));

        IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);		
		status = DispatchClose(Irp, status, 0);
        
		// Record the action in the Event Log.
	    LogMsg(DeviceObject, L"PnPCancelStopDevice: s3_1000 PCIe driver SendIrpSync failed");
		KdPrint (("s3_1000.sys: PnPCancelStopDevice - SendIrpSync failed\n"));
        return status;
								// we succeeded earlier query
    }
	// Make sure the Remove Lock is not set prior to passing this IRP down. 
	// If it is just complete the IRP
	// If we acquire the RemoveLock it will be released at the end of DispatchPnP
    // If this test fails, the driver has received an IRP_MN_REMOVE_DEVICE and
    // is waiting at an IoReleaseRemoveLockAndWait (multiple processors)
    if (&deviceExtension->RemoveLock && 
		status != IoAcquireRemoveLock(&deviceExtension->RemoveLock, Irp))
	{ // Someone has set the remove device lock
		KdPrint (("s3_1000.sys: PnPQueryCancelStopDevice exits - RemoveLock set\n"));
        IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
		status = DispatchClose(Irp, status, 0);
		return status;
	}
	status = PnPPassDown(DeviceObject, Irp); // unexpected cancel
	
	RESTORE_PREVIOUS_PNP_STATE (deviceExtension);
	
	KdPrint(("s3_1000.sys: To %s from STOPPENDING in PnPCancelStopDevice\n", 
		         statenames[deviceExtension->DevicePnPState]));
	
	// Record the action in the Event Log.
	LogMsg(DeviceObject, L"PnPCancelStopDevice: s3_1000 PCIe driver stop canceled");
	KdPrint (("s3_1000.sys: PnPCancelStopDevice complete\n"));
    
	IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
	status = DispatchClose(Irp, status, 0);
	return status;
	
} // PnPCancelStopDevice

NTSTATUS
PnPQueryRemoveDevice(
    IN PDEVICE_OBJECT     DeviceObject,
    IN PIRP               Irp)
	{							// HandleQueryRemove
	   NTSTATUS status = STATUS_SUCCESS;
	   PDEVICE_EXTENSION deviceExtension;
       
       PAGED_CODE (); 
   
       deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
	   
       // Record the action in the Event Log.
	   LogMsg(DeviceObject, L"PnPQueryRemoveDevice: s3_1000 PCIe driver received a query remove device");
	   KdPrint (("s3_1000.sys: PnPQueryRemoveDevice\n"));
	   
	   if (deviceExtension->DevicePnPState == Started)
	   {						// currently working

	      // See if it's okay to remove this device. The test includes asking the client
		  // driver. Then stall the queues for the duration of the query.  

		  if (!OkToRemove(deviceExtension))
		  {
			  // Record the action in the Event Log.
	          LogMsg(DeviceObject, L"PnPQueryRemoveDevice: s3_1000 PCIe driver - not OK to remove device");
	          KdPrint (("s3_1000.sys: PnPQueryPRemoveDevice - not OK to remove device\n"));
              
			  IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
			  status = DispatchClose(Irp, STATUS_UNSUCCESSFUL, 0);
              return status;  // can't remove because busy
	      }
		  
          // stall any queued IRP's from being processed
          StallRequests(deviceExtension, TRUE);       // read
          StallRequests(deviceExtension, FALSE);      // write
          
          // Wait for any IRP's presently execoting to finish
          WaitForCurrentIrp(deviceExtension, TRUE);   // read IRP
          WaitForCurrentIrp(deviceExtension, FALSE);  // write IRP
 
	   } // currently working
	   
		KdPrint(("s3_1000.sys: To PENDINGREMOVE from %s\n", 
		           statenames[deviceExtension->DevicePnPState]));
	    SET_NEW_PNP_STATE (deviceExtension, RemovePending); 
	
	   // Make sure the Remove Lock is not set prior to passing this IRP down. 
	   // If it is just complete the IRP
	   // If we acquire the RemoveLock it will be released at the end of DispatchPnP
       // If this test fails, the driver has received an IRP_MN_REMOVE_DEVICE and
       // is waiting at an IoReleaseRemoveLockAndWait (multiple processors)
       if (&deviceExtension->RemoveLock && 
		   status != IoAcquireRemoveLock(&deviceExtension->RemoveLock, Irp))
	   { // Someone has set the remove device lock
		   KdPrint (("s3_1000.sys: PnPQueryCancelStopDevice exits - RemoveLock set\n"));
           IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
		   status = DispatchClose(Irp, status, 0);
		   return status;
	   }
	   // Pass down the stack so other drivers have a chance
	   // to accept or decline
	   status = PnPPassDown(DeviceObject, Irp);
	
	   // Record the action in the Event Log.
	   LogMsg(DeviceObject, L"PnPQueryRemoveDevice: s3_1000 PCIe driver remove device query received");
	   KdPrint (("s3_1000.sys: PnPQueryRemoveDevice completed\n"));
	 
	   return status;
	} //PnPQueryRemoveDevice

NTSTATUS
PnPRemoveDevice (
  IN PDEVICE_OBJECT     DeviceObject,
  IN PIRP               Irp
  )
{
	NTSTATUS              status = STATUS_SUCCESS;
	PDEVICE_EXTENSION     deviceExtension;

	PAGED_CODE ();
	
	// Record the action in the Event Log.
	LogMsg(DeviceObject, L"PnPRemoveDevice: s3_1000 PCIe driver received a remove device");
	KdPrint (("s3_1000.sys: PnPRemoveDevice\n"));
	
	status = PnPStopDevice(DeviceObject, Irp);

	deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    KdPrint(("s3_1000.sys: To DELETED from %s\n", 
		         statenames[deviceExtension->DevicePnPState]));
	SET_NEW_PNP_STATE (deviceExtension, Deleted);

    // We are basically finished so pass this IRP down the stack
    status = PnPPassDown(DeviceObject, Irp);
    
    IoReleaseRemoveLockAndWait(&deviceExtension->RemoveLock, Irp);
	// remove device from stack
	if (deviceExtension->NextStackDeviceObject)
		IoDetachDevice (deviceExtension->NextStackDeviceObject);

	// free DeviceExtension
	RtlFreeUnicodeString (&deviceExtension->InterfaceName);

	// delete device
	IoDeleteDevice (DeviceObject);

    // Record the action in the Event Log.
	LogMsg(DeviceObject, L"PnPRemoveDevice: s3_1000 device removed sucessfully");
	KdPrint (("s3_1000.sys: PnPRemoveDevice completed\n"));
    
	return status;
} // PnPRemoveDevice

NTSTATUS
PnPCancelRemoveDevice(
  IN PDEVICE_OBJECT     DeviceObject, 
  IN PIRP               Irp)
{
	NTSTATUS              status = STATUS_SUCCESS;
	PDEVICE_EXTENSION     deviceExtension;
    
    PAGED_CODE (); 

	// Record the action in the Event Log.
	LogMsg(DeviceObject, L"PnPCancelRemoveDevice: s3_1000 PCIe driver received a cancel remove device");
	KdPrint (("s3_1000.sys: PnPCancelRemoveDevice\n"));

	deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
	if (deviceExtension->DevicePnPState == RemovePending)
	{						// we succeeded earlier query

		// Lower-level drivers are presumably in the pending-remove state as
		// well, so we need to tell them that the remove has been cancelled
		// before we start sending IRPs down to them.
        status = SendIrpSync (deviceExtension, Irp); // wait for lower layers

		if (status == STATUS_SUCCESS)
		{					// completed successfully
			KdPrint(("s3_1000.sys: To %s from PENDINGREMOVE\n", 
			         statenames[deviceExtension->PreviousPnPState]));
			RestartRequests(deviceExtension, DeviceObject, TRUE );  // read
            RestartRequests(deviceExtension, DeviceObject, FALSE);  // write
		}					// completed successfully
		else
		{
			KdPrint(("s3_1000.sys: Status %8.8lX returned by PDO for IRP_MN_CANCEL_REMOVE_DEVICE", status));
		}
        RESTORE_PREVIOUS_PNP_STATE (deviceExtension);
	    KdPrint(("s3_1000.sys: To %s from STOPPENDING in PnPCancelRemoveDevice\n", 
		         statenames[deviceExtension->DevicePnPState]));
        IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
		status = DispatchClose(Irp, status, 0);
	    return status;
	}						// we succeeded earlier query
	
    // Make sure the Remove Lock is not set prior to passing this IRP down. 
	// If it is just complete the IRP
	// If we acquire the RemoveLock it will be released at the end of DispatchPnP
    // If this test fails, the driver has received an IRP_MN_REMOVE_DEVICE and
    // is waiting at an IoReleaseRemoveLockAndWait (multiple processors)
    if (&deviceExtension->RemoveLock && 
		status != IoAcquireRemoveLock(&deviceExtension->RemoveLock, Irp))
	{ // Someone has set the remove device lock
		LogMsg(DeviceObject, L"PnPCancelRemoveDevice: s3_1000 device remove lock was set!");
	    KdPrint (("s3_1000.sys: PnPQueryCancelStopDevice exits - RemoveLock set\n"));
        IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
	    status = DispatchClose(Irp, status, 0);
	    return status;
	}
	// Pass down the stack so other drivers have a chance
	// to accept or decline
	status = PnPPassDown(DeviceObject, Irp);
	
    LogMsg(DeviceObject, L"PnPCancelRemoveDevice: s3_1000 device remove canceled successfully!");
	KdPrint (("s3_1000.sys: PnPCancelRemoveDevice completed\n"));
	IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
	status = DispatchClose(Irp, status, 0);
	return status;
	 
} // PnPCancelRemoveDevice

NTSTATUS
PnPSurpriseRemoval(
  IN PDEVICE_OBJECT     DeviceObject,
  IN PIRP               Irp)
{
	NTSTATUS              status = STATUS_SUCCESS;
	PDEVICE_EXTENSION     deviceExtension;
	ULONG				  idx;
    KIRQL				  oldIrql;
    
    PAGED_CODE (); 

	LogMsg(DeviceObject, L"PnPSurpriseRemoval: s3_1000 device surprise removal!");
	KdPrint (("s3_1000.sys: PnPSurpriseRemoval\n"));

	deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
	AbortRequests(deviceExtension, STATUS_DELETE_PENDING, TRUE); // Abort read IRP's
	AbortRequests(deviceExtension, STATUS_DELETE_PENDING, FALSE); // Abort write IRP's

	KdPrint(("s3_1000.sys: To SURPRISEREMOVED from %s\n",statenames[deviceExtension->DevicePnPState]));
	SET_NEW_PNP_STATE(deviceExtension, SurpriseRemoved );

	// Disable the interface
	// this triggers notifications so apps can close their handles
	IoSetDeviceInterfaceState (&deviceExtension->InterfaceName, FALSE);	 
	
	// Delete our Interrupt object
	if (deviceExtension->InterruptObject)
		IoDisconnectInterrupt( deviceExtension->InterruptObject );

	// unmap memory
	for (idx = 0; idx < deviceExtension->bar; idx++)
		if (deviceExtension->MemoryStart[idx])
			MmUnmapIoSpace (deviceExtension->MemoryStart[idx],
			  deviceExtension->MemoryLength[idx]);

	// Return the DMA adapters. 
	if (deviceExtension->pReadAdapter != NULL)
		deviceExtension->pReadAdapter->DmaOperations->PutDmaAdapter(deviceExtension->pReadAdapter);

	if (deviceExtension->pWriteAdapter != NULL)
		deviceExtension->pWriteAdapter->DmaOperations->PutDmaAdapter(deviceExtension->pWriteAdapter);

    // Make sure the Remove Lock is not set prior to passing this IRP down. 
	// If it is just complete the IRP
	// If we acquire the RemoveLock it will be released at the end of DispatchPnP
    // If this test fails, the driver has received an IRP_MN_REMOVE_DEVICE and
    // is waiting at an IoReleaseRemoveLockAndWait (multiple processors)
    if (&deviceExtension->RemoveLock && 
		status != IoAcquireRemoveLock(&deviceExtension->RemoveLock, Irp))
	{ // Someone has set the remove device lock
		LogMsg(DeviceObject, L"PnPSurpriseRemoval: s3_1000 device surprise removal - remove lock was set!");
	    KdPrint (("s3_1000.sys: PnPSurpriseRemoval - remove lock was set!\n"));
        IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
		status = DispatchClose(Irp, status, 0);
		return status;
	}
	status = PnPPassDown(DeviceObject, Irp); // unexpected cancel
	
	LogMsg(DeviceObject, L"PnPSurpriseRemoval: s3_1000 device surprise removal completed");
	KdPrint (("s3_1000.sys: PnPSurpriseRemoval complete\n"));
	IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
	status = DispatchClose(Irp, status, 0);
	return status;
} // PnPSurpriseRemoval


// IRQL PASSIVE_LEVEL
NTSTATUS
DispatchPower (
  IN PDEVICE_OBJECT     DeviceObject,
  IN PIRP               Irp
  )
{
	NTSTATUS              status = STATUS_SUCCESS;
	PDEVICE_EXTENSION     deviceExtension;

	KdPrint (("s3_1000.sys: DispatchPower\n"));

	deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

	// pass IRP down the stack
	PoStartNextPowerIrp (Irp);
	IoSkipCurrentIrpStackLocation (Irp);
	status = PoCallDriver (deviceExtension->NextStackDeviceObject, Irp);

	KdPrint (("s3_1000.sys: DispatchPower complete\n"));
  
	return status;
} // DispatchPower


// IRQL unknown
NTSTATUS
CompletionRoutine (
  IN PDEVICE_OBJECT     DeviceObject,
  IN PIRP               Irp,
  IN PVOID              pContext
  )
{
    PDEVICE_EXTENSION deviceExtension;
    
	// Cast the pContext argument into its true identity as an event pointer.
	PKEVENT event = (PKEVENT) pContext;
    deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

	// Raise the event flag to signal the waiting thread.
	KeSetEvent (event, IO_NO_INCREMENT, FALSE);
    IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
	// Hold off higher level processing until this level completes.
	return STATUS_MORE_PROCESSING_REQUIRED;
} // CompletionRoutine


// IRQL PASSIVE_LEVEL
// This function sets up an I/O completion routine to be called by a lower level driver.
// During the call to the lower level driver, this thread is blocked to ensure that our
// PnP handlers will all be called at PASSIVE_LEVEL.
NTSTATUS
SendIrpSync (
  IN PDEVICE_EXTENSION     deviceExtension,
  IN PIRP               Irp
  )
{
    PDEVICE_OBJECT        pLowerDevice = deviceExtension->NextStackDeviceObject;
	KEVENT                event;			// Reserve space for the event in non-paged memory.
	NTSTATUS              status;

	PAGED_CODE ();

  IoCopyCurrentIrpStackLocationToNext (Irp);

  // There is no way to guarantee that I/O Completion routines will be called at PASSIVE_LEVEL,
  // (for example, any callback from a DPC function will execute at DISPATCH_LEVEL).  To ensure
  // that this handler will execute at PASSIVE_LEVEL, a KERNEL EVENT (analagous to a flag) is used.
  // The event thread will wait until the flag is "raised", at which time it will be scheduled
  // for execution.
  // Using the space allocated above in non-paged memory, initialize the Kernel Event with the
  // "flag" lowered.
  KeInitializeEvent (&event, NotificationEvent, FALSE);
  
  // Register our callback (completion) routine, passing the event object we have just initialized.
  // All boolean values are set to TRUE, meaning our completion routine will always be invoked by
  // the lower level driver.
  IoSetCompletionRoutineEx (
    deviceExtension->NextStackDeviceObject,
    Irp,
    CompletionRoutine,
    &event,
    TRUE,
    TRUE,
    TRUE);

  // Call the lower level driver - in this case the Physical Driver.
  status = IoCallDriver (pLowerDevice, Irp);

  // Wait for the lower level driver to complete its function.
  if (status == STATUS_PENDING) 
  {
    KeWaitForSingleObject (
      &event,
      Executive,
      KernelMode,
      FALSE,
      NULL);
    status = Irp->IoStatus.Status;
  }

  return status;
} // SendIrpSync

// IRQL PASSIVE_LEVEL
// This function allocates DMA Adapter objects.  It returns STATUS_SUCCESS if all goes well, and
// a failure code if things go wrong.
NTSTATUS
GetDMAInfo (
  IN PDEVICE_OBJECT     pDeviceObject,
  IN PIRP               Irp				// RRW - is this really required here?
  )
{
	NTSTATUS              status = STATUS_SUCCESS;
	PDEVICE_EXTENSION     devExt;
	DEVICE_DESCRIPTION	  dd;
	PIO_STACK_LOCATION    ioStack;
	KIRQL				  oldIrql;

	devExt = (PDEVICE_EXTENSION) pDeviceObject->DeviceExtension;
	ioStack = IoGetCurrentIrpStackLocation (Irp);

	// Zero out the entire device description structure.  This is convenient since it is
	// required that all fields not explicitly set be initialized to zero.
	RtlZeroMemory(&dd, sizeof(dd));

	// Set up the fields of the DMA device description structure as they apply to our hardware.
	dd.Version = DEVICE_DESCRIPTION_VERSION1;
	dd.Master = TRUE;
	dd.ScatterGather = FALSE;
	dd.Dma32BitAddresses = TRUE;
	dd.InterfaceType = PCIBus;
	dd.MaximumLength = MAX_TRANSFER_SIZE;		

	// Allocate one Adapter object for each simultaneous operation that can be performed
	// by the hardware.  Since our device can operate in full duplex, there will be one
	// Adapter object for a DMA Write (transfer to the device) and one for a DMA Read (transfer
	// from the device.  The call to IoGetDmaAdapter returns the maximum number of map registers
	// that a transfer is allowed to request.  If the number of registers * 4K bytes (map reg size) is less 
	// than the entire transfer size, multiple calls to the DMA engine must be coordinated by
	// the driver to complete the operation.
	devExt->pReadAdapter = IoGetDmaAdapter(devExt->PhysicalDeviceObject, 
										  &dd,
										  &devExt->ReadMapRegMax);
	if (devExt->pReadAdapter == NULL)
	{
		LogMsg(pDeviceObject, L"GetDMAInfo: PCIe DMA Read Adapter Object creation failed!");
		status = STATUS_NO_MEMORY;
		return status;
	}

	devExt->pWriteAdapter = IoGetDmaAdapter(devExt->PhysicalDeviceObject, 
										   &dd,
										   &devExt->WriteMapRegMax);

	if (devExt->pWriteAdapter == NULL)
	{
		LogMsg(pDeviceObject, L"GetDMAInfo: PCIe DMA Write Adapter Object creation failed!");
		status = STATUS_NO_MEMORY;

		// Return the allocated Read Adapter object, but this call must be made at IRQL_DISPATCH_LEVEL
		// RRW - See page 160 of the Windows Vol 2 ref manual for info on returning adapter in response
		// to a STOP_DEVICE Irp.
		KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
		devExt->pReadAdapter->DmaOperations->PutDmaAdapter(devExt->pReadAdapter);
		KeLowerIrql(oldIrql);
		return status;
	}

	LogMsg(pDeviceObject, L"GetDMAInfo: PCIe device created successfully!");

	return status;
}