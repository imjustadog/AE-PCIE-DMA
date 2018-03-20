
DriverMgrps.dll: dlldata.obj DriverMgr_p.obj DriverMgr_i.obj
	link /dll /out:DriverMgrps.dll /def:DriverMgrps.def /entry:DllMain dlldata.obj DriverMgr_p.obj DriverMgr_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del DriverMgrps.dll
	@del DriverMgrps.lib
	@del DriverMgrps.exp
	@del dlldata.obj
	@del DriverMgr_p.obj
	@del DriverMgr_i.obj
