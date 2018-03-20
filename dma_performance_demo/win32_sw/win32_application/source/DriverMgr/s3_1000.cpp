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
// s3_1000.cpp : Implementation of Cs3_1000
#include "stdafx.h"
#include "DriverMgr.h"
#include "s3_1000.h"
#include <comdef.h>
#include "winioctl.h"	// Contains the CTRL_CODE macro definition
#include "ioctrl.h"		// Contains Spartan3-1000 Control Codes

#define	MY_NAME	_bstr_t("PCIe Demo Driver Manager 2")

enum OpStatus  { pass = 0, fail, running, error };

// Identifies the driver by its GUID in the WDM Model
// {8C3A04F6-26EC-44d2-9037-8C67D24CE4CF}
static const GUID GuidDriver = 
{ 0x8c3a04f6, 0x26ec, 0x44d2, { 0x90, 0x37, 0x8c, 0x67, 0xd2, 0x4c, 0xe4, 0xcf } };
/////////////////////////////////////////////////////////////////////////////
// Cs3_1000

void Cs3_1000::InitializeRegInfo()
{
	RegInfo[2].offset = WRITE_ADDR_OFFSET;
	RegInfo[2].writeControlCode = IOCTL_SET_DMA_WRITE_ADDR;
	RegInfo[2].readControlCode = IOCTL_GET_DMA_WRITE_ADDR;
	RegInfo[2].name = "DMA Write Address Register";

	RegInfo[3].offset = WRITE_SIZE_OFFSET;
	RegInfo[3].writeControlCode = IOCTL_SET_DMA_WRITE_SIZE;
	RegInfo[3].readControlCode = IOCTL_GET_DMA_WRITE_SIZE;
	RegInfo[3].name = "DMA Write Size Register";

	RegInfo[4].offset = WRITE_COUNT_OFFSET;
	RegInfo[4].writeControlCode = IOCTL_SET_DMA_WRITE_COUNT;
	RegInfo[4].readControlCode = IOCTL_GET_DMA_WRITE_COUNT;
	RegInfo[4].name = "DMA Write Count Register";

	RegInfo[5].offset = WRITE_PATTERN_OFFSET;
	RegInfo[5].writeControlCode = IOCTL_SET_DMA_WRITE_PATTERN;
	RegInfo[5].readControlCode = IOCTL_GET_DMA_WRITE_PATTERN;
	RegInfo[5].name = "DMA Write Pattern Register";

	RegInfo[6].offset = READ_PATTERN_OFFSET;
	RegInfo[6].writeControlCode = IOCTL_SET_DMA_READ_PATTERN;
	RegInfo[6].readControlCode = IOCTL_GET_DMA_READ_PATTERN;
	RegInfo[6].name = "DMA Read Pattern Register";

	RegInfo[7].offset = READ_ADDR_OFFSET;
	RegInfo[7].writeControlCode = IOCTL_SET_DMA_READ_ADDR;
	RegInfo[7].readControlCode = IOCTL_GET_DMA_READ_ADDR;
	RegInfo[7].name = "DMA Read Address Register";

	RegInfo[8].offset = READ_SIZE_OFFSET;
	RegInfo[8].writeControlCode = IOCTL_SET_DMA_READ_SIZE;
	RegInfo[8].readControlCode = IOCTL_GET_DMA_READ_SIZE;
	RegInfo[8].name = "DMA Read Size Register";

	RegInfo[9].offset = READ_COUNT_OFFSET;
	RegInfo[9].writeControlCode = IOCTL_SET_DMA_READ_COUNT;
	RegInfo[9].readControlCode = IOCTL_GET_DMA_READ_COUNT;
	RegInfo[9].name = "DMA Read Count Register";
}

