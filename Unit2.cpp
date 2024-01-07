//setup
#define FOR_EKG   //additional function outside of this project
#define START_LOCK 30  //30''  time of prevent sleep on start in seconds (for run Tor success)
#define CHANGE_LOCK 120 //time of wake after IP was changed
#define NEXT_LOCK 30 //time of wake periodically
#define INTERV_LOCK 1800 //30' //interval betwewn periodically unlock
#define RESET_LOCK 600 //5' //interval after tor restart due IP changed to next  lock
#define MAXCOUNT 30

//button border colors
#define clBlack 0xFF000000
#define clBlue 0xFF0000FF
#define clGreen 0xFF00FF00
#define clRed 0xFFFF0000

//button fill colors
#define crBlack 0xFFE0E0E0
#define crBlue 0xFFC0C0FF
#define crGreen 0xFFC0FFC0
#define crRed 0xFFFFC0C0

//--------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop
//this
#include "Unit2.h"
//Android
#include <System.IOUtils.hpp>
#include <System.UITypes.hpp>
#include <System.Android.Service.hpp>
#include <Androidapi.Helpers.hpp>
#include <System.DateUtils.hpp>
#include "Androidapi.JNI.PowerManager.hpp"

//C utilites
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
#pragma resource ("*.XLgXhdpiTb.fmx", _PLAT_ANDROID)
#pragma resource ("*.LgXhdpiPh.fmx", _PLAT_ANDROID)
#pragma resource ("*.iPhone55in.fmx", _PLAT_IOS)


//globals
TForm2 *Form2;   //form
TStringList *List = new TStringList;  //String list for easy save/load test files
TLocalServiceConnection* con = new TLocalServiceConnection(); //for start Android service

unsigned int lastip=0xFFFFFFFF; //current IP address of main interface for detect changing

//status flags
unsigned char InCall=0;  //call mode
unsigned char InTalk=0;  //talk/mute mode
unsigned char InDir=0;   //allow direct mode flag
unsigned char InList=0;  //allow key receiving flag
unsigned char InWork=0;  //work mode (in call after IKE stage)
unsigned char InInit=0;  //Initialization
unsigned char InSave=0;  //need save changed configuration
unsigned char InRun=0;   //thread run flag
unsigned char InLock=0;  //wake lock mode
unsigned char InSelf=0; //call to himself
unsigned char InPing=0; //ping Tor
unsigned char InCnt=0; //counter of lock interval after Tor restart

unsigned int InTmr=0; //counter for periodicall unlock interval
int InTst=0; //counter for debug lock evnts

char torpath[512]={0}; //command string for Tor with parameters (set up on FormCreate, use for restart Tor
char par[32];  //notification parameter

 String SLOG="";

#include "whereami.h" //for check current path
#include "if.h"    //Torfone core general definitions
#include "ui.h"   //Torfone core interface GUI to UI

//for Telnet (test only)
extern "C" {
#define NOTE_IDDL 5 //call iddle
#define NOTE_RING 6 //incoming call
#define NOTE_ANSW 7 //remote party answer
  void fl_note(unsigned char type, char* par);
  void fl_init_path(char* path);
 }

//---------------------------------------------------------------------------
//Create Thread for Torfone
//---------------------------------------------------------------------------

    class MyThread: public TThread   //derive our thread class from TThread
    {
       public:
          __fastcall MyThread( int aParam ); //optionally pass parameter to our class

       protected:
          void __fastcall Execute();

       private:
          int param;
    };
//---------------------------------------------------------------------------
    __fastcall MyThread::MyThread( int aParam ): TThread( true )  //constructor
    {
       param = aParam; //save parameter
       Resume(); //run thread
    }
//---------------------------------------------------------------------------

    void __fastcall MyThread::Execute() //task body
    {   //Torfone Thread body

     InRun=1; //set run flag

     while(InRun)  //thread loop
     {
      tf_loop();  //process Torfone core
     }
    }

 //---------------------------------------------------------------------------

MyThread *Thread = NULL; //create object of our thread (will be run from FormCreate)

