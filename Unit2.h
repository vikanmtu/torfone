//---------------------------------------------------------------------------

#ifndef Unit2H
#define Unit2H
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Layouts.hpp>
#include <FMX.Types.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.Edit.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.TabControl.hpp>
#include <FMX.ImgList.hpp>
#include <System.ImageList.hpp>
#include <FMX.Objects.hpp>
#include <FMX.MultiView.hpp>
#include <FMX.ListBox.hpp>
#include <FMX.Memo.hpp>
#include <FMX.ScrollBox.hpp>
#include <System.Notification.hpp>
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdGlobal.hpp>
#include <IdSocketHandle.hpp>
#include <IdUDPBase.hpp>
#include <IdUDPServer.hpp>
//---------------------------------------------------------------------------
class TForm2 : public TForm
{
__published:	// IDE-managed Components
    TGridPanelLayout *GridPanelLayoutPhone;
    TGridPanelLayout *GridPanelLayoutDirect;
    TSpeedButton *SpeedButtonCall;
    TSpeedButton *SpeedButtonDirect;
    TSpeedButton *SpeedButtonCancel;
    TPanel *PanelLogo;
    TLabel *LabelFGP;
    TLabel *LabelSAS;
    TTimer *Timer1;
    TStyleBook *StyleBook1;
    TMultiView *MultiView1;
    TListBox *ListBox1;
    TMultiView *MultiView2;
    TGridPanelLayout *GridPanelLayoutSettings;
    TPanel *PanelSettings;
    TLabel *LabelOnion;
    TGridPanelLayout *GridPanelLayout1;
    TLabel *LabelTor;
    TLabel *LabelTCP;
    TLabel *LabelWAN;
    TEdit *EditTor;
    TEdit *EditTCP;
    TCheckBox *CheckBoxWAN;
    TLabel *LabelSTUN;
    TEdit *EditSTUN;
    TLabel *LabelBook;
    TEdit *EditBook;
    TEdit *EditOnion;
    TGridPanelLayout *GridPanelLayoutApply;
    TSpeedButton *SpeedButtonBack;
    TSpeedButton *SpeedButtonApply;
    TSpeedButton *SpeedButtonExit;
    TCheckBox *CheckBoxRcvd;
    TLabel *LabelKey;
    TGridPanelLayout *GridPanelLayoutName;
    TEdit *EditName;
    TPanel *Panel2;
    TImage *ImageName;
    TImage *ImageNameOK;
    TGridPanelLayout *GridPanelLayoutAddress;
    TEdit *EditAddress;
    TPanel *Panel1;
    TImage *ImageAddr;
    TImage *ImageAddrOK;
    TPanel *PanelNew;
    TLabel *LabelInfo;
    TEdit *EditNew;
    TGridPanelLayout *GridPanelLayoutFGP;
    TSpeedButton *SpeedButtonNew;
    TImage *ImageTor;
    TSpeedButton *SpeedButtonSpeke;
    TTimer *Timer2;
    TLabel *Label1;
    TCheckBox *CheckBoxSpk;
    TNotificationCenter *NotificationCenter1;
    TCheckBox *CheckBoxPasw;
    TCheckBox *CheckBoxNote;
    TRectangle *RectangleCall;
    TImage *ImageCall;
    TImage *ImageCallA;
    TRectangle *RectangleDirect;
    TRectangle *RectangleCancel;
    TRectangle *RectangleNew;
    TRectangle *RectangleSpeke;
    TImage *ImageDirect;
    TImage *ImageCancel;
    TImage *ImageNew;
    TImage *ImageSpeke;
    TRectangle *RectangleBack;
    TRectangle *RectangleApply;
    TRectangle *RectangleExit;
    TImage *ImageBack;
    TImage *ImageApply;
    TImage *ImageExit;
    TSpeedButton *SpeedButtonChange;
    TRectangle *RectangleChange;
    TImage *ImageChange;
    TImage *ImageNewA;
    TImage *ImageChangeA;
    TTimer *Timer3;
    TTimer *Timer4;
    TTimer *Timer5;
    TIdUDPServer *IdUDPServer1;
    TTimer *Timer6;
    TTimer *Timer7;
    void __fastcall EditNameChange(TObject *Sender);
    void __fastcall EditAddressChange(TObject *Sender);
    void __fastcall EditOnionChange(TObject *Sender);
    void __fastcall EditTorChange(TObject *Sender);
    void __fastcall EditTCPChange(TObject *Sender);
    void __fastcall EditSTUNChange(TObject *Sender);
    void __fastcall EditBookChange(TObject *Sender);
    void __fastcall CheckBoxWANChange(TObject *Sender);
    void __fastcall Timer1Timer(TObject *Sender);
    void __fastcall SpeedButtonNewClick(TObject *Sender);
    void __fastcall SpeedButtonCallClick(TObject *Sender);
    void __fastcall SpeedButtonDirectClick(TObject *Sender);
    void __fastcall SpeedButtonCancelClick(TObject *Sender);
    void __fastcall SpeedButtonBackClick(TObject *Sender);
    void __fastcall SpeedButtonApplyClick(TObject *Sender);
    void __fastcall SpeedButtonExitClick(TObject *Sender);
    void __fastcall FormCreate(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall ListBox1ItemClick(TCustomListBox * const Sender, TListBoxItem * const Item);
    void __fastcall CheckBoxRcvdChange(TObject *Sender);
    void __fastcall Timer2Timer(TObject *Sender);
    void __fastcall CheckBoxSpkChange(TObject *Sender);
    void __fastcall CheckBoxPaswChange(TObject *Sender);
    void __fastcall CheckBoxNoteChange(TObject *Sender);
    void __fastcall SpeedButtonChangeClick(TObject *Sender);
    void __fastcall Timer3Timer(TObject *Sender);
    void __fastcall Timer4Timer(TObject *Sender);
    void __fastcall Timer5Timer(TObject *Sender);
    void __fastcall IdUDPServer1UDPRead(TIdUDPListenerThread *AThread, const TIdBytes AData,
          TIdSocketHandle *ABinding);
    void __fastcall Timer6Timer(TObject *Sender);
    void __fastcall Timer7Timer(TObject *Sender);


private:	// User declarations
public:		// User declarations
    __fastcall TForm2(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm2 *Form2;
//---------------------------------------------------------------------------
#endif