HANDLE Cs3_1000::OpenDeviceByGuid(LPGUID pGuid)
{
	SP_DEVICE_INTERFACE_DATA			data;
	PSP_DEVICE_INTERFACE_DETAIL_DATA	detail;	
	DWORD								error;
	HANDLE								hDevice;
	DWORD								i;
	HDEVINFO							info;
	DWORD								reqSize;

	// get handle to relevant device information set
  
	info = SetupDiGetClassDevs(
      pGuid,
      NULL,
      NULL,
      DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

	if (info == INVALID_HANDLE_VALUE)  
	{
		GetSystemMessage();
		Fire_DisplayMessage(_bstr_t((LPCTSTR)SystemMessage), MY_NAME);
		return INVALID_HANDLE_VALUE;
	}

	// get interface data 

	data.cbSize = sizeof (data);

	for (i = 0, error = 0; error != ERROR_NO_MORE_ITEMS; i++)  
	{
		if (!SetupDiEnumDeviceInterfaces (info, NULL, pGuid, i, &data))  
		{
			error = GetLastError();

			if (error != ERROR_NO_MORE_ITEMS)
			{
				SetLastError(error);
				GetSystemMessage();
				Fire_DisplayMessage(_bstr_t((LPCTSTR)SystemMessage), MY_NAME);
				SetupDiDestroyDeviceInfoList (info);
				return INVALID_HANDLE_VALUE;
			}
		}
	}
  
	// get size of symbolic link name

	SetupDiGetDeviceInterfaceDetail (info, &data, NULL, 0, &reqSize, NULL);

	error = GetLastError();
  
	if (error != ERROR_INSUFFICIENT_BUFFER)
	{
		SetLastError(error);
		GetSystemMessage();
		Fire_DisplayMessage(_bstr_t((LPCTSTR)SystemMessage), MY_NAME);
		SetupDiDestroyDeviceInfoList (info);
		return INVALID_HANDLE_VALUE;
	}
  
	// get symbolic link name
	detail = PSP_INTERFACE_DEVICE_DETAIL_DATA(new char[reqSize]);
  
	if (!detail)  
	{
		SetupDiDestroyDeviceInfoList (info);
		return INVALID_HANDLE_VALUE;
	}

	detail->cbSize = sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);

	if (!SetupDiGetDeviceInterfaceDetail (
		  info, &data, detail, reqSize, &reqSize, NULL))  
	{
		GetSystemMessage();
		Fire_DisplayMessage(_bstr_t((LPCTSTR)SystemMessage), MY_NAME);
		SetupDiDestroyDeviceInfoList (info);
		delete detail;
		return INVALID_HANDLE_VALUE;
	}
   
	// Open a "File", which in this context means that we are receiving a
	// handle to a Device Object.  This handle is used in subsequent calls
	// to identify the Device Object, and ultimately interact with the
	// physical device it represents.

	// Take note of the attribute "FILE_FLAG_OVERLAPPED".  If your physical
	// device supports simultaneous operations (for example, a DMA device
	// that can perform 1 read and 1 write operation at the same time), you
	// MUST specify this attribute.  If you do not, when your application
	// attempts to perform a read (ReadFile) or write (WriteFile) on the device
	// the CALL WILL BLOCK in the I/O Manager, and your application will freeze.
	// This obviously prevents you from programming any simultaneous operations.
	// The complication with specifying the overlapped attribute is that you
	// now must provide an Overlapped structure in your Writefile and Readfile
	// calls.  When your operation starts at the device driver level, it will
	// eventually return STATUS_PENDING to the I/O Manager.  The I/O Manager 
	// will then return an ERROR_IO_PENDING status to the calling function, 
	// allowing your application to proceed.  This makes use of the Overlapped
	// structure, and is not really an error at all (assuming that your I/O
	// operation is supposed to be pending, of course!).

	hDevice = CreateFile (
      detail->DevicePath,
      GENERIC_READ | GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE,
      NULL,
      OPEN_EXISTING,
      FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
      NULL);

	if (hDevice == INVALID_HANDLE_VALUE)
	{
		GetSystemMessage();
		Fire_DisplayMessage(_bstr_t((LPCTSTR)SystemMessage), MY_NAME);
	}

	SetupDiDestroyDeviceInfoList (info);
	delete detail;

	return hDevice;
}

void Cs3_1000::GetSystemMessage()
{
	LPVOID lpMsgBuf;
	
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
		);
	strcpy(SystemMessage, "Relayed from Windows I/O Manager:\n\n");
	strcat (SystemMessage, (LPTSTR)lpMsgBuf);

	// Free the buffer.
	LocalFree( lpMsgBuf );
}

long Cs3_1000::SetRegister(unsigned long offset, unsigned long value)
{

	ULONG	nOutput;				// bytes returned from IO ctrl call
	char	message[132];			// local mesage buffer
	ULONG	regValue;				// register value read

	// Set the target (on host) address for the DMA transfer.
	if(!DeviceIoControl(hs3_1000, 
						RegInfo[offset / 4].writeControlCode, 
						&value,
						sizeof(value),
						NULL,
						0,
						&nOutput,
						NULL))
	{
		GetSystemMessage();
		sprintf(message, "Cannot set %s.  Press OK for OS info.", RegInfo[offset / 4].name);
		Fire_DisplayMessage(_bstr_t(message), MY_NAME);
		Fire_DisplayMessage(_bstr_t((LPCTSTR)SystemMessage), MY_NAME);
		return 1;
	}

	// Verify that the register has been correctly set.
	if(!DeviceIoControl(hs3_1000, 
						RegInfo[offset / 4].readControlCode, 
						NULL,
						0,
						&regValue,
						sizeof(regValue),
						&nOutput,
						NULL))
	{
		GetSystemMessage();
		sprintf(message, "Cannot read %s.  Press OK for OS info.", RegInfo[offset / 4].name);
		Fire_DisplayMessage(_bstr_t(message), MY_NAME);
		Fire_DisplayMessage(_bstr_t((LPCTSTR)SystemMessage), MY_NAME);
		return 1;
	}
	
	if (regValue != (regValue & value))
	{
		sprintf(message, "%s verification failed.",  RegInfo[offset / 4].name);
		Fire_DisplayMessage(_bstr_t(message), MY_NAME);
		return 1;
	}

	return 0;		// Successful write and verify of register
}

