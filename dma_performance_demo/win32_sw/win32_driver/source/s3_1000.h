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
#ifndef _S3_1000_H_
#define _S3_1000_H_

#include <wdm.h>
#include "ioctrl.h"

// defined types
typedef enum _DEVICE_PNP_STATE {

  NotStarted = 0,         // Not started yet
  Deleted,                // Device has received the REMOVE_DEVICE IRP
  SurpriseRemoved,        // Device has received the SURPRISE_REMOVE IRP
  Stopped,                // Device has received the STOP_DEVICE IRP
  RemovePending,          // Device has received the QUERY_REMOVE IRP
  FailHardware,           // Fail hardware transfers
  StopPending,            // Device has received the QUERY_STOP IRP
  FailSoftware,           // Fail incoming IRPs
  Started                 // Device has received the START_DEVICE IRP

} DEVICE_PNP_STATE;

typedef struct _DEVICE_EXTENSION  {

  PDEVICE_OBJECT        PhysicalDeviceObject;
  PDEVICE_OBJECT        FunctionDeviceObject;
  PDEVICE_OBJECT        NextStackDeviceObject;

  UNICODE_STRING        InterfaceName;

  DEVICE_PNP_STATE      DevicePnPState;
  DEVICE_PNP_STATE      PreviousPnPState;

  // Memory Information (includes support for 2 bars, although only 1 is used)
  ULONG					bar;
  ULONG					baseOffset[2];		  // Offset to beginning of RAM (BRAM)
  PHYSICAL_ADDRESS      Memory[2];
  PVOID                 MemoryStart[2];
  ULONG                 MemoryOffset[2];	  // Offset within RAM
  ULONG                 MemoryLength[2];

  // Bus Master DMA Information
  PDMA_ADAPTER			pReadAdapter;
  ULONG					ReadMapRegMax;
  ULONG					ReadMapRegNeeded;
  ULONG					ReadLength;
  ULONG					ReadOffset;
  ULONG					ReadTotalLength;
  PIRP					ReadIrp;		      // In-progress read request 
  PVOID					ReadMapRegisterBase;
  KSPIN_LOCK			ReadQueueLock;		  // Interlock object for the Read Irp list  
  LIST_ENTRY			ReadQueue;			  // Doubly-linked list of pending Read Irqs
  BOOLEAN				ReadDone;
  PSCATTER_GATHER_LIST	pReadSGL;
  PDMA_ADAPTER			pWriteAdapter;
  ULONG					WriteMapRegMax;
  ULONG					WriteMapRegNeeded;
  ULONG					WriteLength;
  ULONG					WriteOffset;
  ULONG					WriteTotalLength;
  PIRP					WriteIrp;			  // In-progress write request 
  PVOID					WriteMapRegisterBase;
  KSPIN_LOCK			WriteQueueLock;		  // Interlock object for the Write Irp list  
  LIST_ENTRY			WriteQueue;			  // Doubly-linked list of pending Write Irqs
  BOOLEAN				WriteDone;
  PSCATTER_GATHER_LIST	pWriteSGL;
  NTSTATUS              readabortstatus;      // Used in restarting read IRP's
  LONG                  readstallcount;       // Used to stall new read IRP's if device stalled
  KEVENT                readevStop;           // Used in WaitForCurrentIrp
  NTSTATUS              writeabortstatus;     // Used in restarting write IRP's
  LONG                  writestallcount;      // Used to stall new wrire IRP's if device stalled
  KEVENT                writeevStop;          // Used in WaitForCurrentIrp
  IO_REMOVE_LOCK        RemoveLock;           // Used to protect Device removal
  
  // Interrupt Information
  PKINTERRUPT			InterruptObject;
  KIRQL					IRQL;
  ULONG					Vector;
  KAFFINITY				Affinity;
  KINTERRUPT_MODE		TriggerMode;
  
  // timer variables
  LONG                  timer;			      // # seconds remaining in timeout
  BOOLEAN               iotimeout;            // If RUE an IO timeout has occured
  BOOLEAN               busy;			      // device is busy with an IRP

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

// macros
#define INITIALIZE_PNP_STATE(_Data_)    \
        (_Data_)->DevicePnPState =  NotStarted;\
        (_Data_)->PreviousPnPState = NotStarted;

#define SET_NEW_PNP_STATE(_Data_, _state_) \
        (_Data_)->PreviousPnPState =  (_Data_)->DevicePnPState;\
        (_Data_)->DevicePnPState = (_state_);

#define RESTORE_PREVIOUS_PNP_STATE(_Data_)   \
        (_Data_)->DevicePnPState =   (_Data_)->PreviousPnPState;

// forward declarations

// s3_1000.c
VOID 
OnTimer ( 
  IN PDEVICE_OBJECT     DeviceObject, 
  IN PDEVICE_EXTENSION  DeviceExtension
  );
  
VOID 
CheckTimer (
  IN PDEVICE_EXTENSION  DeviceExtension
  );
  
VOID
SetIoTimerValue (
  IN PDEVICE_EXTENSION  DeviceExtension
  );
  
NTSTATUS
DriverEntry (
  IN PDRIVER_OBJECT     DriverObject,
  IN PUNICODE_STRING    RegistryPath
  );

VOID
Unload (
  IN PDRIVER_OBJECT     DriverObject
  );

NTSTATUS
Dispatch (
  IN PDEVICE_OBJECT     DeviceObject,
  IN PIRP               Irp
  );

// pnp.c
NTSTATUS
AddDevice (
  IN PDRIVER_OBJECT     DriverObject,
  IN PDEVICE_OBJECT     PhysicalDeviceObject
  );

NTSTATUS
Control (
  PDEVICE_OBJECT DeviceObject,
  PIRP Irp
  );

NTSTATUS
DispatchPnP (
  IN PDEVICE_OBJECT     DeviceObject,
  IN PIRP               Irp
  );

NTSTATUS
DispatchPower (
  IN PDEVICE_OBJECT     DeviceObject,
  IN PIRP               Irp
  );

VOID
LogMsg(
	IN PDEVICE_OBJECT deviceObject,		//  Device Object
	IN const wchar_t *logMsg			//  Message for event log
	);

NTSTATUS 
StartDMAWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS 
StartDMARead(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
	);

BOOLEAN BDMA_Isr (
	IN PKINTERRUPT pIntObj,
	IN PVOID pServiceContext	
	);

VOID
DpcForIsr(
	IN PKDPC pDpc,
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp,
	IN PVOID pContext
	);
	
VOID CancelReadRequest(
   PDEVICE_EXTENSION DeviceExtension, 
   PIRP Irp);
   
VOID CancelWriteRequest(
   PDEVICE_EXTENSION DeviceExtension, 
   PIRP Irp);
   
VOID ReadCancelRoutine(
   PDEVICE_OBJECT fdo,
   PIRP Irp);
   
VOID WriteCancelRoutine(
   PDEVICE_OBJECT fdo,
   PIRP Irp);

// Identifies the driver by its GUID in the WDM Model.  Use Microsoft GUIDGEN tool to create a unique ID.
// {8C3A04F6-26EC-44d2-9037-8C67D24CE4CF}
static const GUID GuidDriver = 
{ 0x8c3a04f6, 0x26ec, 0x44d2, { 0x90, 0x37, 0x8c, 0x67, 0xd2, 0x4c, 0xe4, 0xcf } };

#endif // _S3_1000_H_
