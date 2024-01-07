// CodeGear C++Builder
// Copyright (c) 1995, 2016 by Embarcadero Technologies, Inc.
// All rights reserved

// (DO NOT EDIT: machine generated header) 'NotificationServiceUnit.pas' rev: 32.00 (Android)

#ifndef NotificationserviceunitHPP
#define NotificationserviceunitHPP

#pragma delphiheader begin
#pragma option push
#pragma option -w-      // All warnings off
#pragma option -Vx      // Zero-length empty class member 
#pragma pack(push,8)
#include <System.hpp>
#include <SysInit.hpp>
#include <Posix.Stdlib.hpp>
#include <System.IOUtils.hpp>
#include <System.SysUtils.hpp>
#include <System.Classes.hpp>
#include <System.Android.Service.hpp>
#include <System.Threading.hpp>
#include <Androidapi.Helpers.hpp>
#include <Androidapi.JNI.GraphicsContentViewText.hpp>
#include <System.Android.Notification.hpp>
#include <Androidapi.JNI.Support.hpp>
#include <Androidapi.JNI.Os.hpp>
#include <System.Notification.hpp>
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdUDPBase.hpp>
#include <IdUDPServer.hpp>
#include <IdGlobal.hpp>
#include <IdSocketHandle.hpp>
#include <IdUDPClient.hpp>

//-- user supplied -----------------------------------------------------------

namespace Notificationserviceunit
{
//-- forward type declarations -----------------------------------------------
class DELPHICLASS TNotificationServiceDM;
//-- type declarations -------------------------------------------------------
#pragma pack(push,4)
class PASCALIMPLEMENTATION TNotificationServiceDM : public System::Android::Service::TAndroidService
{
	typedef System::Android::Service::TAndroidService inherited;
	
__published:
	int __fastcall AndroidServiceStartCommand(System::TObject* const Sender, const Androidapi::Jni::Graphicscontentviewtext::_di_JIntent Intent, int Flags, int StartId);
	void __fastcall AndroidServiceCreate(System::TObject* Sender);
	
private:
	System::Classes::TThread* FThread;
	void __fastcall Thinfinito(void);
	void __fastcall LaunchMain(void);
public:
	/* TDataModule.Create */ inline __fastcall virtual TNotificationServiceDM(System::Classes::TComponent* AOwner) : System::Android::Service::TAndroidService(AOwner) { }
	/* TDataModule.CreateNew */ inline __fastcall virtual TNotificationServiceDM(System::Classes::TComponent* AOwner, int Dummy) : System::Android::Service::TAndroidService(AOwner, Dummy) { }
	/* TDataModule.Destroy */ inline __fastcall virtual ~TNotificationServiceDM(void) { }
	
};

#pragma pack(pop)

//-- var, const, procedure ---------------------------------------------------
extern DELPHI_PACKAGE TNotificationServiceDM* NotificationServiceDM;
extern DELPHI_PACKAGE int flg;
extern DELPHI_PACKAGE __int64 last;
}	/* namespace Notificationserviceunit */
#if !defined(DELPHIHEADER_NO_IMPLICIT_NAMESPACE_USE) && !defined(NO_USING_NAMESPACE_NOTIFICATIONSERVICEUNIT)
using namespace Notificationserviceunit;
#endif
#pragma pack(pop)
#pragma option pop

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// NotificationserviceunitHPP