// Public Methods from the COM Interface Is3_1000
STDMETHODIMP Cs3_1000::OpenDevice(long *status)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	if (hs3_1000 != INVALID_HANDLE_VALUE)
	{
		Fire_DisplayMessage(_bstr_t("Driver is already open."), MY_NAME);
		*status = 0;
		return S_OK;
	}

	// WDM
	hs3_1000 = OpenDeviceByGuid( (LPGUID) &GuidDriver);

	if (hs3_1000 && hs3_1000 != INVALID_HANDLE_VALUE)
	{
		*status = 0;
    } 
	
	else 
	{
		Fire_DisplayMessage(_bstr_t("Cannot locate device in this system.  Please ensure that:\n  1: The hardware is installed.\n  2: The device driver is running."), MY_NAME);
		*status = 1;
	}

	return S_OK;
}

STDMETHODIMP Cs3_1000::CloseDevice()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	if (hs3_1000 != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hs3_1000);
		hs3_1000 = INVALID_HANDLE_VALUE;
	}

	else
	{
		Fire_DisplayMessage(_bstr_t("Driver is not open."), MY_NAME);
	}

	return S_OK;
}

STDMETHODIMP Cs3_1000::GetDIPState(long *state)
{
	ULONG	nOutput;				// bytes returned from IO ctrl call

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	if (hs3_1000 && hs3_1000 != INVALID_HANDLE_VALUE)
	{
		if(!DeviceIoControl(hs3_1000, 
							MDS_IOCTL_GET_DIP_STATE, 
							NULL,
							0,
							state,
							sizeof(LONG),
							&nOutput,
							NULL))
		{
			GetSystemMessage();
			Fire_DisplayMessage(_bstr_t((LPCTSTR)SystemMessage), MY_NAME);
		}
	}

	else
	{
		Fire_DisplayMessage(
		  _bstr_t("Cannot read DIP state when the Device Driver is not running."), MY_NAME);
	}

	return S_OK;
}

STDMETHODIMP Cs3_1000::GetLEDState(long index, long *state)
{
	ULONG	nOutput;				// bytes returned from IO ctrl call

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	if (hs3_1000 && hs3_1000 != INVALID_HANDLE_VALUE)
	{
		if(!DeviceIoControl(hs3_1000, 
							MDS_IOCTL_GET_USER_LED_STATE, 
							&index,
							sizeof(LONG),
							state,
							sizeof(LONG),
							&nOutput,
							NULL))
		{
			GetSystemMessage();
			Fire_DisplayMessage(_bstr_t((LPCTSTR)SystemMessage), MY_NAME);
		}
	}

	else
	{
		Fire_DisplayMessage(
		  _bstr_t("Cannot retrieve LED state when the Device Driver is not running."), MY_NAME);
	}

	return S_OK;
}

STDMETHODIMP Cs3_1000::GetPBState(long *state)
{
	ULONG	nOutput;				// bytes returned from IO ctrl call

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	if (hs3_1000 && hs3_1000 != INVALID_HANDLE_VALUE)
	{
		if(!DeviceIoControl(hs3_1000, 
							MDS_IOCTL_GET_PB_STATE, 
							NULL,
							0,
							state,
							sizeof(LONG),
							&nOutput,
							NULL))
		{
			GetSystemMessage();
			Fire_DisplayMessage(_bstr_t((LPCTSTR)SystemMessage), MY_NAME);
		}
	}

	else
	{
		Fire_DisplayMessage(
		  _bstr_t("Cannot retrieve Push Button state when the Device Driver is not running."), MY_NAME);
	}

	return S_OK;
}

STDMETHODIMP Cs3_1000::SetLEDState(long index, long state)
{
	ULONG	nOutput;				// bytes returned from IO ctrl call
	ULONG	param[2];				// [0] = index, [1] = value to set

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	if (hs3_1000 && hs3_1000 != INVALID_HANDLE_VALUE)
	{
		param[0] = (ULONG)index;
		param[1] = (ULONG)state;

		if(!DeviceIoControl(hs3_1000, 
							MDS_IOCTL_SET_USER_LED_STATE, 
							&param,
							sizeof(LONG) + sizeof(LONG),
							NULL,
							0,
							&nOutput,
							NULL))
		{
			GetSystemMessage();
			Fire_DisplayMessage(_bstr_t((LPCTSTR)SystemMessage), MY_NAME);
		}
	}

	else
	{
		Fire_DisplayMessage(
		  _bstr_t("LED State cannot be set when the Device Driver is not running."), MY_NAME);
	}

	return S_OK;
}

