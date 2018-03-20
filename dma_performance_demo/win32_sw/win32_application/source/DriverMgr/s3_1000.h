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
	
// s3_1000.h : Declaration of the Cs3_1000

#ifndef __S3_1000_H_
#define __S3_1000_H_

#include "resource.h"						// main symbols
#include "DriverMgrCP.h"
#include <setupapi.h>						// WDM declarations for PnP

#define TLP_MAX_SIZE		32				// RRW - workaround until this value can be determined dynamically
#define MAX_DMA_TRANSFER	32768 / 4		// Maximum DMA transfer size in Words

/////////////////////////////////////////////////////////////////////////////
// Cs3_1000
class ATL_NO_VTABLE Cs3_1000 : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<Cs3_1000, &CLSID_s3_1000>,
	public IConnectionPointContainerImpl<Cs3_1000>,
	public IDispatchImpl<Is3_1000, &IID_Is3_1000, &LIBID_DRIVERMGRLib>,
	public CProxy_Is3_1000Events< Cs3_1000 >
{
public:
	Cs3_1000()
	{
		unsigned long *pBuf;

		m_pUnkMarshaler = NULL;
		hs3_1000 = INVALID_HANDLE_VALUE;
		Operation = none;
		WritePattern = 0xDEADBEEF;
		InitializeRegInfo();

		// Calculate 4K aligned starting point within the read/write buffer areas
		// to use as our data starting point.  The DMA hardware requires that the
		// logical address of our map buffers be aligned on a 4K byte boundary, so
		// this is a compiler independent way of ensuring that.  Note that the 
		// HAL will ensure that when it allocates map buffers for System Scatter/Gather,
		// the alignment of these buffers will be the same as the user buffers
		// containing the original data (otherwise, we would have no way to force a 
		// particular alignment in the system-allocated map buffers).
		//
		// Note that the buffer areas were created as the maximum size required plus 4K bytes,
		// which is wasteful of memory but ensures that we will have an area 4K aligned within
		// each buffer area sufficiently large to contain the maximum amount of data.
		pBuf = TLPReadBufferArea;

		while ((unsigned long)pBuf % 0x1000 != 0)
			pBuf++;

		TLPReadBuffer = pBuf;

		pBuf = TLPWriteBufferArea;

		while ((unsigned long)pBuf % 0x1000 != 0)
			pBuf++;

		TLPWriteBuffer = pBuf;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_S3_1000)
DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(Cs3_1000)
	COM_INTERFACE_ENTRY(Is3_1000)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()
BEGIN_CONNECTION_POINT_MAP(Cs3_1000)
CONNECTION_POINT_ENTRY(DIID__Is3_1000Events)
END_CONNECTION_POINT_MAP()


	HRESULT FinalConstruct()
	{
		return CoCreateFreeThreadedMarshaler(
			GetControllingUnknown(), &m_pUnkMarshaler.p);
	}

	void FinalRelease()
	{
		m_pUnkMarshaler.Release();
		hs3_1000 = INVALID_HANDLE_VALUE;
	}

	CComPtr<IUnknown> m_pUnkMarshaler;

// Is3_1000
public:
	STDMETHOD(SetInterruptState)(/*[in]*/ long state);
	STDMETHOD(GetCycleTime)(/*[out, retval]*/ long *cycleTime);
	STDMETHOD(GetFPGAFamily)(/*[out, retval]*/ long *index);
	STDMETHOD(GetTLPMaxSize)(/*[out, retval]*/ long *size);
	STDMETHOD(GetRegister32)(/*[in]*/ long index, /*[out, retval]*/ long *regValue);
	STDMETHOD(VerifyDMAWrite)(/*[out, retval]*/ long *status);
	STDMETHOD(GetDMAReadPerf)(/*[out, retval]*/ long *cycles);
	STDMETHOD(GetDMAWritePerf)(/*[out, retval]*/ long *cycles);
	STDMETHOD(GetDMAStatus)(/*[out, retval]*/ long *status);
	STDMETHOD(StartDMA)(/*[out. retval]*/ long *status);
	STDMETHOD(SetDMARead)(/*[in]*/ long size, /*[in]*/ long count, /*[in]*/ long pattern, /*[out, retval]*/ long *status);
	STDMETHOD(SetDMAWrite)(/*[in]*/ long size, /*[in]*/ long count, /*[in]*/ long pattern, /*[out, retval]*/ long *status);
	STDMETHOD(Reset)(/*[out, retval]*/ long *status);
	STDMETHOD(GetPBState)(/*[out, retval]*/ long *state);
	STDMETHOD(GetDIPState)(/*[out, retval]*/ long *state);
	STDMETHOD(GetLEDState)(/*[in]*/ long index,/*[out, retval]*/ long *on);
	STDMETHOD(SetLEDState)(/*[in]*/ long displayID, /*[in]*/ long on);
	STDMETHOD(CloseDevice)();
	STDMETHOD(OpenDevice)(/*[out, retval]*/ long *status);

private:
	// Allocate user buffer space large enough to accomodate a 4K aligned starting point within the field.
	unsigned long	TLPReadBufferArea[MAX_DMA_TRANSFER + 4096];		// Buffer space for maximum DMA Read size
	unsigned long  *TLPReadBuffer;									// 4K aligned starting point (see constructor)
	unsigned long	TLPWriteBufferArea[MAX_DMA_TRANSFER + 4096];	// Buffer space for maximum DMA Write size
	unsigned long  *TLPWriteBuffer;									// 4K aligned starting point (see constructor)

	typedef struct 
	{
		long	offset;
		long	writeControlCode;
		long	readControlCode;
		char  *	name;
	} RegisterInfo;

	void			GetSystemMessage();
	void			InitializeRegInfo();
	HANDLE			OpenDeviceByGuid(LPGUID pGuid);
	long			SetRegister(unsigned long offset, unsigned long value);

	HANDLE			hs3_1000;

	char			SystemMessage[256];							// Buffer for System generated messages
	unsigned long	WritePattern;
	unsigned long	WriteLen;

	enum DMAType {none = 0, read = 0x10000, write = 0x1, readwrite = 0x10001} Operation;
	
	RegisterInfo	RegInfo[11];

};

#endif //__S3_1000_H_
