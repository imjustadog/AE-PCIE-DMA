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
// DriverMgr.cpp : Implementation of DLL Exports.


// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL, 
//      run nmake -f DriverMgrps.mk in the project directory.

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "DriverMgr.h"

#include "DriverMgr_i.c"
#include "s3_1000.h"


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_s3_1000, Cs3_1000)
END_OBJECT_MAP()

class CDriverMgrApp : public CWinApp
{
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDriverMgrApp)
	public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CDriverMgrApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CDriverMgrApp, CWinApp)
	//{{AFX_MSG_MAP(CDriverMgrApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CDriverMgrApp theApp;

BOOL CDriverMgrApp::InitInstance()
{
    _Module.Init(ObjectMap, m_hInstance, &LIBID_DRIVERMGRLib);
    return CWinApp::InitInstance();
}

int CDriverMgrApp::ExitInstance()
{
    _Module.Term();
    return CWinApp::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return (AfxDllCanUnloadNow()==S_OK && _Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
    // registers object, typelib and all interfaces in typelib
    return _Module.RegisterServer(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}