STDMETHODIMP Cs3_1000::Reset(long *status)
{
	ULONG	nOutput;				// bytes returned from IO ctrl call
	SYSTEMTIME time0, time1;

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	if (hs3_1000 && hs3_1000 != INVALID_HANDLE_VALUE)
	{
		*status = 0;				// Assume success for now

		// Put the device into Reset.  This clears the Read-Only status registers.
		if(!DeviceIoControl(hs3_1000, 
							IOCTL_INITIATOR_RESET, 
							NULL,
							0,
							NULL,
							0,
							&nOutput,
							NULL))
		{
			GetSystemMessage();
			Fire_DisplayMessage(_bstr_t("Cannot perform Initiator Reset.  Failed to set DCR bit 0 high.  Press OK for OS info."), MY_NAME);
			Fire_DisplayMessage(_bstr_t((LPCTSTR)SystemMessage), MY_NAME);
			*status = 1;
			
			return S_OK;
		}
		
		// Wait for 125 ns (from HW specification) to ensure the reset operation is completed.
		// Note:  We end up waiting for at least 1 Millisecond since that's the finest granularity
		//        of the Windows timer function.
		GetSystemTime(&time0);

		do
		{
			GetSystemTime(&time1);
		} while (time1.wMilliseconds == time0.wMilliseconds);

		// Bring the device out of Reset.
		if(!DeviceIoControl(hs3_1000, 
							IOCTL_CLEAR_INITIATOR_RESET, 
							NULL,
							0,
							NULL,
							0,
							&nOutput,
							NULL))
		{
			GetSystemMessage();
			Fire_DisplayMessage(_bstr_t("Cannot bring Initiator out of reset.  Failed to set DCR bit 0 low.  Press OK for OS info."), MY_NAME);
			Fire_DisplayMessage(_bstr_t((LPCTSTR)SystemMessage), MY_NAME);
			*status = 1;
			
			return S_OK;
		}

	}

	else
	{
		Fire_DisplayMessage(_bstr_t("Cannot perform Initiator Reset.  PCIe Device Driver is not running."), MY_NAME);
		*status = 1;
	}

	return S_OK;
}

STDMETHODIMP Cs3_1000::SetDMAWrite(long size, long count, long pattern, long *status)
{
	ULONG			addr;				// Address of memory buffer
	unsigned long *	idx;				// pointer into read buffer
	ULONG			nTransfer;			// bytes read/written
	OVERLAPPED		asyncWrite;			// Required structure for asynchronous file operations
	DWORD			lastError;			// Error code returned from Write File operation

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	if (hs3_1000 && hs3_1000 != INVALID_HANDLE_VALUE)
	{
		*status = 0;											// Assume success for now
		memset(TLPWriteBuffer, 0, sizeof(TLPWriteBuffer));		// Clear our target buffer on the host

		// Set the host address register to the Virtual Address of our user buffer.  This is NOT the
		// actual address that will be used by the DMA Device - it requires a logical address (this is
		// actually the address of a set of contiguous memory locations set up by the OS to hold a 
		// copy of the user buffer, translated to the device address space).  The value set in this
		// register will be accessed later by the driver and converted to the logical address required.
		addr = (unsigned long) TLPWriteBuffer;

		// Set the DMA Write Address register with the value of our target buffer.  Return if the
		// register cannot be written and verified.
		*status = SetRegister(WRITE_ADDR_OFFSET, addr);

		if (*status != 0)
			return S_OK;		// Failure to set and verify register

		// Set the DMA Write TLP size register with the value specified in the GUI.  Return if the
		// register cannot be written and verified.
		*status = SetRegister(WRITE_SIZE_OFFSET, size);

		if (*status != 0)
			return S_OK;		// Failure to set and verify register

		// Set the DMA Write count register with the value specified in the GUI.  Return if the
		// register cannot be written and verified.
		*status = SetRegister(WRITE_COUNT_OFFSET, count);

		if (*status != 0)
			return S_OK;		// Failure to set and verify register

		// Set the DMA Write pattern register with the value specified in the GUI.  Return if the
		// register cannot be written and verified.
		*status = SetRegister(WRITE_PATTERN_OFFSET, pattern);

		if (*status != 0)
			return S_OK;		// Failure to set and verify register

		// Record the number of words and pattern to be written for later verification.
		WriteLen = count * size;
		WritePattern = pattern;									
	
		// Fill our Write buffer on the host with a default pattern to help in error detection.
		for (idx = TLPWriteBuffer; idx < TLPWriteBuffer + MAX_DMA_TRANSFER; idx++)
			*idx = 0x12345678;

		Operation = write;				// Write is always called first by the GUI (v.0.4.0.3)

		// Set up a contiguous area (System Scatter/Gather) for our user buffer.  This relieves us from the
		// responsibility of gathering up all the different pieces of physical memory which comprise our
		// user buffer in virtual memory, and then programming the DMA engine multiple times to transfer each
		// segment.
		// Note that this does NOT actually kick off the DMA engine - because we have a Bus Master device,
		// that is done later via the StartDMA function.
		asyncWrite.Offset = 0;				// Start transfer at the beginning of the file
		asyncWrite.OffsetHigh = 0;			// We don't need this, and I don't understand it anyway
		asyncWrite.hEvent = 0;				// We don't need a signal event, the device will signal us

		// The DMA device we are ultimately communicating with in this call is capable of simultaneous
		// reads and writes.  On success, this function will return an error code indicating that the
		// requested operation is PENDING.  This allows the application to continue and program another
		// file operation.  This is possible because when we created the device object with CreateFile, 
		// the FILE_FLAG_OVERLAPPED attribute was specified.  That forces us to provide the asyncWrite
		// structure on the file operation call, but has the distinct advantage that it DOES NOT BLOCK
		// as it would if we did not specify overlapped operation.
		if (WriteFile(hs3_1000, TLPWriteBuffer, size * count * sizeof(ULONG), &nTransfer, &asyncWrite) == 0)
		{
			lastError = GetLastError();

			if (lastError != ERROR_IO_PENDING)
			{
				SetLastError(lastError);
				GetSystemMessage();
				Fire_DisplayMessage(_bstr_t("Cannot perform BMDMA Write System Scatter/Gather.  Press OK for OS info."), MY_NAME);
				Fire_DisplayMessage(_bstr_t((LPCTSTR)SystemMessage), MY_NAME);
				*status = 1;

				return S_OK;
			}
		}
	}

	else
	{
		Fire_DisplayMessage(_bstr_t("Cannot perform DMA Write set up.  PCIe Device Driver has been stopped."), MY_NAME);
		*status = 1;
	}

	return S_OK;
}

