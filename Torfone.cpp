//---------------------------------------------------------------------------

#include <fmx.h>
#ifdef _WIN32
#include <tchar.h>
#endif
#pragma hdrstop
#include <System.StartUpCopy.hpp>
//---------------------------------------------------------------------------
USEFORM("Unit2.cpp", Form2);
USEFORMNS("NotificationService\NotificationServiceUnit.pas", Notificationserviceunit, NotificationServiceDM); /* TAndroidService: File Type */
//---------------------------------------------------------------------------
extern "C" int FMXmain()
{
    try
    {
         Application->Initialize();
         Application->FormFactor->Orientations = TScreenOrientations() << TScreenOrientation::Portrait;
         Application->CreateForm(__classid(TForm2), &Form2);
         Application->Run();
    }
    catch (Exception &exception)
    {
         Application->ShowException(&exception);
    }
    catch (...)
    {
         try
         {
             throw Exception("");
         }
         catch (Exception &exception)
         {
             Application->ShowException(&exception);
         }
    }
    return 0;
}
//---------------------------------------------------------------------------
