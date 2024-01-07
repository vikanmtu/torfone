//---------------------------------------------------------------------------
// Copyright (c) 2016 Embarcadero Technologies, Inc. All rights reserved.  
//
// This software is the copyrighted property of Embarcadero Technologies, Inc. 
// ("Embarcadero") and its licensors. You may only use this software if you 
// are an authorized licensee of Delphi, C++Builder or RAD Studio 
// (the "Embarcadero Products").  This software is subject to Embarcadero's 
// standard software license and support agreement that accompanied your 
// purchase of the Embarcadero Products and is considered a Redistributable, 
// as such term is defined thereunder. Your use of this software constitutes 
// your acknowledgement of your agreement to the foregoing software license 
// and support agreement. 
//---------------------------------------------------------------------------

//KeepAlive Android Service for restart App after killing by system
//Main activity will be reastarted on Service start
//or bing to front if already started
//Service will be reastarted automatically if killed by system

unit NotificationServiceUnit;

interface

uses
  Posix.Stdlib,
  System.IOUtils,
  System.SysUtils,
  System.Classes,
  System.Android.Service,
  System.Threading,
  Androidapi.Helpers,
  AndroidApi.JNI.GraphicsContentViewText,
  System.Android.Notification, AndroidApi.JNI.Support,
  Androidapi.JNI.Os, System.Notification, IdBaseComponent, IdComponent,
  IdUDPBase, IdUDPServer, IdGlobal, IdSocketHandle, IdUDPClient;

type
  TNotificationServiceDM = class(TAndroidService)
    function AndroidServiceStartCommand(const Sender: TObject; const Intent: JIntent; Flags, StartId: Integer): Integer;
    procedure AndroidServiceCreate(Sender: TObject);
  private
    { Private declarations }
    FThread: TThread;
    procedure Thinfinito;
    procedure LaunchMain;
  public
    { Public declarations }
  end;

var
  NotificationServiceDM: TNotificationServiceDM;
  flg: Integer; //flag of app run
  last : Int64; //timestamp of next try running AMain App

implementation

{%CLASSGROUP 'FMX.Controls.TControl'}

uses
  Androidapi.JNI.App, System.DateUtils;

{$R *.dfm}

//OnCreate service: run timer's thread
procedure TNotificationServiceDM.AndroidServiceCreate(Sender: TObject);
  begin
    Thinfinito; //run infinite loop for check time and restart main app
  end;

function TNotificationServiceDM.AndroidServiceStartCommand(const Sender: TObject; const Intent: JIntent; Flags,
  StartId: Integer): Integer;
// {$Define FOREGROUND}  //uncomment for mount foregroung service
  var
  fff: Integer;  //flag of stop request
  ppp: Integer;  //flag of ping
  ExtraData: String;
{$ifdef FOREGROUND}
  NewIntent: JIntent;
  ncb: JNotificationCompat_Builder;
  ntf: JNotification;
  PendingIntent: JPendingIntent;
{$endif}

begin

   fff := 0;    //start action by default
   ppp := 0;

   //check intent started service
   if(Assigned(intent)) then   //check intent was assigned (on start from App)
   begin
     //check extra data
     if(intent.hasExtra(StringToJString('stop'))) then fff := 1; //set stop flag for user's stop action
     if(intent.hasExtra(StringToJString('ping'))) then ppp := 1; //set ping flag for prevent restart main activity
     flg := 1; //set flag of App run
   end;


   //check stop flag
   if(fff>0) then
   begin
      JavaService.stopSelf;  //stop Service (App will not be reastarted)
   end
   else
   begin
      if(ppp = 0) then  //check this is start intent, not ping
      begin

          // ==============start service foreground ========================

    {$ifdef FOREGROUND}
    {$ifdef ORDINARY_NOTIFICATION}
    // for mount unclickable permanet notification of foreground service
    PendingIntent := TJPendingIntent.JavaClass.getActivity(
      JavaService.getApplicationContext, 0, Intent, 0
      );
    ntf := TJNotification.Create;
    ntf.icon := JavaService.getApplicationInfo.icon;
    ntf.setLatestEventInfo(
      JavaService.getApplicationContext,
      StrToJCharSequence('Torfone Service'),
      StrToJCharSequence('Torfone Service'), PendingIntent);
    {$else}
    // ALL THIS code is to make the host come to foreground when the user taps
    // the permanent notification
    // thanks to:
    // https://forums.embarcadero.com/message.jspa?messageID=847227
    // https://gist.github.com/kristopherjohnson/6211176

    NewIntent:= TAndroidHelper.Context.getPackageManager().getLaunchIntentForPackage(
      TAndroidHelper.Context.getPackageName());
    NewIntent.setAction(TJIntent.JavaClass.ACTION_MAIN);
    NewIntent.addCategory(TJIntent.JavaClass.CATEGORY_LAUNCHER);
    NewIntent.addFlags(TJIntent.JavaClass.FLAG_ACTIVITY_NEW_TASK);

    PendingIntent := TJPendingIntent.JavaClass.getActivity(
      TAndroidHelper.Context, 0, NewIntent,
      TJIntent.JavaClass.FLAG_ACTIVITY_NEW_TASK
      );

    ncb := TJNotificationCompat_Builder.JavaClass.init(TAndroidHelper.Context);
    ncb.setContentTitle(StrToJCharSequence('Torfone Service'));
    ncb.setTicker(StrToJCharSequence('Torfone Service'));
    ncb.setSmallIcon(JavaService.getApplicationInfo.icon);
    ncb.setContentIntent(PendingIntent);
    ncb.setOngoing(True);
    ntf := ncb.build;
    {$endif}

    JavaService.startForeground(StartId, ntf);  //start service foreground
  {$endif}

      //=========================================================

        LaunchMain; //restart main activity
      end;
   end;

  Result := TJService.JavaClass.START_STICKY;  //restart Service automatically
end;



//restart main activity
procedure TNotificationServiceDM.LaunchMain;
var
  Intent2: JIntent;
begin
  //create intent for reastart main activity
  Intent2 := TJIntent.Create;
  Intent2.setClassName(SharedActivityContext.getPackageName(), StringToJString('com.embarcadero.firemonkey.FMXNativeActivity'));
  Intent2.setAction(TJIntent.JavaClass.ACTION_MAIN);
  Intent2.addCategory(TJIntent.JavaClass.CATEGORY_DEFAULT);
  Intent2.addFlags(TJIntent.JavaClass.FLAG_ACTIVITY_NEW_TASK); //set this flag because reastarted other activity
  SharedActivityContext.startActivity(Intent2);  //start main activity or bring to front if already started
end;


//infinite loop for emulate Timer
procedure TNotificationServiceDM.Thinfinito;
 var
  atask : Itask;


begin

atask := Ttask.create(procedure()
      var
      hacer: boolean; //thread keep flag
      stamp : Int64;  //actual timestamp
     begin
      hacer := false;
      last := 0; //clear last activity for immediately force new
      flg := 1;  //set flag of ping on start service

        REPEAT
          begin
            sleep(1000); //do every second (or rary on sleep mode)
            stamp := DateTimetoUnix(now); //get actual timestamp
            if(stamp > last) then  //check for repeat
            begin
              last := stamp + 120;  //set new time for repeat
              if(flg = 0) then LaunchMain; //launch main app if no ping flag
              flg := 0; //clear ping flag for next
            end;
          end;
        UNTIL hacer = true;;
     end);

 atask.Start;   //start Timer emulation task
 //Note: in sleep this task is freezing and can runs generally rary then timer period 120 s

end;

end.