STDMETHODIMP Cs3_1000::SetDMARead(long size, long count, long pattern, long *status)
{
	ULONG			addr;				// Address of memory buffer
	unsigned long *	idx;				// pointer into read buffer
	ULONG			nTransfer;			// bytes read/written
	OVERLAPPED		asyncRead;			// Required structure for asynchronous file operations
	DWORD			lastError;			// Error code returned from Read File operation

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	if (hs3_1000 && hs3_1000 != INVALID_HANDLE_VALUE)
	{
		*status = 0;											// Assume success for now

		// Set the host address register to the Virtual Address of our user buffer.  This is NOT the
		// actual address that will be used by the DMA Device - it requires a logical address (this is
		// actually the address of a set of contiguous memory locations set up by the OS to hold a 
		// copy of the user buffer, translated to the device address space).  The value set in this
		// register will be accessed later by the driver and converted to the logical address required.
		addr = (unsigned long) TLPReadBuffer;

		// Set the host address register to the Virtual Address of our user buffer.  This is NOT the
		// actual address that will be used by the DMA Device - it requires a logical address (this is
		// actually the address of a set of contiguous memory locations set up by the OS to hold a 
		// copy of the user buffer, translated to the device address space).  The value set in this
		// register will be accessed later by the driver and converted to the logical address required.
		*status = SetRegister(READ_ADDR_OFFSET, addr);

		if (*status != 0)
			return S_OK;		// Failure to set and verify register

		// Set the DMA Read TLP size register with the value specified in the GUI.  Return if the
		// register cannot be written and verified.  
		*status = SetRegister(READ_SIZE_OFFSET, size);

		if (*status != 0)
			return S_OK;		// Failure to set and verify register

		// Set the DMA Read count register with the the value specified in the GUI.  Return if the
		// register cannot be written and verified.
		*status = SetRegister(READ_COUNT_OFFSET, count);

		if (*status != 0)
			return S_OK;		// Failure to set and verify register

		// Set the DMA Read pattern register with the value specified in the GUI.  Return if the
		// register cannot be written and verified.
		*status = SetRegister(READ_PATTERN_OFFSET, pattern);

		if (*status != 0)
			return S_OK;		// Failure to set and verify register

		// Fill our Read buffer on the host with the expected pattern.
		for (idx = TLPReadBuffer; idx < TLPReadBuffer + MAX_DMA_TRANSFER; idx++)
			*idx = (unsigned long)pattern;

		Operation = (Operation == write) ? readwrite : read;

		// Set up a contiguous area (System Scatter/Gather) for our user buffer.  This relieves us from the
		// responsibility of gathering up all the different pieces of physical memory which comprise our
		// user buffer in virtual memory, and then programming the DMA engine multiple times to transfer each
		// segment.
		// Note that this does NOT actually kick off the DMA engine - because we have a Bus Master device,
		// that is done later via the StartDMA function.
		asyncRead.Offset = 0;				// Start transfer at the beginning of the file
		asyncRead.OffsetHigh = 0;			// We don't need this, and I don't understand it anyway
		asyncRead.hEvent = 0;				// We don't need a signal event, the device will signal us

		// The DMA device we are ultimately communicating with in this call is capable of simultaneous
		// reads and writes.  On success, this function will return an error code indicating that the
		// requested operation is PENDING.  This allows the application to continue and program another
		// file operation.  This is possible because when we created the device object with CreateFile, 
		// the FILE_FLAG_OVERLAPPED attribute was specified.  That forces us to provide the asyncWrite
		// structure on the file operation call, but has the distinct advantage that it DOES NOT BLOCK
		// as it would if we did not specify overlapped operation.
		if (ReadFile(hs3_1000, TLPReadBuffer, size * count * sizeof(ULONG), &nTransfer, &asyncRead) == 0)
		{
			lastError = GetLastError();

			if (lastError != ERROR_IO_PENDING)
			{
				SetLastError(lastError);
				GetSystemMessage();
				Fire_DisplayMessage(_bstr_t("Cannot perform BMDMA Read System Scatter/Gather.  Press OK for OS info."), MY_NAME);
				Fire_DisplayMessage(_bstr_t((LPCTSTR)SystemMessage), MY_NAME);
				*status = 1;

				return S_OK;
			}
		}
	}

	else
	{
		Fire_DisplayMessage(_bstr_t("Cannot perform DMA Read set up.  PCIe Device Driver has been stopped."), MY_NAME);
		*status = 1;
	}

	return S_OK;
}

