//---------------------------------------------------------------------------
#ifndef mainH
#define mainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <stdio.h>
#include <float.h>
#include <math.h>

//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
    TImage *Image1;
    void __fastcall FormCreate(TObject *Sender);
    void __fastcall FormShow(TObject *Sender);
    void __fastcall Image1MouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
    void __fastcall Image1MouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
    void __fastcall Image1MouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
private:	// User declarations
     FILE *FileHandle;
     FILE *DATHandle;
     void __fastcall PrepareDAT();
     bool __fastcall OpenDAT();
     void __fastcall Convert();
     void __fastcall PrepareData();
     bool __fastcall OpenDataFile();
     bool __fastcall GetData(float &longitude, float &latitude, float &depth);
     void __fastcall GetDATBuffer();
     void __fastcall Draw();

     void __fastcall Shade();
     float __fastcall GetDepthAt(int x, int y);

     int begX, begY, endX, endY;
     bool mouse_down;

     TColor __fastcall HSVtoRGB(float h, float s, float v );
     TColor __fastcall HSLRangeToRGB (int H, int S, int L);
     TColor __fastcall HSLtoRGB (double H, double S, double L);
     BYTE __fastcall HueToColourValue(double Hue, double M1, double M2);
     
public:		// User declarations
    __fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;

long __fastcall filesize(FILE *stream);
//---------------------------------------------------------------------------
#endif
