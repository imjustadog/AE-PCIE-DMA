/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Mon Sep 11 14:21:09 2006
 */
/* Compiler settings for C:\Projects_2006\PCIe_Perf\3s1000\DriverMgr\DriverMgr.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __DriverMgr_h__
#define __DriverMgr_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __Is3_1000_FWD_DEFINED__
#define __Is3_1000_FWD_DEFINED__
typedef interface Is3_1000 Is3_1000;
#endif 	/* __Is3_1000_FWD_DEFINED__ */


#ifndef ___Is3_1000Events_FWD_DEFINED__
#define ___Is3_1000Events_FWD_DEFINED__
typedef interface _Is3_1000Events _Is3_1000Events;
#endif 	/* ___Is3_1000Events_FWD_DEFINED__ */


#ifndef __s3_1000_FWD_DEFINED__
#define __s3_1000_FWD_DEFINED__

#ifdef __cplusplus
typedef class s3_1000 s3_1000;
#else
typedef struct s3_1000 s3_1000;
#endif /* __cplusplus */

#endif 	/* __s3_1000_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __Is3_1000_INTERFACE_DEFINED__
#define __Is3_1000_INTERFACE_DEFINED__

/* interface Is3_1000 */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_Is3_1000;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3E75B3E2-14E8-46f7-9E64-2AE01A61A651")
    Is3_1000 : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OpenDevice( 
            /* [retval][out] */ long __RPC_FAR *status) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CloseDevice( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetLEDState( 
            /* [in] */ long index,
            /* [in] */ long on) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetLEDState( 
            /* [in] */ long index,
            /* [retval][out] */ long __RPC_FAR *state) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetDIPState( 
            /* [retval][out] */ long __RPC_FAR *state) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetPBState( 
            /* [retval][out] */ long __RPC_FAR *state) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Reset( 
            /* [retval][out] */ long __RPC_FAR *status) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetDMAWrite( 
            /* [in] */ long size,
            /* [in] */ long count,
            /* [in] */ long pattern,
            /* [retval][out] */ long __RPC_FAR *status) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetDMARead( 
            /* [in] */ long size,
            /* [in] */ long count,
            /* [in] */ long pattern,
            /* [retval][out] */ long __RPC_FAR *status) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE StartDMA( 
            /* [retval][out] */ long __RPC_FAR *status) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetDMAStatus( 
            /* [retval][out] */ long __RPC_FAR *status) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetDMAWritePerf( 
            /* [retval][out] */ long __RPC_FAR *cycles) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetDMAReadPerf( 
            /* [retval][out] */ long __RPC_FAR *cycles) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE VerifyDMAWrite( 
            /* [retval][out] */ long __RPC_FAR *status) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetRegister32( 
            /* [in] */ long index,
            /* [retval][out] */ long __RPC_FAR *regValue) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetTLPMaxSize( 
            /* [retval][out] */ long __RPC_FAR *size) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetFPGAFamily( 
            /* [retval][out] */ long __RPC_FAR *index) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetCycleTime( 
            /* [retval][out] */ long __RPC_FAR *cycleTime) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetInterruptState( 
            /* [in] */ long state) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct Is3_1000Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            Is3_1000 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            Is3_1000 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            Is3_1000 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            Is3_1000 __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            Is3_1000 __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            Is3_1000 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            Is3_1000 __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenDevice )( 
            Is3_1000 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *status);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CloseDevice )( 
            Is3_1000 __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetLEDState )( 
            Is3_1000 __RPC_FAR * This,
            /* [in] */ long index,
            /* [in] */ long on);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLEDState )( 
            Is3_1000 __RPC_FAR * This,
            /* [in] */ long index,
            /* [retval][out] */ long __RPC_FAR *state);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDIPState )( 
            Is3_1000 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *state);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPBState )( 
            Is3_1000 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *state);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            Is3_1000 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *status);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDMAWrite )( 
            Is3_1000 __RPC_FAR * This,
            /* [in] */ long size,
            /* [in] */ long count,
            /* [in] */ long pattern,
            /* [retval][out] */ long __RPC_FAR *status);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDMARead )( 
            Is3_1000 __RPC_FAR * This,
            /* [in] */ long size,
            /* [in] */ long count,
            /* [in] */ long pattern,
            /* [retval][out] */ long __RPC_FAR *status);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StartDMA )( 
            Is3_1000 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *status);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDMAStatus )( 
            Is3_1000 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *status);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDMAWritePerf )( 
            Is3_1000 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *cycles);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDMAReadPerf )( 
            Is3_1000 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *cycles);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *VerifyDMAWrite )( 
            Is3_1000 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *status);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRegister32 )( 
            Is3_1000 __RPC_FAR * This,
            /* [in] */ long index,
            /* [retval][out] */ long __RPC_FAR *regValue);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTLPMaxSize )( 
            Is3_1000 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *size);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetFPGAFamily )( 
            Is3_1000 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *index);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCycleTime )( 
            Is3_1000 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *cycleTime);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetInterruptState )( 
            Is3_1000 __RPC_FAR * This,
            /* [in] */ long state);
        
        END_INTERFACE
    } Is3_1000Vtbl;

    interface Is3_1000
    {
        CONST_VTBL struct Is3_1000Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Is3_1000_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Is3_1000_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Is3_1000_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Is3_1000_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Is3_1000_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Is3_1000_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Is3_1000_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Is3_1000_OpenDevice(This,status)	\
    (This)->lpVtbl -> OpenDevice(This,status)

#define Is3_1000_CloseDevice(This)	\
    (This)->lpVtbl -> CloseDevice(This)

#define Is3_1000_SetLEDState(This,index,on)	\
    (This)->lpVtbl -> SetLEDState(This,index,on)

#define Is3_1000_GetLEDState(This,index,state)	\
    (This)->lpVtbl -> GetLEDState(This,index,state)

#define Is3_1000_GetDIPState(This,state)	\
    (This)->lpVtbl -> GetDIPState(This,state)

#define Is3_1000_GetPBState(This,state)	\
    (This)->lpVtbl -> GetPBState(This,state)

#define Is3_1000_Reset(This,status)	\
    (This)->lpVtbl -> Reset(This,status)

#define Is3_1000_SetDMAWrite(This,size,count,pattern,status)	\
    (This)->lpVtbl -> SetDMAWrite(This,size,count,pattern,status)

#define Is3_1000_SetDMARead(This,size,count,pattern,status)	\
    (This)->lpVtbl -> SetDMARead(This,size,count,pattern,status)

#define Is3_1000_StartDMA(This,status)	\
    (This)->lpVtbl -> StartDMA(This,status)

#define Is3_1000_GetDMAStatus(This,status)	\
    (This)->lpVtbl -> GetDMAStatus(This,status)

#define Is3_1000_GetDMAWritePerf(This,cycles)	\
    (This)->lpVtbl -> GetDMAWritePerf(This,cycles)

#define Is3_1000_GetDMAReadPerf(This,cycles)	\
    (This)->lpVtbl -> GetDMAReadPerf(This,cycles)

#define Is3_1000_VerifyDMAWrite(This,status)	\
    (This)->lpVtbl -> VerifyDMAWrite(This,status)

#define Is3_1000_GetRegister32(This,index,regValue)	\
    (This)->lpVtbl -> GetRegister32(This,index,regValue)

#define Is3_1000_GetTLPMaxSize(This,size)	\
    (This)->lpVtbl -> GetTLPMaxSize(This,size)

#define Is3_1000_GetFPGAFamily(This,index)	\
    (This)->lpVtbl -> GetFPGAFamily(This,index)

#define Is3_1000_GetCycleTime(This,cycleTime)	\
    (This)->lpVtbl -> GetCycleTime(This,cycleTime)

#define Is3_1000_SetInterruptState(This,state)	\
    (This)->lpVtbl -> SetInterruptState(This,state)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Is3_1000_OpenDevice_Proxy( 
    Is3_1000 __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *status);


void __RPC_STUB Is3_1000_OpenDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Is3_1000_CloseDevice_Proxy( 
    Is3_1000 __RPC_FAR * This);


void __RPC_STUB Is3_1000_CloseDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Is3_1000_SetLEDState_Proxy( 
    Is3_1000 __RPC_FAR * This,
    /* [in] */ long index,
    /* [in] */ long on);


void __RPC_STUB Is3_1000_SetLEDState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Is3_1000_GetLEDState_Proxy( 
    Is3_1000 __RPC_FAR * This,
    /* [in] */ long index,
    /* [retval][out] */ long __RPC_FAR *state);


void __RPC_STUB Is3_1000_GetLEDState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Is3_1000_GetDIPState_Proxy( 
    Is3_1000 __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *state);


void __RPC_STUB Is3_1000_GetDIPState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Is3_1000_GetPBState_Proxy( 
    Is3_1000 __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *state);


void __RPC_STUB Is3_1000_GetPBState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Is3_1000_Reset_Proxy( 
    Is3_1000 __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *status);


void __RPC_STUB Is3_1000_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Is3_1000_SetDMAWrite_Proxy( 
    Is3_1000 __RPC_FAR * This,
    /* [in] */ long size,
    /* [in] */ long count,
    /* [in] */ long pattern,
    /* [retval][out] */ long __RPC_FAR *status);


void __RPC_STUB Is3_1000_SetDMAWrite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Is3_1000_SetDMARead_Proxy( 
    Is3_1000 __RPC_FAR * This,
    /* [in] */ long size,
    /* [in] */ long count,
    /* [in] */ long pattern,
    /* [retval][out] */ long __RPC_FAR *status);


void __RPC_STUB Is3_1000_SetDMARead_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Is3_1000_StartDMA_Proxy( 
    Is3_1000 __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *status);


void __RPC_STUB Is3_1000_StartDMA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Is3_1000_GetDMAStatus_Proxy( 
    Is3_1000 __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *status);


void __RPC_STUB Is3_1000_GetDMAStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Is3_1000_GetDMAWritePerf_Proxy( 
    Is3_1000 __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *cycles);


void __RPC_STUB Is3_1000_GetDMAWritePerf_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Is3_1000_GetDMAReadPerf_Proxy( 
    Is3_1000 __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *cycles);


void __RPC_STUB Is3_1000_GetDMAReadPerf_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Is3_1000_VerifyDMAWrite_Proxy( 
    Is3_1000 __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *status);


void __RPC_STUB Is3_1000_VerifyDMAWrite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Is3_1000_GetRegister32_Proxy( 
    Is3_1000 __RPC_FAR * This,
    /* [in] */ long index,
    /* [retval][out] */ long __RPC_FAR *regValue);


void __RPC_STUB Is3_1000_GetRegister32_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Is3_1000_GetTLPMaxSize_Proxy( 
    Is3_1000 __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *size);


void __RPC_STUB Is3_1000_GetTLPMaxSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Is3_1000_GetFPGAFamily_Proxy( 
    Is3_1000 __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *index);


void __RPC_STUB Is3_1000_GetFPGAFamily_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Is3_1000_GetCycleTime_Proxy( 
    Is3_1000 __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *cycleTime);


void __RPC_STUB Is3_1000_GetCycleTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Is3_1000_SetInterruptState_Proxy( 
    Is3_1000 __RPC_FAR * This,
    /* [in] */ long state);


void __RPC_STUB Is3_1000_SetInterruptState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __Is3_1000_INTERFACE_DEFINED__ */



#ifndef __DRIVERMGRLib_LIBRARY_DEFINED__
#define __DRIVERMGRLib_LIBRARY_DEFINED__

/* library DRIVERMGRLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_DRIVERMGRLib;

#ifndef ___Is3_1000Events_DISPINTERFACE_DEFINED__
#define ___Is3_1000Events_DISPINTERFACE_DEFINED__

/* dispinterface _Is3_1000Events */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID__Is3_1000Events;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("59C3DB58-C9B2-4a94-A49E-6A00DE5EF600")
    _Is3_1000Events : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _Is3_1000EventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            _Is3_1000Events __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            _Is3_1000Events __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            _Is3_1000Events __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            _Is3_1000Events __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            _Is3_1000Events __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            _Is3_1000Events __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            _Is3_1000Events __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } _Is3_1000EventsVtbl;

    interface _Is3_1000Events
    {
        CONST_VTBL struct _Is3_1000EventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _Is3_1000Events_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _Is3_1000Events_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _Is3_1000Events_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _Is3_1000Events_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _Is3_1000Events_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _Is3_1000Events_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _Is3_1000Events_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___Is3_1000Events_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_s3_1000;

#ifdef __cplusplus

class DECLSPEC_UUID("5B145E4D-5D9E-45f6-B580-ADBD48380118")
s3_1000;
#endif
#endif /* __DRIVERMGRLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
