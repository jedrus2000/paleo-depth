//---------------------------------------------------------------------------
#ifndef infoH
#define infoH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TInfoForm : public TForm
{
__published:	// IDE-managed Components
    TStaticText *LongText;
    TStaticText *StaticText2;
    TStaticText *StaticText3;
    TStaticText *LatText;
    TStaticText *StaticText1;
    TStaticText *DepthText;
    TStaticText *StaticText4;
    TStaticText *DistanceText;
private:	// User declarations
public:		// User declarations
    __fastcall TInfoForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TInfoForm *InfoForm;
//---------------------------------------------------------------------------
#endif
