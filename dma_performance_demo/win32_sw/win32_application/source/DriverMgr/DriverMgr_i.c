/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 5.01.0164 */
/* at Mon Sep 11 14:21:09 2006
 */
/* Compiler settings for C:\Projects_2006\PCIe_Perf\3s1000\DriverMgr\DriverMgr.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

const IID IID_Is3_1000 = {0x3E75B3E2,0x14E8,0x46f7,{0x9E,0x64,0x2A,0xE0,0x1A,0x61,0xA6,0x51}};


const IID LIBID_DRIVERMGRLib = {0xFE9E3996,0x959F,0x4054,{0xB3,0xCE,0xA0,0xED,0xA2,0xAA,0x0C,0x31}};


const IID DIID__Is3_1000Events = {0x59C3DB58,0xC9B2,0x4a94,{0xA4,0x9E,0x6A,0x00,0xDE,0x5E,0xF6,0x00}};


const CLSID CLSID_s3_1000 = {0x5B145E4D,0x5D9E,0x45f6,{0xB5,0x80,0xAD,0xBD,0x48,0x38,0x01,0x18}};


#ifdef __cplusplus
}
#endif