//Torfone constructor
//---------------------------------------------------------------------------
__fastcall TForm2::TForm2(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------

//change contact name field
void __fastcall TForm2::EditNameChange(TObject *Sender)
{
   EditName->FontColor=clBlue;
}
//---------------------------------------------------------------------------
//change contact address field
void __fastcall TForm2::EditAddressChange(TObject *Sender)
{
  EditAddress->FontColor=clBlue;
}
//---------------------------------------------------------------------------
 //change our address field
void __fastcall TForm2::EditOnionChange(TObject *Sender)
{
 InSave=2;
 LabelOnion->FontColor=clBlue;
 RectangleApply->Stroke->Color=clRed;
 RectangleApply->Fill->Color=crRed;
}
//---------------------------------------------------------------------------

//change Tor SOCKS5 port field
void __fastcall TForm2::EditTorChange(TObject *Sender)
{
  InSave=2;
  LabelTor->FontColor=clBlue;
  RectangleApply->Stroke->Color=clRed;
  RectangleApply->Fill->Color=crRed;
}
//---------------------------------------------------------------------------
//change TCP listener port field
void __fastcall TForm2::EditTCPChange(TObject *Sender)
{
  InSave=2;
  LabelTCP->FontColor=clBlue;
  RectangleApply->Stroke->Color=clRed;
  RectangleApply->Fill->Color=crRed;
}
//---------------------------------------------------------------------------

//change STUN adress field
void __fastcall TForm2::EditSTUNChange(TObject *Sender)
{
  InSave=2;
  LabelSTUN->FontColor=clBlue;
  RectangleApply->Stroke->Color=clRed;
  RectangleApply->Fill->Color=crRed;
}
//---------------------------------------------------------------------------

//change book/keystorage file/device name field
void __fastcall TForm2::EditBookChange(TObject *Sender)
{
  InSave=2;
  LabelBook->FontColor=clBlue;
  RectangleApply->Stroke->Color=clRed;
  RectangleApply->Fill->Color=crRed;
}
//---------------------------------------------------------------------------

//change WAN incoming connecting allow box
void __fastcall TForm2::CheckBoxWANChange(TObject *Sender)
{
  InSave=2;
  LabelWAN->FontColor=clBlue;
  RectangleApply->Stroke->Color=clRed;
  RectangleApply->Fill->Color=crRed;
}
//---------------------------------------------------------------------------

//check contact receiving allow box
void __fastcall TForm2::CheckBoxRcvdChange(TObject *Sender)
{
 if(CheckBoxRcvd->IsChecked) InList=1; else InList=0;
 ui_dokeyrcvd(InList);
 InSave=2;
 CheckBoxRcvd->FontColor=clBlue;
 RectangleApply->Stroke->Color=clRed;
 RectangleApply->Fill->Color=crRed;
}
//---------------------------------------------------------------------------

//Change speech output box
void __fastcall TForm2::CheckBoxSpkChange(TObject *Sender)
{
 InSave=2;
 CheckBoxSpk->FontColor=clBlue;
 RectangleApply->Stroke->Color=clRed;
 RectangleApply->Fill->Color=crRed;
}
//---------------------------------------------------------------------------

//Chenge password request box
void __fastcall TForm2::CheckBoxPaswChange(TObject *Sender)
{
 InSave=2;
 CheckBoxPasw->FontColor=clBlue;
 RectangleApply->Stroke->Color=clRed;
 RectangleApply->Fill->Color=crRed;

 //Delete password file if password requsted (box checked)
 if(CheckBoxPasw->IsChecked)
 {
  String S = System::Ioutils::TPath::GetDocumentsPath()+PathDelim+"pasword";
  if(FileExists(S)) DeleteFile(S);
 }

}
//---------------------------------------------------------------------------
 //Change Notification allow box
void __fastcall TForm2::CheckBoxNoteChange(TObject *Sender)
{
 InSave=2;
 CheckBoxNote->FontColor=clBlue;
 RectangleApply->Stroke->Color=clRed;
 RectangleApply->Fill->Color=crRed;
}
//---------------------------------------------------------------------------

 //20mS timer for periodically poll UI in GUI context
void __fastcall TForm2::Timer1Timer(TObject *Sender)
{
 int i;
 unsigned char cmd;

 while(cmd=ui_getcommand(par))  //get command from Torfone
 {
  switch(cmd) //do command by type
  {
   //set text
   case GUI_NAME_TEXT:{EditName->Text=AnsiString(par);EditName->FontColor=clBlack;EditName->Repaint();break;}
   case GUI_ADDR_TEXT:{EditAddress->Text=AnsiString(par);EditAddress->FontColor=clBlack;EditAddress->Repaint();break;}
   case GUI_SAS_TEXT:{LabelSAS->Text=AnsiString(par);LabelSAS->Repaint();break;}
   case GUI_INFO_TEXT:{LabelInfo->Text=AnsiString(par);LabelInfo->Repaint();break;}
   case GUI_FGP_TEXT:{LabelFGP->Text=AnsiString(par);LabelFGP->Repaint();break;}

   //set name icon
   case GUI_NAME_W:{EditName->FontColor=clBlack; ImageNameOK->Visible=false;ImageNameOK->Repaint();break;}
   case GUI_NAME_G:{EditName->FontColor=clBlack;ImageNameOK->Visible=true;ImageNameOK->Repaint();break;}

   //set adress icon
   case GUI_ADDR_W:{EditAddress->FontColor=clBlack; ImageAddrOK->Visible=false;ImageAddrOK->Repaint();break;}
   case GUI_ADDR_G:{EditAddress->FontColor=clBlack; ImageAddrOK->Visible=true;ImageAddrOK->Repaint();break;}

   //set fingerprint label color
   case GUI_FGP_W:{ LabelFGP->FontColor=clBlack;LabelFGP->Repaint();break;}
   case GUI_FGP_G:{ LabelFGP->FontColor=clGreen;LabelFGP->Repaint();break;}

   //set info data color
   case GUI_INFO_W:{LabelInfo->FontColor=clBlack;LabelInfo->Repaint();break;}
   case GUI_INFO_B:{LabelInfo->FontColor=clBlue;LabelInfo->Repaint();break;}
   case GUI_INFO_G:{LabelInfo->FontColor=clGreen;LabelInfo->Repaint();break;}
   case GUI_INFO_R:{LabelInfo->FontColor=clRed;LabelInfo->Repaint();break;}

   //button new color
   case GUI_BTNNEW_W:{RectangleNew->Stroke->Color=clBlack;RectangleNew->Fill->Color=crBlack;SpeedButtonNew->Repaint();break;}
   case GUI_BTNNEW_B:{RectangleNew->Stroke->Color=clBlue;RectangleNew->Fill->Color=crBlue;SpeedButtonNew->Repaint();break;}
   case GUI_BTNNEW_G:{RectangleNew->Stroke->Color=clGreen;RectangleNew->Fill->Color=crGreen;SpeedButtonNew->Repaint();break;}
   case GUI_BTNNEW_R:{RectangleNew->Stroke->Color=clRed;RectangleNew->Fill->Color=crRed;SpeedButtonNew->Repaint();break;}

   //button speke color
   case GUI_BTNSPK_W:{RectangleSpeke->Stroke->Color=clBlack;RectangleSpeke->Fill->Color=crBlack;SpeedButtonSpeke->Repaint();break;}
   case GUI_BTNSPK_B:{RectangleSpeke->Stroke->Color=clBlue;RectangleSpeke->Fill->Color=crBlue;SpeedButtonSpeke->Repaint();break;}
   case GUI_BTNSPK_G:{RectangleSpeke->Stroke->Color=clGreen;RectangleSpeke->Fill->Color=crGreen;SpeedButtonSpeke->Repaint();break;}
   case GUI_BTNSPK_R:{RectangleSpeke->Stroke->Color=clRed;RectangleSpeke->Fill->Color=crRed;SpeedButtonSpeke->Repaint();break;}

   //button change color
   case GUI_BTNCHG_W:{RectangleChange->Stroke->Color=clBlack;RectangleChange->Fill->Color=crBlack;SpeedButtonChange->Repaint();break;}
   case GUI_BTNCHG_B:{RectangleChange->Stroke->Color=clBlue;RectangleChange->Fill->Color=crBlue;SpeedButtonChange->Repaint();break;}
   case GUI_BTNCHG_G:{RectangleChange->Stroke->Color=clGreen;RectangleChange->Fill->Color=crGreen;SpeedButtonChange->Repaint();break;}
   case GUI_BTNCHG_R:{RectangleChange->Stroke->Color=clRed;RectangleChange->Fill->Color=crRed;SpeedButtonChange->Repaint();break;}

   //button key color: actually New button during call
   case GUI_BTNKEY_W:{RectangleNew->Stroke->Color=clBlack;RectangleNew->Fill->Color=crBlack;break;}
   case GUI_BTNKEY_B:{RectangleNew->Stroke->Color=clBlue;RectangleNew->Fill->Color=crBlue;break;}
   case GUI_BTNKEY_G:{RectangleNew->Stroke->Color=clGreen;RectangleNew->Fill->Color=crGreen;break;}
   case GUI_BTNKEY_R:{RectangleNew->Stroke->Color=clRed;RectangleNew->Fill->Color=crRed;break;}

   //button authent color: actually change button during call
   //button change color
   case GUI_AU_W:{RectangleChange->Stroke->Color=clBlack;RectangleChange->Fill->Color=crBlack;SpeedButtonChange->Repaint();break;}
   case GUI_AU_B:{RectangleChange->Stroke->Color=clBlue;RectangleChange->Fill->Color=crBlue;SpeedButtonChange->Repaint();break;}
   case GUI_AU_G:{RectangleChange->Stroke->Color=clGreen;RectangleChange->Fill->Color=crGreen;SpeedButtonChange->Repaint();break;}
   case GUI_AU_R:{RectangleChange->Stroke->Color=clRed;RectangleChange->Fill->Color=crRed;SpeedButtonChange->Repaint();break;}

   //button call/mute color
   case GUI_BTNCLL_W:{RectangleCall->Stroke->Color=clBlack;RectangleCall->Fill->Color=crBlack;SpeedButtonCall->Repaint();break;}
   case GUI_BTNCLL_B:{RectangleCall->Stroke->Color=clBlue;RectangleCall->Fill->Color=crBlue;SpeedButtonCall->Repaint();break;}
   case GUI_BTNCLL_G:{RectangleCall->Stroke->Color=clGreen;RectangleCall->Fill->Color=crGreen;SpeedButtonCall->Repaint();break;}
   case GUI_BTNCLL_R:{RectangleCall->Stroke->Color=clRed;RectangleCall->Fill->Color=crRed;SpeedButtonCall->Repaint();break;}

   //button list/direct color
   case GUI_BTNDIR_W:{RectangleDirect->Stroke->Color=clBlack;RectangleDirect->Fill->Color=crBlack;InDir=0;SpeedButtonDirect->Repaint();break;}
   case GUI_BTNDIR_B:{RectangleDirect->Stroke->Color=clBlue;RectangleDirect->Fill->Color=crBlue;InDir=0;SpeedButtonDirect->Repaint();break;}
   case GUI_BTNDIR_G:{RectangleDirect->Stroke->Color=clGreen;RectangleDirect->Fill->Color=crGreen;InDir=1;SpeedButtonDirect->Repaint();break;}
   case GUI_BTNDIR_R:{RectangleDirect->Stroke->Color=clRed;RectangleDirect->Fill->Color=crRed;InDir=1;SpeedButtonDirect->Repaint();break;}

   //button menu/cancel color
   case GUI_BTNTRM_W:{RectangleCancel->Stroke->Color=clBlack;RectangleCancel->Fill->Color=crBlack;SpeedButtonCancel->Repaint();break;}
   case GUI_BTNTRM_B:{RectangleCancel->Stroke->Color=clBlue;RectangleCancel->Fill->Color=crBlue;SpeedButtonCancel->Repaint();break;}
   case GUI_BTNTRM_G:{RectangleCancel->Stroke->Color=clGreen;RectangleCancel->Fill->Color=crGreen;SpeedButtonCancel->Repaint();break;}
   case GUI_BTNTRM_R:{RectangleCancel->Stroke->Color=clRed;RectangleCancel->Fill->Color=crRed;SpeedButtonCancel->Repaint();break;}

   //add contact to list
   case GUI_LIST_ADD:
   {
    ListBox1->Sorted=false;
    ListBox1->Items->Add(AnsiString(par)); //add contact
    ListBox1->Sorted=true; //esorte list
    ListBox1->Repaint();
    break;
   }

   //delete contact from list
   case GUI_LIST_DEL:
   {
    ListBox1->Sorted=false;
    i=ListBox1->Items->IndexOf(AnsiString(par)); //search in list
    if(i>0) ListBox1->Items->Delete(i); //remove from list
    ListBox1->Sorted=true;  //resorte
    ListBox1->Repaint();
    EditName->Text="";   //clear fieds: contact not selected
    EditAddress->Text="";
    EditName->FontColor=clBlack;
    EditAddress->FontColor=clBlack;
    EditName->Repaint();
    EditAddress->Repaint();
    break;
   }

   //set states
   case GUI_STATE_IDDLE: //state in iddle
   {
    //up buttons: iddle, book avaliable
    RectangleNew->Stroke->Color=clBlue;
    RectangleSpeke->Stroke->Color=clBlack;
    RectangleChange->Stroke->Color=clBlack;

    RectangleNew->Fill->Color=crBlue;
    RectangleSpeke->Fill->Color=crBlack;
    RectangleChange->Fill->Color=crBlack;


    SpeedButtonNew->Enabled=true;
    SpeedButtonSpeke->Enabled=false;
    SpeedButtonChange->Enabled=true;

    SpeedButtonNew->Repaint();
    SpeedButtonSpeke->Repaint();
    SpeedButtonChange->Repaint();

    //set down buttons: iddle
    ImageCallA->Visible=false;
    RectangleCall->Stroke->Color=clBlack;
    RectangleDirect->Stroke->Color=clBlack;
    RectangleCancel->Stroke->Color=clBlack;

    RectangleCall->Fill->Color=crBlack;
    RectangleDirect->Fill->Color=crBlack;
    RectangleCancel->Fill->Color=crBlack;

    SpeedButtonCall->Enabled=true;
    SpeedButtonDirect->Enabled=false;

    SpeedButtonCall->Repaint();
    SpeedButtonDirect->Repaint();
    SpeedButtonCancel->Repaint();

    //clear info/new
    LabelInfo->Text="";
    LabelInfo->FontColor=clBlack;
    EditNew->Text="";
    EditNew->FontColor=clBlack;
    EditNew->Visible=false;
    LabelInfo->Visible=true;
    EditNew->Repaint();
    LabelInfo->Repaint();

    //clear sas/tor
    LabelSAS->Text="";
    LabelSAS->FontColor=clBlack;
    LabelSAS->Repaint();
    ImageTor->Visible=true;
    ImageTor->Repaint();

    //clear name
    EditName->Text="";
    EditName->FontColor=clBlack;
    ImageNameOK->Visible=false; //set au icon black
    EditName->Enabled=true;
    EditName->Repaint();

    //clear addr
    EditAddress->Text="";
    EditAddress->FontColor=clBlack;
    ImageAddrOK->Visible=false;
    EditAddress->Enabled=true;
    EditAddress->Repaint();

   //clear values
    InCall=0;
    InTalk=0;
    InDir=0;
    InWork=0;
    InInit=0;
    InSelf=0; //clear call to himself flag

    #ifdef FOR_EKG
     fl_note(NOTE_IDDL, 0);  //notify over Telnet (test only)
    #endif

    //unlock app for allow sleep
    if(InLock==1) //check lock flag is set
    {
     ReleaseWakeLock(); //unlock app for sleep
     InLock=0;  //clear lock flag
    }

    break;
   }

   //state after incoming TCP connecting
   case GUI_WAKE:
   {
    //wake application
    if(!InLock) //check is not already lock
    {
     LabelOnion->Text="GUI_WAKE";
     AcquireWakeLock(); //lock for wake and prevent sleep during call
     InLock=1; //set lock flag
    }
    break;
   }

   //state during Initial Key Exchange
   case GUI_STATE_IKE:
   {
    //restart  service for bring app to front
    if((!InSelf) || (InLock)) con->StartService("NotificationService");

    RectangleNew->Stroke->Color=clBlack;
    RectangleSpeke->Stroke->Color=clBlack;
    RectangleChange->Stroke->Color=clBlack;

    RectangleNew->Fill->Color=crBlack;
    RectangleSpeke->Fill->Color=crBlack;
    RectangleChange->Fill->Color=crBlack;

    SpeedButtonNew->Enabled=false;
    SpeedButtonSpeke->Enabled=false;
    SpeedButtonChange->Enabled=false;

    SpeedButtonNew->Repaint();
    SpeedButtonSpeke->Repaint();
    SpeedButtonChange->Repaint();

     //set down buttons: iddle
    RectangleCall->Stroke->Color=clBlack;
    RectangleDirect->Stroke->Color=clBlack;
    RectangleCancel->Stroke->Color=clBlack;

    RectangleCall->Fill->Color=crBlack;
    RectangleDirect->Fill->Color=crBlack;
    RectangleCancel->Fill->Color=crBlack;

    SpeedButtonDirect->Enabled=false;

    SpeedButtonCall->Repaint();
    SpeedButtonDirect->Repaint();
    SpeedButtonCancel->Repaint();

    EditName->FontColor=clBlack;
    ImageNameOK->Visible=false; //set au icon black
    EditName->Enabled=false;
    EditName->Repaint();
    ImageNameOK->Repaint();
    ImageName->Repaint();

    EditAddress->FontColor=clBlack;
    ImageAddrOK->Visible=false;
    EditAddress->Enabled=false;
    EditAddress->Repaint();
    ImageAddrOK->Repaint();
    ImageAddr->Repaint();

    ImageTor->Visible=false;
    LabelSAS->Visible=true;
    ImageTor->Repaint();
    LabelSAS->Repaint();

    //clear info/new
    LabelInfo->Text="";
    LabelInfo->FontColor=clBlack;
    EditNew->Text="";
    EditNew->FontColor=clBlack;
    EditNew->Visible=false;
    LabelInfo->Visible=true;
    EditNew->Repaint();
    LabelInfo->Repaint();

    InInit=0;
    InCall=1;

    break;
   }

   //state in call
   case GUI_STATE_CALL:
   {

    SpeedButtonSpeke->Enabled=true;
    SpeedButtonSpeke->Repaint();

    //set down buttons: iddle
    SpeedButtonCall->Repaint();
    SpeedButtonDirect->Enabled=true;
    SpeedButtonDirect->Repaint();

    InWork=1;
    ui_dokeyrcvd(InList);

    break;
   }

   //state during call terminated
   case GUI_STATE_TERM:
   {
    RectangleNew->Stroke->Color=clBlack;
    RectangleSpeke->Stroke->Color=clBlack;
    RectangleChange->Stroke->Color=clBlack;

    RectangleNew->Fill->Color=crBlack;
    RectangleSpeke->Fill->Color=crBlack;
    RectangleChange->Fill->Color=crBlack;

    SpeedButtonNew->Enabled=false;
    SpeedButtonSpeke->Enabled=false;
    SpeedButtonChange->Enabled=false;

    SpeedButtonNew->Repaint();
    SpeedButtonSpeke->Repaint();
    SpeedButtonChange->Repaint();

    RectangleCall->Stroke->Color=clBlack;
    RectangleDirect->Stroke->Color=clBlack;
    RectangleCancel->Stroke->Color=clBlack;

    RectangleCall->Fill->Color=crBlack;
    RectangleDirect->Fill->Color=crBlack;
    RectangleCancel->Fill->Color=crBlack;

    SpeedButtonDirect->Enabled=false;

    SpeedButtonCall->Repaint();
    SpeedButtonDirect->Repaint();
    SpeedButtonCancel->Repaint();

    //up buttons: iddle, book unavaiable
    RectangleNew->Stroke->Color=clBlack;
    RectangleSpeke->Stroke->Color=clBlack;
    RectangleChange->Stroke->Color=clBlack;

    RectangleNew->Fill->Color=crBlack;
    RectangleSpeke->Fill->Color=crBlack;
    RectangleChange->Fill->Color=crBlack;

    LabelInfo->Text="";
    EditNew->Text="";
    EditNew->Visible=false;
    LabelInfo->Visible=true;
    InInit=0;

    InWork=0;
    InTalk=0;
    InDir=0;

    GridPanelLayoutPhone->Repaint();
    break;
   }


   case GUI_NOTE_TEXT:  //notification of incoming call: for acceptor only
   {

   #ifdef FOR_EKG
     //Command for exit app
    if(!par[0])
    {
     Application->Terminate();
     break;
    }
    #endif


      #ifdef FOR_EKG
      fl_note(NOTE_RING, 0);
      #endif
      if(!CheckBoxNote->IsChecked) break; //Check is notify allow
      if (NotificationCenter1->Supported())
      {
       TNotification *myNotification = NotificationCenter1->CreateNotification();
       __try
       {
        myNotification->AlertBody = String(par); //set text generated by Torfone
        NotificationCenter1->PresentNotification(myNotification);
       }
       __finally
       {
        myNotification->DisposeOf();
       }
      }
      break;
     }

   //callee accept call:  for originator only
   case GUI_NOTE_ANS:
   {
    if(InWork &&(!InTalk)) //in call  but not Talk mode
    {
     #ifdef FOR_EKG
     fl_note(NOTE_ANSW, 0);
     #endif

     InTalk=1;  //set Talk mode
     //check voice output
     if(CheckBoxSpk->IsChecked) ui_domute(1);  //LoudSpeaker
     else ui_domute(2);  //AirPhone

     SpeedButtonCall->Enabled=true;  //enable Call button disabled on call originate
     ImageCallA->Visible=true;
     SpeedButtonCall->Repaint();
    }
    break;
   }

  }//switch

 }//while(cmd)

}
//---------------------------------------------------------------------------



 //left-up button: password on start, new contact in iddle, speke in call
void __fastcall TForm2::SpeedButtonNewClick(TObject *Sender)
{
 char buf[32];
 strncpy(buf, ((AnsiString)EditNew->Text).c_str(), 32);  //copy New filed tesxt to buffer

 //******************************* before init: apply password ****************
 if(InInit>1)
 {
  ui_conf cf; //pointer to configuration structure
  String S;
  AnsiString SS;
  char str[256];
  FILE* fl=0;
  FILE* fw=0;
  int len, j;
  char c;


  //=============save password to file in private directory==============
  if((InInit!=3)&&(!CheckBoxPasw->IsChecked))
  {  //check if Manually press   and not CheckBoxPasw -> save passw to file
   S = System::Ioutils::TPath::GetDocumentsPath()+PathDelim+"pasword";
   List->Clear();
   List->Add(EditNew->Text);
   List->SaveToFile(S);
   List->Clear();
  }

  //====================restore Tor's private_key========================
  if(InInit==4)
  {
   S=System::Ioutils::TPath::GetDocumentsPath()+PathDelim+"hidden_service";
   if(!DirectoryExists(S))
   {
    CreateDir(S); //create hidden_service dir
    //open private_key  for write
    S=System::Ioutils::TPath::GetDocumentsPath()+PathDelim+"hidden_service"+PathDelim+"private_key";
    SS=AnsiString(S);
    strncpy(str, SS.c_str(), sizeof(str));
    fw = fopen(str, "wb" ); //create private_key file
    if(fw)
    {
     //open backup for reading
     S=System::Ioutils::TPath::GetSharedDocumentsPath()+PathDelim+"torfone"+PathDelim+"hs_backup";
     SS=AnsiString(S);
     strncpy(str, SS.c_str(), sizeof(str));
     fl = fopen(str, "rb" ); //read backup file
     if(fl)
     {
      //byte-by-byte copy from hs_backup to private_key file
      while(1)
      {
       c=fgetc(fl);
       if(c != EOF) fputc(c, fw);
       else break;
      } //while(1)

      fclose(fl);  //close backup
      fl=0;
      /////DeleteFile(S); //delete backup
      EditName->Text="Data restored success";
      EditAddress->Text="Restart Torfone now";
      SpeedButtonNew->Enabled=false; //disable this button
      InInit=6; //set flag for exit from inirialization procedure
     } //if(fl)
     fclose(fw); //close private_key file
     fw=0;
    } // if(fw)
   } //if(!DirectoryExists(S))
  } //if(InInit==4)

  //=================backup Tor's private_key=======================
  if(InInit==5)
  {
   S=System::Ioutils::TPath::GetDocumentsPath()+PathDelim+"hidden_service"+PathDelim+"private_key";
   if(FileExists(S))
   {
    SS=AnsiString(S);
    strncpy(str, SS.c_str(), sizeof(str));
    fl = fopen(str, "rb" ); //create private_key file
    if(fl)
    {
     S=System::Ioutils::TPath::GetSharedDocumentsPath()+PathDelim+"torfone"+PathDelim+"hs_backup";
     SS=AnsiString(S);
     strncpy(str, SS.c_str(), sizeof(str));
     fw = fopen(str, "wb" ); //read backup file
     if(fw)
     {
      //byte-by-byte copy from private_key to hs_backup file
      while(1)
      {
       c=fgetc(fl);
       if(c != EOF) fputc(c, fw);
       else break;
      } //while(1)
      fclose(fw);
      fw=0;
     } //if(fw)
     fclose(fl);
     fl=0;
    } //if(fl)
   } //if(FileExists(S)
  } //if(InInit==5)

  if(InInit==6) return;

  //copy current settings
  strncpy(cf.myadr, ((AnsiString)EditOnion->Text).c_str(), sizeof(cf.myadr)); //our address
 //------------------------------------------
 //try to load onion adress generated by tor
  if(cf.myadr[0]=='?') //if '?' specified in config file
  {  //path to hostname file created by Tor
   S=System::Ioutils::TPath::GetDocumentsPath()+PathDelim+"hidden_service"+PathDelim+"hostname";
   SS=AnsiString(S);
   strcpy(str, SS.c_str());
   fl = fopen(str, "r" ); //read hostname file created by Tor
   if(fl)
   {
    str[0]=0;
    fgets(str, sizeof(str), fl); //read hostname
    if(str[0]) //if readed success
    {
     len=strlen(str);
     for(j=0;j<len;j++) if(str[j]<=0x20) str[j]=0;  //skip <CR><LF>
     EditOnion->Text=String(str)+":"+EditTCP->Text;  //add port
     strncpy(cf.myadr, ((AnsiString)EditOnion->Text).c_str(), sizeof(cf.myadr)); //our address
    }
    fclose(fl);
    fl=0;
   }
  }
 //--------------------------------------------
  strncpy(cf.tor, ((AnsiString)EditTor->Text).c_str(), sizeof(cf.tor)); //tor port as string
  strncpy(cf.tcp, ((AnsiString)EditTCP->Text).c_str(), sizeof(cf.tcp)); //tcp listenet as string
  strncpy(cf.stun, ((AnsiString)EditSTUN->Text).c_str(), sizeof(cf.stun)); //STUN address as string
  strncpy(cf.com, ((AnsiString)EditBook->Text).c_str(), sizeof(cf.com)); //pass to addressbook/tocken
  strncpy(cf.mike, (char*)"", sizeof(cf.mike)); //audio in device
  strncpy(cf.spk, (char*)"", sizeof(cf.spk)); //audio out device

  cf.wan = (unsigned char) CheckBoxWAN->IsChecked;   //listen WAN
  InList =  (unsigned char) CheckBoxRcvd->IsChecked;  //receive keys from remote

  //set EDIT fields back with actual data
  EditOnion->Text=AnsiString((char*)cf.myadr);
  EditTor->Text=AnsiString((char*)cf.tor);
  EditTCP->Text=AnsiString((char*)cf.tcp);
  EditSTUN->Text=AnsiString((char*)cf.stun);
  EditBook->Text=AnsiString((char*)cf.com);

  ui_setconfig(&cf); //set config to UI

  //set password
  ui_doinit(buf);

  //set GUI
  EditNew->Text="";
  EditAddress->Text="";
  EditName->Text="";
  EditName->Enabled=true;
  EditAddress->Enabled=true;
  EditNew->Repaint();
  EditName->Repaint();
  EditAddress->Repaint();

  memset(buf, 0, sizeof(buf));
  InInit=1;
  LabelInfo->Text="Initialization...";
  LabelInfo->FontColor=clBlack;

  LabelFGP->Text="Init...";
  LabelFGP->Repaint();

  ImageNew->Visible=true;
  ImageNewA->Visible=false;
  ImageChange->Visible=true;
  ImageChangeA->Visible=false;

  SpeedButtonCall->Enabled=true;
  SpeedButtonCall->Repaint();
  
  SpeedButtonCancel->Enabled=true;
  SpeedButtonCancel->Repaint();
 }

 //New button press: show New Edit field
 if(!InInit)
 {
  LabelInfo->Text="";
  LabelInfo->Repaint();
  EditNew->Visible=true;
  LabelInfo->Visible=false;
  LabelInfo->Repaint();
  EditNew->Repaint();
  InInit=1;
 }
 else  //New button press once more: process data in New Edit field
 {
  EditNew->Visible=false;
  LabelInfo->Visible=true;
  EditNew->Repaint();
  LabelInfo->Repaint();
  InInit=0;
  if(buf[0]) //check some data exist
  {  //check for data are Latin chars only
   int i, j=0;
   int len=strlen(buf);  //number of chars
   for(i=0;i<len;i++) if(buf[i]<0) j=1;  //set flag if some char is not Latin (UTF8)
   if(!j)  //only Latin chars
   {
    if(!InCall) ui_donew(buf);  //in iddle state: create contact
    else if(InWork) ui_dospeke(buf); //in call: do SPEKE protocol
   }
   else //there are non-Latin chars
   {
    LabelInfo->Text="Only Latin!";  //notify user
    LabelInfo->FontColor=clRed;
    RectangleNew->Stroke->Color=clRed;
    RectangleNew->Fill->Color=crRed;
   }
   EditNew->Text="";
   EditNew->Repaint();
   memset(buf, 0, sizeof(buf)); //clear data
  }
 }

}
//---------------------------------------------------------------------------

 //right-up button: exit on start, change contact in iddle, send contact in call
void __fastcall TForm2::SpeedButtonChangeClick(TObject *Sender)
{
  //right-up button: exit on start, change contact in iddle, send contact in call
  char name[16];
  char adr[32];
  int len, i, j=0;

  //copy contact's name and address
  strncpy(name, ((AnsiString)EditName->Text).c_str(), 16);
  strncpy(adr, ((AnsiString)EditAddress->Text).c_str(), 32);

  if(InInit>1) Application->Terminate();  //before init: terminate Torfone
  else if(!InCall)  //after init: change name and/or address of selected contact
  {  //check for Latin chars only
   len=strlen(name);  //number of chars in name
   for(i=0;i<len;i++) if(name[i]<0) j=1; //set flag if some char is not Latin (UTF8)
   len=strlen(adr);  //number of chars in address
   for(i=0;i<len;i++) if(adr[1]<0) j=1;  //set flag if some char is not Latin (UTF8)
   if(!j) ui_dochange(name, adr); //in iddle: change contact
   else //there are non-Latin chars
   {
    LabelInfo->Text="Only Latin!"; //notify user
    LabelInfo->FontColor=clRed;
    RectangleChange->Stroke->Color=clRed;
    RectangleChange->Fill->Color=crRed;
   }
  }
}
//---------------------------------------------------------------------------

 //left-down button: outgoing call on iddle, talk/mute in call
void __fastcall TForm2::SpeedButtonCallClick(TObject *Sender)
{
  char name[16];
  char adr[32];

  //copy current name and address
  strncpy(name, ((AnsiString)EditName->Text).c_str(), 16);
  strncpy(adr, ((AnsiString)EditAddress->Text).c_str(), 32);

  if(!InCall) //in iddle: statrt outgoing call
  {
   ui_docall(name, adr); //start outgoing call to Torfone
   InCall=1; //set call flag

   RectangleNew->Stroke->Color=clBlack;
   RectangleNew->Fill->Color=crBlack;
   SpeedButtonNew->Repaint();
   SpeedButtonCall->Enabled=false; //temporary disable Call button
   SpeedButtonCall->Repaint();
  }
  else if(InWork) //in call: talk/mute
  {
   InTalk^=1; //change Talk state

   if(!InTalk) ui_domute(0);  //no talk
   else if(CheckBoxSpk->IsChecked) ui_domute(1);  //LoudSpeaker
   else ui_domute(2);  //AirPhone

   if(InTalk) ImageCallA->Visible=true; //set icon Talk
   else ImageCallA->Visible=false;   //set icon Mute
  }
}
//---------------------------------------------------------------------------

//middle_bottom button: change direct call state
void __fastcall TForm2::SpeedButtonDirectClick(TObject *Sender)
{
 if(InWork) //in call only
 {
  InDir^=1;  //change direct state
  ui_dodirect(InDir); //apply new state

  if(!InDir)
  {
   RectangleDirect->Stroke->Color=clBlack;  //denied
   RectangleDirect->Fill->Color=crBlack;  //denied
  }
  else
  {
   RectangleDirect->Stroke->Color=clRed; //request
   RectangleDirect->Fill->Color=crRed; //request
  }

  SpeedButtonDirect->Repaint();
 }
}
//---------------------------------------------------------------------------

 //right-down button: menu in iddle, cancel in call
void __fastcall TForm2::SpeedButtonCancelClick(TObject *Sender)
{
 ui_docancel();
}
//---------------------------------------------------------------------------

//menu down-left button: close menu
void __fastcall TForm2::SpeedButtonBackClick(TObject *Sender)
{
 MultiView2->HideMaster();
}
//---------------------------------------------------------------------------
 //menu down-middle button: save/apply settings
void __fastcall TForm2::SpeedButtonApplyClick(TObject *Sender)
{
 String S;

 //set path to configuration file
 S=System::Ioutils::TPath::GetSharedDocumentsPath()+PathDelim+"torfone"+PathDelim+"tf.ini";
 if(InSave) //saving config
 {
  //save config
  List->Clear();
  List->Add(EditOnion->Text);
  List->Add(IntToStr((int)CheckBoxRcvd->IsChecked));
  List->Add(IntToStr((int)CheckBoxPasw->IsChecked));
  List->Add(EditTCP->Text);
  List->Add(IntToStr((int)CheckBoxWAN->IsChecked));
  List->Add(EditTor->Text);
  List->Add(EditSTUN->Text);
  List->Add(EditBook->Text);
  List->Add(IntToStr((int)CheckBoxNote->IsChecked));
  List->Add(IntToStr((int)CheckBoxSpk->IsChecked));
  List->SaveToFile(S);

  RectangleApply->Stroke->Color=clBlue;
  RectangleApply->Fill->Color=crBlue;

  SpeedButtonApply->Repaint();
  InSave=0;
 }
 else //apply config
 {
  SpeedButtonBackClick(NULL);
  FormCreate(NULL);
 }
}
//---------------------------------------------------------------------------

 // right-down button: exit torfone
void __fastcall TForm2::SpeedButtonExitClick(TObject *Sender)
{
  //send intent to keepalive service with stop flag for self-stop
  String S="com.embarcadero.services.NotificationService";
  _di_JIntent intent;
  //send intent with data string to keepalive service
  intent = TJIntent::JavaClass->init();
  intent->setClassName(TAndroidHelper::Context->getPackageName(), TAndroidHelper::StringToJString(S));
  intent->setAction(TJIntent::JavaClass->ACTION_MAIN);
  intent->addCategory(TJIntent::JavaClass->CATEGORY_LAUNCHER);
  intent->addFlags(TJIntent::JavaClass->FLAG_ACTIVITY_NEW_TASK);
  intent->putExtra(StringToJString("stop"), true);  //set stop flag
  TAndroidHelper::Activity->startService(intent); //send for force stop service
  Label1->Text="Torfone will be closed, please wait...";  //show message
  Timer4->Enabled=true; //run timer for stop application after service was already sttopped
}
//---------------------------------------------------------------------------

//application startup, run torfone
void __fastcall TForm2::FormCreate(TObject *Sender)
{
  int l, i, j;
  String S;
  char str[512];
  char par[512];
  char tor[512];
  char chm[512];
  AnsiString SS;
  //pid_t pid=-1;
  FILE* fl=0;
  int torport=0;
  int length, dirname_length;
  int status=-9999;

  AcquireWakeLock1();  //lock sleep mode
  InLock=2;  //start flag CPU was locked on start
  Timer3->Enabled=true; //start unlock timer 30 sec

  InInit=2; //set starup state

 #ifdef FOR_EKG
 //create file send/received directory (for extended functional)
  S=System::Ioutils::TPath::GetSharedDocumentsPath()+PathDelim+"EKG";
  if(!DirectoryExists(S)) CreateDir(S);
 #endif

  //create torfone directory in extern storage if not exist yet
  S=System::Ioutils::TPath::GetSharedDocumentsPath()+PathDelim+"torfone";
  if(!DirectoryExists(S)) CreateDir(S);

  //set log output file
  SLOG=System::Ioutils::TPath::GetSharedDocumentsPath()+PathDelim+"torfone"+PathDelim+"pwr.txt";

  //try load settings from application config file
  S=System::Ioutils::TPath::GetSharedDocumentsPath()+PathDelim+"torfone"+PathDelim+"tf.ini";
  List->Clear();
  try
  {
   List->LoadFromFile(S);
  }
  catch(...)
  {

  }

  //set fields corresponds existing config settins
  l=List->Count;
  if(l>0) EditOnion->Text=List->Strings[0];
  if(l>1) CheckBoxRcvd->IsChecked = (bool)StrToIntDef(List->Strings[1], 1);
  if(l>2) CheckBoxPasw->IsChecked = (bool)StrToIntDef(List->Strings[2], 1);
  if(l>3) EditTCP->Text=List->Strings[3];
  if(l>4) CheckBoxWAN->IsChecked = (bool)StrToIntDef(List->Strings[4], 1);
  if(l>5) EditTor->Text=List->Strings[5];
  if(l>6) EditSTUN->Text=List->Strings[6];
  if(l>7) EditBook->Text=List->Strings[7];
  if(l>8) CheckBoxNote->IsChecked = (bool)StrToIntDef(List->Strings[8], 1);
  if(l>9) CheckBoxSpk->IsChecked = (bool)StrToIntDef(List->Strings[9], 1);

  //clear colors of filds by change
  EditOnion->FontColor=clBlack;
  LabelTCP->FontColor=clBlack;
  LabelWAN->FontColor=clBlack;
  LabelTor->FontColor=clBlack;
  LabelSTUN->FontColor=clBlack;;
  LabelBook->FontColor=clBlack;
  CheckBoxRcvd->FontColor=clBlack;
  CheckBoxPasw->FontColor=clBlack;
  CheckBoxNote->FontColor=clBlack;
  CheckBoxSpk->FontColor=clBlack;

  RectangleApply->Stroke->Color=clBlack;
  RectangleApply->Fill->Color=crBlack;

  InSave=0; //clear need saving flag

  //check Tor port is specified in config file
  torport=StrToIntDef(EditTor->Text, 0);
  if((torport<1)||(torport>65535)) torport=0;

   //check we already have private_key in Tor directory
  S=System::Ioutils::TPath::GetDocumentsPath()+PathDelim+"hidden_service"+PathDelim+"private_key";
  if(!FileExists(S)) //if not have yet
  {  //check we have backup file of Tor hidden dervice private key
     S=System::Ioutils::TPath::GetSharedDocumentsPath()+PathDelim+"torfone"+PathDelim+"hs_backup";
     if(FileExists(S)) InInit=4; else InInit=5; //4 is have, 5-have not
  }

  //compose and write torrc file
  S=System::Ioutils::TPath::GetSharedDocumentsPath()+PathDelim+"torfone"+PathDelim+"torrc";
  SS=AnsiString(S);
  strcpy(str, SS.c_str()); //path to torrc
  fl = fopen(str, "w" );
  if(fl)
  {
   //runs Tor as demon (shell must be exited for further work)
   S="RunAsDaemon 1";
   SS=AnsiString(S);
   strcpy(str, SS.c_str());
   fprintf(fl, "%s\r\n", str);
   //Tor's SOCKS5 port: default is 9150 but we use 9155 avoiding conflict with OrBot
   S="SocksPort "+EditTor->Text;
   SS=AnsiString(S);
   strcpy(str, SS.c_str());
   fprintf(fl, "%s\r\n", str);

   S="KeepalivePeriod 120";
   SS=AnsiString(S);
   strcpy(str, SS.c_str());
   fprintf(fl, "%s\r\n", str);

   //S="UseEntryGuards 0";
   //SS=AnsiString(S);
   //strcpy(str, SS.c_str());
   //fprintf(fl, "%s\r\n", str);

   S="AvoidDiskWrites 1";
   SS=AnsiString(S);
   strcpy(str, SS.c_str());
   fprintf(fl, "%s\r\n", str);


   //Tor's data directory in private storage
   S="DataDirectory "+System::Ioutils::TPath::GetDocumentsPath()+PathDelim+"tor_data";
   SS=AnsiString(S);
   strcpy(str, SS.c_str());
   fprintf(fl, "%s\r\n", str);
   //Hidden service directory in private storage
   S="HiddenServiceDir "+System::Ioutils::TPath::GetDocumentsPath()+PathDelim+"hidden_service";
   SS=AnsiString(S);
   strcpy(str, SS.c_str());
   fprintf(fl, "%s\r\n", str);
   //Old version of HS (short onion addreses)
   S="HiddenServiceVersion 2";
   SS=AnsiString(S);
   strcpy(str, SS.c_str());
   fprintf(fl, "%s\r\n", str);
   //Set hidden service port is equal to Torfone's TCP listener
   S="HiddenServicePort "+EditTCP->Text;
   SS=AnsiString(S);
   strcpy(str, SS.c_str());
   fprintf(fl, "%s\r\n", str);
   //Set this port as longlived (for chats)
   S="LongLivedPorts "+ EditTCP->Text;
   SS=AnsiString(S);
   strcpy(str, SS.c_str());
   fprintf(fl, "%s\r\n", str);

   fclose(fl);
  }

  //get path to our executable
  length = wai_getModulePath(NULL, 0, &dirname_length);
  if (length > 0)
  { //get path to module currently executed
   wai_getModulePath(torpath, length, &dirname_length);
  }

  //change name of Torfone executable to Tor executable
  j=0;
  l=strlen(torpath); //total len
  for(i=0;i<l;i++) if(torpath[i]=='/') j=i; //search last delimiter
  if(j) torpath[++j]=0; //cut module file name
  strcpy(torpath+strlen(torpath), (char*)"libTor.so"); //add Tor name

  //set command option: path to torrc
  S=System::Ioutils::TPath::GetSharedDocumentsPath()+PathDelim+"torfone"+PathDelim+"torrc";
  SS=AnsiString(S); //full path to torrc
  strcpy(par, " -f ");
  strncpy(par+strlen(par), SS.c_str(), sizeof(par)-8); //option with parameter

  //add path to log file in extern storage
  S=System::Ioutils::TPath::GetSharedDocumentsPath()+PathDelim+"torfone"+PathDelim+"log.txt";
  SS=AnsiString(S); //path to log file
  strcpy(par+strlen(par), " > ");
  strncpy(par+strlen(par), SS.c_str(), sizeof(par)-8); //option with parameter and output redirecting for shell

  //add options to Tor command line
  strcpy(torpath+strlen(torpath), par);

  //Run Tor over shell (if not restore on first start and tor port is specified)
  if(torport && (InInit!=4)) status = system(torpath);
  else torpath[0]=0;

  //run TF thread
  if(!Thread) Thread = new MyThread(0);

  //setup GUI on start
  ListBox1->Clear();
  ListBox1->Items->Add(" >>>");
  ListBox1->Items->Add("*MYSELF");

  LabelFGP->FontColor=clBlack;
  EditName->Enabled=false;
  EditAddress->Enabled=false;
  SpeedButtonNew->Enabled=true;
  SpeedButtonSpeke->Enabled=false;
  SpeedButtonChange->Enabled=true;

  SpeedButtonCall->Enabled=false;
  SpeedButtonDirect->Enabled=false;
  SpeedButtonCancel->Enabled=false;

  EditName->Text="Torfone V3.1 http://torfone.org";
  EditAddress->Text="Input password:";

  //show buttons OK and Exit for input password
  ImageNew->Visible=false;
  ImageNewA->Visible=true;
  ImageChange->Visible=false;
  ImageChangeA->Visible=true;

  EditNew->Visible=true;
  LabelInfo->Visible=false;

  #ifdef FOR_EKG
  //set path to EKG forlse
  S=System::Ioutils::TPath::GetSharedDocumentsPath()+PathDelim+"EKG"+PathDelim;
  strncpy(str, ((AnsiString)S).c_str(), sizeof(str));
  fl_init_path(str);
  #endif

  //set path to key material and addressbook
  S=System::Ioutils::TPath::GetSharedDocumentsPath()+PathDelim+"torfone"+PathDelim;
  strncpy(str, ((AnsiString)S).c_str(), sizeof(str));
  bk_init_path(str);

  //Try to load Password from file
  if((!CheckBoxPasw->IsChecked)&&(InInit<3)) //if allowed and not first start
  {
   //try load from saved pasw file
   S = System::Ioutils::TPath::GetDocumentsPath()+PathDelim+"pasword";
   List->Clear();
   try
   {
    List->LoadFromFile(S);
   }
   catch(...)
   {

   }

   //check password is loaded
   if(List->Count)
   {
    Sleep(1000); //pause
    InInit=3; //set init state for autoload
    EditNew->Text=List->Strings[0]; //set password
    SpeedButtonNewClick(0); //process password (emulate press OK button
    List->Clear(); //clear List
   }
  }

  //start keepalive service
  con->StartService("NotificationService");
  Timer5->Enabled=true; //start ping timer for periodically sent ping invents to keep alive service




  try
  {
   List->LoadFromFile(SLOG);
  }
  catch(...)
  {
   List->Clear();
  }
  if(List->Count > MAXCOUNT)
  {
   List->SaveToFile(SLOG+".old");
   List->Clear();
  }
  List->Add(FormatDateTime("hh:mm:ss ", Now())+ "start");
  List->SaveToFile(SLOG);


}
//---------------------------------------------------------------------------
//application shutdows, stop torfone
void __fastcall TForm2::FormClose(TObject *Sender, TCloseAction &Action)
{
 //shutdown
 if(Thread) Thread->Terminate();
 Sleep(500);
}
//---------------------------------------------------------------------------

//select contact in list
void __fastcall TForm2::ListBox1ItemClick(TCustomListBox * const Sender, TListBoxItem * const Item)

{
  char name[16];

  if(ListBox1->ItemIndex<0) return; //check contact must be selected
  //copy name to current name
  strncpy(name, ((AnsiString)ListBox1->Items->Strings[ListBox1->ItemIndex]).c_str(), 16); //set name field

  //check valid contact selected
  if(name[0] && (name[1]!='>'))
  {
   if(InWork) ui_dokey(name); //in call: send contact to remote
   else
   {  //in iddle
    EditName->Text=AnsiString(name);
    EditName->FontColor=clBlack;
    EditAddress->FontColor=clBlack;
    EditAddress->Text="";
    ui_doselect(name); //select contact

    //check if *MYSELF selected
    if(name[0]=='*')
    {
     //copy our onion address to clipboard
     _di_IFMXClipboardService service;
     if(TPlatformServices::Current->SupportsPlatformService(__uuidof(IFMXClipboardService)) &&
		(service = TPlatformServices::Current->GetPlatformService(__uuidof(IFMXClipboardService))))
      {
       service->SetClipboard(TValue::_op_Implicit(EditOnion->Text));
       LabelInfo->Text="Copied to clipboard";
       LabelInfo->FontColor=clBlue;
      }
    }

   }
  }
  else  MultiView1->HideMaster();//>>> selected
}
//---------------------------------------------------------------------------



//Timer for redraw form
void __fastcall TForm2::Timer2Timer(TObject *Sender)
{   //repaint buttons
    SpeedButtonNew->Repaint();
    SpeedButtonSpeke->Repaint();
    SpeedButtonChange->Repaint();
    SpeedButtonCall->Repaint();
    SpeedButtonDirect->Repaint();
    SpeedButtonCancel->Repaint();
   //process messages
    Application->ProcessMessages();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//timer for terminate app with timeout for terminate keepalive service
void __fastcall TForm2::Timer4Timer(TObject *Sender)
{
  Application->Terminate(); //exit app
}
//---------------------------------------------------------------------------

//===========================================================================
//              Keep-alive  procedures for Torfone, service and Tor
//===========================================================================

//20s timer for ping server by intents preventing re-run main activity
//always run in interval of 20 sec
void __fastcall TForm2::Timer5Timer(TObject *Sender)
{
 unsigned int nn;

 //send intent to keepalive service with ping flag
 String S="com.embarcadero.services.NotificationService";
 _di_JIntent intent;
  //send ping intent with data string to keepalive service
 intent = TJIntent::JavaClass->init();
 intent->setClassName(TAndroidHelper::Context->getPackageName(), TAndroidHelper::StringToJString(S));
 intent->setAction(TJIntent::JavaClass->ACTION_MAIN);
 intent->addCategory(TJIntent::JavaClass->CATEGORY_LAUNCHER);
 intent->addFlags(TJIntent::JavaClass->FLAG_ACTIVITY_NEW_TASK);
 intent->putExtra(StringToJString("ping"), true);  //set ping flag
 TAndroidHelper::Activity->startService(intent); //send for force stop service

 //check local IP is changed
 nn=(unsigned int)get_local_if(0);  //get local IP
 if(lastip==0xFFFFFFFF) lastip=nn; //on app start
 if(nn!=0x0100007F) //if has internet (our interface is not localhost 127.0.0.1)
 {  //compare current IP with last
  if(nn!=lastip) //if IP was changed
  {
    String IP="127.0.0.1";  //tor command udp interface on localhost
    unsigned short Port=9877;   //udp port of our Tor's command interface
    TByteDynArray ba;  //data array

    lastip=nn; //set new IP
    InTmr=RESET_LOCK+DateTimeToUnix(Now()); //set interval for next test after IP was changed (10 min)

   //log Ip was changed
   try
   {
    List->LoadFromFile(SLOG); //try load current log file
   }
   catch(...)
   {
    List->Clear();
   }
   if(List->Count > MAXCOUNT)   //check log file already too long
   {
    List->SaveToFile(SLOG+".old");   //save current log file to old copy
    List->Clear(); //clear
   }
   S=IntToStr((int)(nn&0xFF))+"."+IntToStr((int)((nn>>8)&0xFF))+"."+IntToStr((int)((nn>>16)&0xFF))+"."+IntToStr((int)((nn>>24)&0xFF));
   List->Add(FormatDateTime("hh:mm:ss ", Now())+ S);
   List->SaveToFile(SLOG);  //log new IP adress


   //Lock CPU if not locked yet and screen is off
    if((!InLock)&&(!GetPowerManager()->isScreenOn()) )
     {
      AcquireWakeLock1();  //lock sleep mode
      InLock=2;  //set flag CPU was locked, need unlock
     }

   //send exit packet to Tor
     ba.Length=1;       //1 byte
     ba[ba.Low]=0xFF;   //request code
     IdUDPServer1->SendBuffer(IP, Port, Id_IPv4, ba); //send
     Timer6->Enabled=true; //start Tor restart timer (10 sec for comletely exit old Tor))
     InCnt=0;

  } //end of IP was changed
  else //ip not changed: check is time for self-test
  {
   char nnn[16];
   char aaa[32];
   String IP="127.0.0.1";
   unsigned short Port=9877;
   TByteDynArray ba;

   //check inping flag and ping Tor periodically during flag is set
   if(InPing)
   {
    ba.Length=1;       //1 byte
    ba[ba.Low]=0;   //request code
    IdUDPServer1->SendBuffer(IP, Port, Id_IPv4, ba); //send 1 byte UDP ping packet to Tor (will be answer)
   }

   //get current timestamp
   nn=DateTimeToUnix(Now());  //get timestamp
   if(!InTmr) InTmr=nn+RESET_LOCK; //send self-test interval once after app was started

   //check time for self test
   if(nn>InTmr)
   {
    InTmr=nn+INTERV_LOCK; //set time for next check
    if((!InCall)&&(!InLock)) //check for we not in call now
    {
     //check is screen off (inactive phone mode) and Lock CPU on test
     if(!GetPowerManager()->isScreenOn())
     {
      AcquireWakeLock1();  //lock sleep mode
      InLock=2;  //set flag CPU was locked, need unlock
     }
     Timer3->Enabled=true; //start timer for check self-test result (90 sec)

     //send ping to Tor
     ba.Length=1;       //1 byte
     ba[ba.Low]=0;   //request code
     IdUDPServer1->SendBuffer(IP, Port, Id_IPv4, ba); //send
     InPing=1; //set ping flag (will be cleared on receive anwer from Tor)

     //try start test call to himself
     memset(aaa, 0, sizeof(aaa)); //clear address
     strncpy(aaa, ((AnsiString)EditOnion->Text).c_str(), 32);  //copy our onion address
     if(!memcmp(aaa+16, (char*)".onion", 6)) //check is valid our onion address
     {
       //log self-test to file
       try
       {
        List->LoadFromFile(SLOG);
       }
       catch(...)
       {
        List->Clear();
       }
       if(List->Count > MAXCOUNT)
       {
        List->SaveToFile(SLOG+".old");
        List->Clear();
       }
       S="test";
       List->Add(FormatDateTime("hh:mm:ss ", Now())+ S);  //log start of selftest
       List->SaveToFile(SLOG);

       //call himself (to own onion address over Tor)
       InSelf=1; //set flag: selftest active (wiil be cleared on call to self will be success)
       strcpy(nnn, (char*)"*MYSELF"); //copy myself name
       ui_docall(nnn, aaa); //start outgoing call to Torfone
       InCall=1; //set call flag
       RectangleNew->Stroke->Color=clBlack;
       RectangleNew->Fill->Color=crBlack;
       SpeedButtonCall->Enabled=false; //temporary disable Call button
     } //valid onion
    } //not in call
   } //time for test
  } //ip not changed
 } //has internet
 else lastip=nn;


}
//---------------------------------------------------------------------------

//receive UDP ping answer from Tor
void __fastcall TForm2::IdUDPServer1UDPRead(TIdUDPListenerThread *AThread, const TIdBytes AData,
          TIdSocketHandle *ABinding)
{
 if(AData.Length==1) InPing=0; //clear ping flag
}
//---------------------------------------------------------------------------

//90 sec Timer for check self-test result
void __fastcall TForm2::Timer3Timer(TObject *Sender)
{
 Timer3->Enabled=false;  //stop timer (chek self-testing result once)

 //check selftest result flags
  if(InSelf || InPing)  //if selftest or Tor ping was fail
  {
   String S;
   String IP="127.0.0.1";
   unsigned short Port=9877;
   TByteDynArray ba;

   //lock CPU if sreen is off and was not locked yet
   if((!InLock)&&(!GetPowerManager()->isScreenOn()) )
   {
    AcquireWakeLock1();  //lock sleep mode
    InLock=2;  //set flag CPU was locked, need unlock
   }

   if(InSelf) ui_docancel();  //stop call to himself if active

   //log to file
   try
   {
    List->LoadFromFile(SLOG);  //load log file
   }
   catch(...)
   {
    List->Clear();  //no log
   }
   if(List->Count > MAXCOUNT)  //check log size
   {
    List->SaveToFile(SLOG+".old");  //save old log to file
    List->Clear();  //clear log for new file
   }

   //log test was fail
   S="failPS="+IntToStr(InPing)+IntToStr(InSelf);
   List->Add(FormatDateTime("hh:mm:ss ", Now())+ S); //log selftest fail event
   List->SaveToFile(SLOG);

   InPing=0;  //clear flags
   InSelf=0;

   //send exit packet to Tor
   ba.Length=1;       //1 byte
   ba[ba.Low]=0xFF;   //request code
   IdUDPServer1->SendBuffer(IP, Port, Id_IPv4, ba); //send
   Timer6->Enabled=true; //start Tor restart timer 10 sec (for comletely exit old Tor)
   InCnt=0;
  }
  else if(InLock==2) //Self-test is OK and check CPU was locked
  {
   InLock=0; //clear lock flag
   ReleaseWakeLock();  //ulock for enebling sleep
  }
}



//restart Tor after send exit 10s Timer
void __fastcall TForm2::Timer6Timer(TObject *Sender)
{
//first fire after Exit Tor
//lock CPU if sreen is off and was not locked yet
  if((!InLock)&&(!GetPowerManager()->isScreenOn()) )
  {
   AcquireWakeLock1();  //lock sleep mode
   InLock=2;  //set flag CPU was locked, need unlock
  }

  if(torpath[0]) system(torpath); //start Tor if allowed
  InTmr=RESET_LOCK+DateTimeToUnix(Now()); //set time for next selftes after Tor was restarted  (10 min)

  Timer6->Enabled=false; //disable Timer6 (restart once)
  Timer7->Enabled=true; //start 60 sec timer for Unlock CPU after new Tor will be completely runs
}
//---------------------------------------------------------------------------

//60 sec Timer for connect new Tor to network after restart
void __fastcall TForm2::Timer7Timer(TObject *Sender)
{
  if(InLock==2) //check CPU was locked and unlock it
  {
   InLock=0; //clear lock flag
   ReleaseWakeLock();  //ulock for enebling sleep
  }
  Timer7->Enabled=false; //stop timer (once)
}
//---------------------------------------------------------------------------