STDMETHODIMP Cs3_1000::StartDMA(long *status)
{
	ULONG	nOutput;				// bytes returned from IO ctrl call
	ULONG	regValue;				// value to write to the DCSR register start bits

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	if (hs3_1000 && hs3_1000 != INVALID_HANDLE_VALUE)
	{
		*status = 0;									// Assume success for now
		regValue = (ULONG) Operation;					// Set the appropriate start bits (v.0.4.0.3)

		// Start the DMA transfer.
		if(!DeviceIoControl(hs3_1000, 
							IOCTL_DMA_START, 
							&regValue,
							sizeof(regValue),
							NULL,
							0,
							&nOutput,
							NULL))
		{
			GetSystemMessage();
			Fire_DisplayMessage(_bstr_t("Cannot start DMA engine.  Press OK for OS info."), MY_NAME);
			Fire_DisplayMessage(_bstr_t((LPCTSTR)SystemMessage), MY_NAME);
			Operation = none;							// Reset current operation type (v.0.4.0.3)
			*status = 1;
			
			return S_OK;
		}

	}

	else
	{
		Fire_DisplayMessage(_bstr_t("Cannot start DMA engine.  PCIe Device Driver has been stopped."), MY_NAME);
		*status = 1;
	}

	return S_OK;
}

STDMETHODIMP Cs3_1000::GetDMAStatus(long *status)
{
	OpStatus	DMAStatus = pass;
	ULONG		nOutput;				// bytes returned from IO ctrl call
	ULONG		statusReg;

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	// If PASS, then the GUI will call a new interface method or methods to get the statistics it needs
	// in order to update the interface (cycles, bytes read/written).  
	if (Operation != none)
	{
		if (hs3_1000 && hs3_1000 != INVALID_HANDLE_VALUE)
		{
			// Read the Status register.
			if(!DeviceIoControl(hs3_1000, 
								IOCTL_GET_DMA_STATUS, 
								NULL,
								0,
								&statusReg,
								sizeof(statusReg),
								&nOutput,
								NULL))
			{
				GetSystemMessage();
				Fire_DisplayMessage(_bstr_t("Cannot read DMA status.  Press OK for OS info."), MY_NAME);
				Fire_DisplayMessage(_bstr_t((LPCTSTR)SystemMessage), MY_NAME);
				*status = (long) error;
				
				return S_OK;
			}
			
			if (Operation == read)
			{
				// Check for read completion.
				DMAStatus = (statusReg & 0x1000000) ? pass : running;

				// Check for read error
				if (DMAStatus == pass  && statusReg & 0x80000000)
					DMAStatus = fail;
			}

			else if (Operation == write)
			{
				// Check for write completion.
				DMAStatus = (statusReg & 0x100) ? pass : running;
			}

			else if (Operation == readwrite)
			{
				// Check for read completion.
				DMAStatus = (statusReg & 0x1000000) ? pass : running;

				// Check for write completion.
				if (DMAStatus == pass)
					DMAStatus = (statusReg & 0x100) ? pass : running;

				// Check for read error
				if (DMAStatus == pass  && statusReg & 0x80000000)
					DMAStatus = fail;
			}
			
			// Are we done?
			if (DMAStatus != running) 
			{
				Operation = none;						// Reset current operation type (v.0.4.0.3)
				
				if(!DeviceIoControl(hs3_1000, 
									IOCTL_CLEANUP, 
									NULL,
									0,
									NULL,
									0,
									&nOutput,
									NULL))
				{
					GetSystemMessage();
					Fire_DisplayMessage(_bstr_t("Cleanup failed  Press OK for OS info."), MY_NAME);
					Fire_DisplayMessage(_bstr_t((LPCTSTR)SystemMessage), MY_NAME);
					*status = (long) error;
					
					return S_OK;
				}
			}
		}

		else
		{
			Fire_DisplayMessage(_bstr_t("Cannot read DMA status.  PCIe Device Driver has been stopped."), MY_NAME);
			DMAStatus = error;
		}

	}

	*status = (long) DMAStatus;
	return S_OK;
}

