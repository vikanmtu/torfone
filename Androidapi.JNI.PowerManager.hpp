// CodeGear C++Builder
// Copyright (c) 1995, 2016 by Embarcadero Technologies, Inc.
// All rights reserved

// (DO NOT EDIT: machine generated header) 'Androidapi.JNI.PowerManager.pas' rev: 32.00 (Android)

#ifndef Androidapi_Jni_PowermanagerHPP
#define Androidapi_Jni_PowermanagerHPP

#pragma delphiheader begin
#pragma option push
#pragma option -w-      // All warnings off
#pragma option -Vx      // Zero-length empty class member 
#pragma pack(push,8)
#include <System.hpp>
#include <SysInit.hpp>
#include <System.SysUtils.hpp>
#include <Androidapi.JNIBridge.hpp>
#include <Androidapi.JNI.GraphicsContentViewText.hpp>
#include <Androidapi.JNI.JavaTypes.hpp>
#include <Androidapi.Helpers.hpp>
#include <FMX.Helpers.Android.hpp>

//-- user supplied -----------------------------------------------------------

namespace Androidapi
{
namespace Jni
{
namespace Powermanager
{
//-- forward type declarations -----------------------------------------------
__interface JPowerManager_WakeLockClass;
typedef System::DelphiInterface<JPowerManager_WakeLockClass> _di_JPowerManager_WakeLockClass;
__interface JWakeLock;
typedef System::DelphiInterface<JWakeLock> _di_JWakeLock;
class DELPHICLASS TJWakeLock;
__interface JPowerManagerClass;
typedef System::DelphiInterface<JPowerManagerClass> _di_JPowerManagerClass;
__interface JPowerManager;
typedef System::DelphiInterface<JPowerManager> _di_JPowerManager;
class DELPHICLASS TJPowerManager;
//-- type declarations -------------------------------------------------------
__interface  INTERFACE_UUID("{DA204013-460E-4CE5-B77E-772870E53853}") JPowerManager_WakeLockClass  : public Androidapi::Jni::Javatypes::JObjectClass 
{
	
};

__interface  INTERFACE_UUID("{944B58EB-1BDA-403B-AF6F-D37E07CFE914}") JWakeLock  : public Androidapi::Jni::Javatypes::JObject 
{
	virtual void __cdecl setReferenceCounted(bool referenceCount) = 0 ;
	virtual void __cdecl acquire(void) = 0 /* overload */;
	virtual void __cdecl acquire(__int64 timeout) = 0 /* overload */;
	virtual void __cdecl release(void) = 0 ;
	virtual bool __cdecl isHeld(void) = 0 ;
	HIDESBASE virtual Androidapi::Jni::Javatypes::_di_JString __cdecl toString(void) = 0 ;
};

#pragma pack(push,4)
class PASCALIMPLEMENTATION TJWakeLock : public Androidapi::Jnibridge::TJavaGenericImport__2<_di_JPowerManager_WakeLockClass,_di_JWakeLock> 
{
	typedef Androidapi::Jnibridge::TJavaGenericImport__2<_di_JPowerManager_WakeLockClass,_di_JWakeLock>  inherited;
	
public:
	/* TObject.Create */ inline __fastcall TJWakeLock(void) : Androidapi::Jnibridge::TJavaGenericImport__2<_di_JPowerManager_WakeLockClass,_di_JWakeLock> () { }
	/* TObject.Destroy */ inline __fastcall virtual ~TJWakeLock(void) { }
	
};

#pragma pack(pop)

__interface  INTERFACE_UUID("{E4AC8BDC-18D1-42AA-84A3-A1ACEAAEDAEF}") JPowerManagerClass  : public Androidapi::Jni::Javatypes::JObjectClass 
{
	virtual int __fastcall _GetPARTIAL_WAKE_LOCK(void) = 0 ;
	virtual int __fastcall _GetSCREEN_DIM_WAKE_LOCK(void) = 0 ;
	virtual int __fastcall _GetSCREEN_BRIGHT_WAKE_LOCK(void) = 0 ;
	virtual int __fastcall _GetFULL_WAKE_LOCK(void) = 0 ;
	virtual int __fastcall _GetACQUIRE_CAUSES_WAKEUP(void) = 0 ;
	virtual int __fastcall _GetON_AFTER_RELEASE(void) = 0 ;
	__property int PARTIAL_WAKE_LOCK = {read=_GetPARTIAL_WAKE_LOCK};
	__property int SCREEN_DIM_WAKE_LOCK = {read=_GetSCREEN_DIM_WAKE_LOCK};
	__property int SCREEN_BRIGHT_WAKE_LOCK = {read=_GetSCREEN_BRIGHT_WAKE_LOCK};
	__property int FULL_WAKE_LOCK = {read=_GetFULL_WAKE_LOCK};
	__property int ACQUIRE_CAUSES_WAKEUP = {read=_GetACQUIRE_CAUSES_WAKEUP};
	__property int ON_AFTER_RELEASE = {read=_GetON_AFTER_RELEASE};
};

__interface  INTERFACE_UUID("{DEAED658-4353-4D17-B0A3-8179E48BE87F}") JPowerManager  : public Androidapi::Jni::Javatypes::JObject 
{
	virtual _di_JWakeLock __cdecl newWakeLock(int levelAndFlags, Androidapi::Jni::Javatypes::_di_JString tag) = 0 ;
	virtual void __cdecl userActivity(__int64 when, bool noChangeLights) = 0 ;
	virtual void __cdecl goToSleep(__int64 time) = 0 ;
	virtual void __cdecl wakeUp(__int64 Long) = 0 ;
	virtual bool __cdecl isScreenOn(void) = 0 ;
	virtual void __cdecl reboot(Androidapi::Jni::Javatypes::_di_JString reason) = 0 ;
};

#pragma pack(push,4)
class PASCALIMPLEMENTATION TJPowerManager : public Androidapi::Jnibridge::TJavaGenericImport__2<_di_JPowerManagerClass,_di_JPowerManager> 
{
	typedef Androidapi::Jnibridge::TJavaGenericImport__2<_di_JPowerManagerClass,_di_JPowerManager>  inherited;
	
public:
	/* TObject.Create */ inline __fastcall TJPowerManager(void) : Androidapi::Jnibridge::TJavaGenericImport__2<_di_JPowerManagerClass,_di_JPowerManager> () { }
	/* TObject.Destroy */ inline __fastcall virtual ~TJPowerManager(void) { }
	
};

#pragma pack(pop)

//-- var, const, procedure ---------------------------------------------------
extern DELPHI_PACKAGE _di_JPowerManager __fastcall GetPowerManager(void);
extern DELPHI_PACKAGE bool __fastcall AcquireWakeLock(void);
extern DELPHI_PACKAGE bool __fastcall AcquireWakeLock1(void);
extern DELPHI_PACKAGE void __fastcall ReleaseWakeLock(void);
}	/* namespace Powermanager */
}	/* namespace Jni */
}	/* namespace Androidapi */
#if !defined(DELPHIHEADER_NO_IMPLICIT_NAMESPACE_USE) && !defined(NO_USING_NAMESPACE_ANDROIDAPI_JNI_POWERMANAGER)
using namespace Androidapi::Jni::Powermanager;
#endif
#if !defined(DELPHIHEADER_NO_IMPLICIT_NAMESPACE_USE) && !defined(NO_USING_NAMESPACE_ANDROIDAPI_JNI)
using namespace Androidapi::Jni;
#endif
#if !defined(DELPHIHEADER_NO_IMPLICIT_NAMESPACE_USE) && !defined(NO_USING_NAMESPACE_ANDROIDAPI)
using namespace Androidapi;
#endif
#pragma pack(pop)
#pragma option pop

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// Androidapi_Jni_PowermanagerHPP
