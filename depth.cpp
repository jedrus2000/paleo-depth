//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USERES("depth.res");
USEFORM("main.cpp", Form1);
USEFORM("progress.cpp", Form2);
USEFORM("info.cpp", InfoForm);
USEFORM("deep.cpp", Form3);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    try
    {
         Application->Initialize();
         Application->CreateForm(__classid(TForm1), &Form1);
         Application->CreateForm(__classid(TForm3), &Form3);
         Application->Run();
    }
    catch (Exception &exception)
    {
         Application->ShowException(&exception);
    }
    return 0;
}
//---------------------------------------------------------------------------