STDMETHODIMP Cs3_1000::GetDMAWritePerf(long *cycles)
{
	ULONG		nOutput;				// bytes returned from IO ctrl call

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	if (hs3_1000 && hs3_1000 != INVALID_HANDLE_VALUE)
	{
		// Read the Status register.
		if(!DeviceIoControl(hs3_1000, 
							IOCTL_GET_DMA_WRITE_PERF, 
							NULL,
							0,
							cycles,
							sizeof(*cycles),
							&nOutput,
							NULL))
		{
			GetSystemMessage();
			Fire_DisplayMessage(_bstr_t("Cannot read DMA Write Performance.  Press OK for OS info."), MY_NAME);
			Fire_DisplayMessage(_bstr_t((LPCTSTR)SystemMessage), MY_NAME);
			
			return S_OK;
		}
	}

	else
	{
		Fire_DisplayMessage(_bstr_t("Cannot read DMA Write Performance.  PCIe Device Driver has been stopped."), MY_NAME);
	}

	return S_OK;
}

STDMETHODIMP Cs3_1000::GetDMAReadPerf(long *cycles)
{
	ULONG		nOutput;				// bytes returned from IO ctrl call

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	if (hs3_1000 && hs3_1000 != INVALID_HANDLE_VALUE)
	{
		// Read the Status register.
		if(!DeviceIoControl(hs3_1000, 
							IOCTL_GET_DMA_READ_PERF, 
							NULL,
							0,
							cycles,
							sizeof(*cycles),
							&nOutput,
							NULL))
		{
			GetSystemMessage();
			Fire_DisplayMessage(_bstr_t("Cannot read DMA Read Performance.  Press OK for OS info."), MY_NAME);
			Fire_DisplayMessage(_bstr_t((LPCTSTR)SystemMessage), MY_NAME);
			
			return S_OK;
		}
	}

	else
	{
		Fire_DisplayMessage(_bstr_t("Cannot read DMA Read Performance.  PCIe Device Driver has been stopped."), MY_NAME);
	}

	return S_OK;
}

STDMETHODIMP Cs3_1000::VerifyDMAWrite(long *status)
{
	unsigned long *	idx;
	unsigned long	cnt;
	char			message[132];
	unsigned long	errCnt = 0;

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	// Verify that our buffer area is filled with the expected pattern.
	for (idx = TLPWriteBuffer, cnt = 0; idx < TLPWriteBuffer + WriteLen; idx++, cnt++)
		if (*idx != WritePattern)
		{
			sprintf(message, "Compare failure at word %d.  Expected 0x%X, read 0x%X", 
				cnt, WritePattern, *idx);
			Fire_DisplayMessage(_bstr_t(message), MY_NAME);
			
			if (++errCnt > 0)
				break;
		}

		*status = (idx >= TLPWriteBuffer + WriteLen) ? 0 : 1;		// 0 == pass, 1 == fail

	return S_OK;
}


STDMETHODIMP Cs3_1000::GetRegister32(long index, long *regValue)
{
	ULONG		nOutput;				// bytes returned from IO ctrl call

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	if (hs3_1000 && hs3_1000 != INVALID_HANDLE_VALUE)
	{
		// Read the Status register.
		if(!DeviceIoControl(hs3_1000, 
							IOCTL_GET_DMA_REGISTER, 
							&index,
							sizeof(index),
							regValue,
							sizeof(*regValue),
							&nOutput,
							NULL))
		{
			GetSystemMessage();
			Fire_DisplayMessage(_bstr_t("Cannot read register, ignore value reported.  Press OK for OS info."), MY_NAME);
			Fire_DisplayMessage(_bstr_t((LPCTSTR)SystemMessage), MY_NAME);
			
			return S_OK;
		}
	}

	else
	{
		Fire_DisplayMessage(_bstr_t("Cannot read register.  PCIe Device Driver has been stopped."), MY_NAME);
	}

	return S_OK;
}

STDMETHODIMP Cs3_1000::GetTLPMaxSize(long *size)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	// RRW - Add code here to read the PCIe configuration data and determine the maximum size
	// in bytes supported for transaction packets.  In a WDM driver, it may be necessary to 
	// use the Plug'n'Play interface, by generating an IRP_QUERY_DEVICE request.  Not sure how
	// to do this at the moment, so this function currently returns a simple hardcoded value.

	*size = TLP_MAX_SIZE;

	return S_OK;
}

STDMETHODIMP Cs3_1000::GetFPGAFamily(long *index)
{
	long		regValue;
	long		regIndex = 0;
	ULONG		nOutput;				// bytes returned from IO ctrl call

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	if (hs3_1000 && hs3_1000 != INVALID_HANDLE_VALUE)
	{
		// Read the Status register.
		if(!DeviceIoControl(hs3_1000, 
							IOCTL_GET_DMA_REGISTER, 
							&regIndex,
							sizeof(regIndex),
							&regValue,
							sizeof(regValue),
							&nOutput,
							NULL))
		{
			GetSystemMessage();
			Fire_DisplayMessage(_bstr_t("Cannot read Device Control register.  Press OK for OS info."), MY_NAME);
			Fire_DisplayMessage(_bstr_t((LPCTSTR)SystemMessage), MY_NAME);
			
			return S_OK;
		}

		regValue >>= 24;		// Family code in bits 24 - 31
		regValue &= 0xFF;

		switch (regValue)
		{
		case	0x11:	// Virtex-II Pro
			*index = 1;
			break;
		case	0x12:	// Virtex-4 FX
			*index = 2;
			break;
		case	0x13:	// Virtex-5 LXT
			*index = 3;
			break;
		case	0x23:	// Virtex-5 SXT
			*index = 4;
			break;
		case	0x33:	// Virtex-5 FXT
			*index = 5;
			break;
		case	0x18:	// Spartan-3
			*index = 6;
			break;
		case	0x28:	// Spartan-3E
			*index = 7;
			break;
		case	0x38:	// Spartan-3A
			*index = 8;
			break;
		default:		// Invalid
			*index = 0;
			break;
		}

	}

	else
	{
		Fire_DisplayMessage(_bstr_t("Cannot read register.  PCIe Device Driver has been stopped."), MY_NAME);
	}

	return S_OK;
}

// Return the cycle time in tens of nanoseconds, based on the following table.  We multiply
// by ten to avoid passing decimal values.
// Negotiated Link Width		ns Cycle Time (32 bit)		ns Cycle Time (64 bit)
//			x1							16							32 (16 for Virtex 5)
//			x2							 8							16
//			x4							 4							 8
//			x8							 2							 4
//			x12							 1.5						 3
//			x16							 1							 2
//			x32							 .5							 1
STDMETHODIMP Cs3_1000::GetCycleTime(long *cycleTime)
{
	long		index;
	long		regValue;
	long		width;

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	if (hs3_1000 && hs3_1000 != INVALID_HANDLE_VALUE)
	{
		// Get the core data path width from the Device Control Register.
		GetRegister32(DCR_OFFSET/4, &regValue);
		regValue &= 0x70000;
		regValue >>= 16;
		width = (regValue == 1) ? 32 : 64;

		// Extract the negotiated link width from the Device link width status register.
		GetRegister32(DEVICE_LINK_WIDTH_STATUS/4, &regValue);
		regValue &= 0x3F00;
		regValue >>= 8;

		// Remember, these ns frequency values set in this switch table are 10x the real
		// values to eliminate the need for float values.
		switch (regValue)
		{
		case 0x1:	// x1
			GetFPGAFamily(&index);

			if (index == 3 || index == 4 || index == 5)			// Virtex-5 devices
				*cycleTime = 160;
			else
				*cycleTime = (width == 32) ? 160 : 320;

			break;

		case 0x2:	// x2
			*cycleTime = (width == 32) ? 80 : 160;
			break;

		case 0x4:	// x4
			if (index == 3 || index == 4 || index == 5)			// Virtex-5 devices
				*cycleTime = 80;
			else
				*cycleTime = (width == 32) ? 40 : 80;
			break;

		case 0x8:	// x8
			if (index == 3 || index == 4 || index == 5)			// Virtex-5 devices
				*cycleTime = 40;
			else
				*cycleTime = (width == 32) ? 20 : 40;
			break;

		case 0xC:	// x12
			*cycleTime = (width == 32) ? 15 : 30;
			break;

		case 0x10:	// x16
			*cycleTime = (width == 32) ? 10 : 20;
			break;

		case 0x20:	// x32
			*cycleTime = (width == 32) ? 5 : 10;
			break;

		default:
			Fire_DisplayMessage(_bstr_t("Illegal Link Width, using cycle time of 32 ns"), MY_NAME);
			*cycleTime = 320;
			break;
		}
	}

	else
	{
		Fire_DisplayMessage(_bstr_t("Cannot read register.  PCIe Device Driver has been stopped."), MY_NAME);
	}

	return S_OK;
}

STDMETHODIMP Cs3_1000::SetInterruptState(long state)
{
	ULONG	nOutput;				// bytes returned from IO ctrl call
	ULONG	controlCode;			// Disable/Enable interrupts control code

	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	if (hs3_1000 && hs3_1000 != INVALID_HANDLE_VALUE)
	{
		// Select the control code corresponding to the state: 0 = turn ints off, 1 = turn ints on
		controlCode = (state == 0) ? IOCTL_DISABLE_INTERRUPTS : IOCTL_ENABLE_INTERRUPTS;

		if(!DeviceIoControl(hs3_1000, 
							controlCode, 
							NULL,
							0,
							NULL,
							0,
							&nOutput,
							NULL))
		{
			GetSystemMessage();
			Fire_DisplayMessage(_bstr_t((LPCTSTR)SystemMessage), MY_NAME);
		}
	}

	else
	{
		Fire_DisplayMessage(
		  _bstr_t("Interrupt State cannot be set when the Device Driver is not running."), MY_NAME);
	}

	return S_OK;
}
