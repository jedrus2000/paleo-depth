//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "main.h"
#include "progress.h"
#include "info.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

#define X_CELLS 15
#define Y_CELLS 7

#define X_CELL_SIZE 75   // 75
#define Y_CELL_SIZE 150  // 150

#define DDEG_PER_CELL 0.1 // 0.1

#define KM_PER_PIX 0.3 / (X_CELL_SIZE / 21.3)

TForm1 *Form1;

int HSLRange = 240;

long file_size;
long curr_pos;

float buffer[Y_CELL_SIZE*Y_CELLS];
// WORD buffer[2048];

//---------------------------------------------------------------------------
//
//
//  My Own
//
//---------------------------------------------------------------------------
long __fastcall filesize(FILE *stream)
{
   long curpos, length;
   curpos = ftell(stream);
   fseek(stream, 0L, SEEK_END);
   length = ftell(stream);
   fseek(stream, curpos, SEEK_SET);
   return length;
}

BYTE __fastcall TForm1::HueToColourValue(double Hue, double M1, double M2)
{
    double V;

    if (Hue < 0.0)
      Hue ++;
    else
      if (Hue > 1.0)
        Hue --;

    if ((6.0 * Hue) < 1.0)
      V = M1 + (M2 - M1) * Hue * 6.0;
    else
      if ((2.0 * Hue) < 1.0)
        V = M2;
      else
        if ((3.0 * Hue) < 2.0)
          V = M1 + (M2 - M1) * (2.0/3.0 - Hue) * 6.0;
        else
          V = M1;

    return (BYTE) (255.0 * V);
}

TColor __fastcall TForm1::HSLtoRGB (double H, double S, double L)
{
    BYTE R, G, B;
    double M1,M2;

    if (S == 0.0)
    {
        R = (BYTE) (255.0 * L);
        G = R;
        B = R;
    }
    else
    {
        if (L <= 0.5)
            M2 = L * (1.0 + S);
        else
            M2 = L + S - L * S;

        M1 = 2.0 * L - M2;

        R = HueToColourValue (H + 1.0/3.0,M1,M2);
        G = HueToColourValue (H,M1,M2);
        B = HueToColourValue (H - 1.0/3.0,M1,M2);
    }

//    ShowMessage("R="+IntToStr(R)+" G="+IntToStr(G)+" B="+IntToStr(B));

    return ( ((DWORD) B << 16) | ((DWORD) G << 8) | ((DWORD) R));
}


TColor __fastcall TForm1::HSLRangeToRGB (int H, int S, int L)
{
  return  HSLtoRGB( (double) H / ((double) HSLRange-1.00),
    (double) S / (double) HSLRange, (double) L / (double) HSLRange);
}

// r,g,b values are from 0 to 1
// h = [0,360], s = [0,1], v = [0,1]
//              if s == 0, then h = -1 (undefined)
TColor __fastcall TForm1::HSVtoRGB(float h, float s, float v )
{
        int i;
        float f, p, q, t;
        float r , g , b;

        if( s == 0 )
        {
                // achromatic (grey)
                r = v;
                g = v;
                b = v;

                r /= 256.0;
                g /= 256.0;
                b /= 256.0;
                return ( ((DWORD) b << 16) | ((DWORD) g << 8) | ((DWORD) r));

        }

        h /= 60;                        // sector 0 to 5
        i = floor( h );
        f = h - i;                      // factorial part of h
        p = v * ( 1 - s );
        q = v * ( 1 - s * f );
        t = v * ( 1 - s * ( 1 - f ) );

        switch( i ) {
                case 0:
                        r = v;
                        g = t;
                        b = p;
                        break;
                case 1:
                        r = q;
                        g = v;
                        b = p;
                        break;
                case 2:
                        r = p;
                        g = v;
                        b = t;
                        break;
                case 3:
                        r = p;
                        g = q;
                        b = v;
                        break;
                case 4:
                        r = t;
                        g = p;
                        b = v;
                        break;
                default:                // case 5:
                        r = v;
                        g = p;
                        b = q;
                        break;
        }

        r /= 256.0;
        g /= 256.0;
        b /= 256.0;
        return ( ((DWORD) b << 16) | ((DWORD) g << 8) | ((DWORD) r));
}

void __fastcall TForm1::PrepareDAT()
{
    FILE *f_des = fopen("depth.dat", "wb");

    for (int y=0;y<(Y_CELL_SIZE*Y_CELLS);y++)
    {
        buffer[y] = -9999.0;
    }

    for (int x=0;x<(X_CELL_SIZE*X_CELLS);x++)
    {
        fwrite(&buffer, sizeof(buffer), 1, f_des);
    }
    fclose(f_des);
}

void __fastcall TForm1::PrepareData()
{
    float latitude, longitude, depth, scale_x, scale_y, temp_x, temp_y;
    float depth_temp;

    int x,y;

    PrepareDAT();
    // Convert();

    scale_x = (float)X_CELL_SIZE / DDEG_PER_CELL;
    scale_y = (float)Y_CELL_SIZE / DDEG_PER_CELL;

    if (OpenDataFile())
    if (OpenDAT())
    {
        while (GetData(longitude, latitude, depth))
        {
            MSG msg;
      			if (PeekMessage (&msg, NULL, NULL, NULL, PM_REMOVE))
            {
				      TranslateMessage (&msg);
      				DispatchMessage (&msg);
			      }

            temp_x = ((longitude-18.00) * scale_x);
            temp_y = ((latitude-54.30)  * scale_y);

            x = (int) temp_x;
            y = (int) temp_y;

            if ((float)((x*2)+1)<(temp_x*2))
                x++;
            if ((float)((y*2)+1)<(temp_y*2))
                y++;

            y = Image1->Height-y;
            long offset = sizeof(depth_temp)*(((long)x*Y_CELLS*Y_CELL_SIZE)+(long)y);
            fseek(DATHandle, offset, SEEK_SET);
            fread(&depth_temp, sizeof(depth_temp), 1, DATHandle);


            if (!(depth<0.0))
            {
                if (!(depth_temp<0.0))
                {
                    depth_temp = (depth+depth_temp) / 2.0;
                }
                else
                    depth_temp = depth;

                fseek(DATHandle, -1*sizeof(depth_temp), SEEK_CUR);
                fwrite(&depth_temp, sizeof(depth_temp), 1, DATHandle);
            }

            /*
            // for word
            if (!(depth<0.0))
            {
                if (!(depth_temp<0))
                {
                    depth_temp = ((WORD)depth+depth_temp) / 2;
                }
                else
                    depth_temp = (WORD) depth;

                fseek(DATHandle, -1*sizeof(depth_temp), SEEK_CUR);
                fwrite(&depth_temp, sizeof(depth_temp), 1, DATHandle);
            }
            */

            if (!(depth_temp<0.0))
            {
                Image1->Canvas->Pixels[x][y] =
                    HSLRangeToRGB(144,240,240 - (BYTE) depth_temp);
            }
            else
                Image1->Canvas->Pixels[x][y] = clOlive;

        }
        fclose(DATHandle);
        fclose(FileHandle);

    }

//    Image1->Picture->SaveToFile("Test.bmp");
}

bool __fastcall TForm1::OpenDAT()
{
    DATHandle = fopen("depth.dat", "r+b");
    if (DATHandle)
    {
        if (Form2==NULL)
        {
            file_size = filesize(DATHandle);
            Form2  = new TForm2(this);
            Form2->Show();
        }
        return true;
    }
    else
        return false;
}

void __fastcall TForm1::GetDATBuffer()
{
    fread(&buffer, sizeof(buffer), 1, DATHandle);
    curr_pos = ftell(DATHandle);
    Form2->ProgressBar1->Position = (int) ((100*curr_pos)/file_size);
}

float __fastcall TForm1::GetDepthAt(int x, int y)
{
    long offset = 4*(((long)x*Y_CELLS*Y_CELL_SIZE)+(long)y);
    fseek(DATHandle, offset, SEEK_SET);
    float depth_temp;
    fread(&depth_temp, sizeof(depth_temp), 1, DATHandle);
    if (depth_temp<0.0)
        depth_temp = 0.3;
    else
        depth_temp *= -1.0;
    return (depth_temp);
}

void __fastcall TForm1::Shade()
{
    float pi, D, H, A, sx, sy, sz, a, b, S, Nx, Ny, Nz, shade,
        P1d, P2d, P3d, P4d;

    long progress;

    // FILE *f_des = fopen("test.txt", "wt");

    if (OpenDAT())
    {
        if (Form2==NULL)
        {
            file_size = filesize(FileHandle);
            Form2  = new TForm2(this);
            Form2->Show();
        }

        pi = 3.14159265358979;
        D = 1.0; // interval of grid
        H = pi/3; // angle of Sun elevation
        A = pi/2; // azimuth of Sun measured from the south in clockwise

        // sunlight vector
        sx = sin(A)*cos(H);
        sy = cos(A)*cos(H);
     	  sz = -1.0 * sin(H);

        // int maxX = (X_CELL_SIZE*X_CELLS)-1;
        // int maxY = (Y_CELL_SIZE*Y_CELLS)-1;
        int maxX = (X_CELL_SIZE*X_CELLS)-1;
        int maxY = (Y_CELL_SIZE*Y_CELLS)-1;

        int minX = 1;
        int minY = 1;

        for (int i=minX;i<maxX;i++)
            for (int j=minY;j<maxY;j++)
            {
                MSG msg;
      			    if (PeekMessage (&msg, NULL, NULL, NULL, PM_REMOVE))
                {
      				    TranslateMessage (&msg);
      				    DispatchMessage (&msg);
      			    }

                long progress;
                progress = (100*(long)i*(long)maxY+(long)j)/((long)maxX*(long)maxY);
                Form2->ProgressBar1->Position = (int) progress;

                P1d = GetDepthAt(i+1,j);
                P2d = GetDepthAt(i-1,j);
                P3d = GetDepthAt(i,j+1);
                P4d = GetDepthAt(i,j-1);

                a = (P1d-P2d) / (2.0*D);
                b = (P3d-P4d) / (2.0*D);

                S = sqrt((a*a)+(b*b)+1.0);

                Nx = a/S;
                Ny = b/S;
                Nz = (-1.0)/S;
                shade = (Nx*sx) + (Ny*sy) + (Nz*sz);

                if (shade<0.0) shade = 0.0;
                Image1->Canvas->Pixels[i][j] =
                    HSLtoRGB(0.999,0.0, shade);

                /*
                AnsiString test;
                test = "X= "+IntToStr(i)+"  Y= "+IntToStr(j)+"  Shade= "+
                    FloatToStr(shade)+"\n";
                fputs(test.c_str(),f_des);
                */

            }
         // fclose(f_des);
         fclose(DATHandle);
         delete Form2;
         Form2 = NULL;
         InfoForm = new TInfoForm(this);
         InfoForm->Show();
    }
}

void __fastcall TForm1::Draw()
{
    float depth;
    if (OpenDAT())
    {
        for (int x=0;x<(X_CELL_SIZE*X_CELLS);x++)
        {
            GetDATBuffer();
            for (int y=0;y<(Y_CELL_SIZE*Y_CELLS);y++)
            {
                MSG msg;
      			    if (PeekMessage (&msg, NULL, NULL, NULL, PM_REMOVE))
                {
      				    TranslateMessage (&msg);
      				    DispatchMessage (&msg);
      			    }

                depth = buffer[y];
                if (!(depth<0.0))
                {
                    Image1->Canvas->Pixels[x][y] =
                        HSLRangeToRGB(144,240,240 - (BYTE) (depth*1.8));
                }
                else
                    Image1->Canvas->Pixels[x][y] = clOlive;
            }
        }
        fclose(DATHandle);
        delete Form2;
        Form2 = NULL;
        InfoForm = new TInfoForm(this);
        InfoForm->Show();
    }
}

void __fastcall TForm1::Convert()
{
    FILE *f_src = fopen("depth.txt", "rt");
    FILE *f_des = fopen("depth1.txt", "wt");

    char line[255];

    while (!feof(f_src))
    {

        fgets(line, 255, f_src);
        AnsiString str = AnsiString(line);
        // int coma = str.Pos(',');
        AnsiString first = str.SubString(1,str.Pos(' ')-1);
        str = str.SubString(str.Pos(' ')+1,str.Length());
        fputs(first.c_str(),f_des);
        fputs("\n",f_des);
        fputs(str.c_str(),f_des);

    }

    fclose(f_src);
    fclose(f_des);
}

bool __fastcall TForm1::OpenDataFile()
{
    FileHandle = fopen("depth1.txt", "rt");
    if (FileHandle)
    {
        if (Form2==NULL)
        {
            file_size = filesize(FileHandle);
            Form2  = new TForm2(this);
            Form2->Show();
        }
        return true;
    }
    else
        return false;
}

bool __fastcall TForm1::GetData(float &longitude, float &latitude, float &depth)
{
    char line[255];

    if (!feof(FileHandle))
    {
        // line[0] = '\0';
        fgets(line, 255, FileHandle);
        AnsiString str = AnsiString(line);
        // int coma = str.Pos(',');
        AnsiString value = str.SubString(1,str.Pos(',')-1);
        longitude = StrToFloat(value);
        str = str.SubString(str.Pos(',')+1,str.Length());
        value = str.SubString(1,str.Pos(',')-1);
        latitude = StrToFloat(value);
        str = str.SubString(str.Pos(',')+1,str.Length());
        value = str.SubString(1,str.Length()-1);
        depth = StrToFloat(value);

        curr_pos = ftell(FileHandle);
        double pos_temp = (100*(double) curr_pos)/(double) file_size;
        Form2->ProgressBar1->Position = (int) pos_temp;

        return true;
    }
    else
    {
        delete Form2;
        Form2 = NULL;
        return false;
    }
}


//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
    : TForm(Owner)
{
    // _control87(MCW_EM, MCW_EM);
    DecimalSeparator = '.';
    mouse_down = false;
}

//---------------------------------------------------------------------------
void __fastcall TForm1::FormCreate(TObject *Sender)
{
    Form1->ClientWidth = 711;
    Form1->ClientHeight = 420;

    Image1->Height =  Y_CELLS * Y_CELL_SIZE;
    Image1->Width  =  X_CELLS * X_CELL_SIZE;

/*
    Graphics::TBitmap *Bitmap;
    Bitmap = new Graphics::TBitmap;
    Bitmap->PixelFormat = pf24bit;
    Bitmap->Width  = Image1->Width;
    Bitmap->Height = Image1->Height;

    Image1->Picture->Graphic = Bitmap;
*/

//    delete Bitmap;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormShow(TObject *Sender)
{
    // PrepareData();
    // Draw();
    Shade();
    // Image1->Picture->SaveToFile("Test.bmp");
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Image1MouseMove(TObject *Sender, TShiftState Shift,
      int X, int Y)
{
    float deg, depth, distance;
    long offset;
    AnsiString str;
    char str_temp[20];

    deg = 54.3 +((((float)Y_CELLS*(float)Y_CELL_SIZE)
        -(float)Y)/((float)Y_CELL_SIZE/(float)DDEG_PER_CELL));

    str = FormatFloat("00.000",deg)+" ddeg ";
    InfoForm->LatText->Caption  = str;

    deg = 18.0 +((float)X/((float)X_CELL_SIZE/(float)DDEG_PER_CELL));
    str = FormatFloat("00.000",deg)+" ddeg ";
    InfoForm->LongText->Caption = str;

    str = "???";
    DATHandle = fopen("depth.dat", "r+b");
    if (DATHandle)
    {
        offset = 4*(((long)X*(long)Y_CELLS*(long)Y_CELL_SIZE)+(long)Y);
        fseek(DATHandle, offset, SEEK_SET);
        fread(&depth, sizeof(depth), 1, DATHandle);
        if (!(depth<0))
        {
            sprintf(str_temp, "% 6.3f",depth);
            str = str_temp;
        }
        else
            str = "  0.000";
        fclose(DATHandle);
    }
    InfoForm->DepthText->Caption = str+" m ";

    InfoForm->BringToFront();

    // draw line if mousee button is on
    if (mouse_down)
    {
        Image1->Canvas->Pen->Color = clBlack;
        Image1->Canvas->Pen->Style = psSolid;
        Image1->Canvas->Pen->Mode = pmNotXor;
        Image1->Canvas->MoveTo(begX, begY);
        Image1->Canvas->LineTo(endX, endY);

        Image1->Canvas->MoveTo(endX, endY);
        Image1->Canvas->LineTo(endX, endY-5);
        Image1->Canvas->MoveTo(endX, endY);
        Image1->Canvas->LineTo(endX, endY+5);
        Image1->Canvas->MoveTo(endX, endY);
        Image1->Canvas->LineTo(endX-5, endY);
        Image1->Canvas->MoveTo(endX, endY);
        Image1->Canvas->LineTo(endX+5, endY);

        Image1->Canvas->Pen->Color = clBlack;
        Image1->Canvas->Pen->Style = psSolid;
        Image1->Canvas->Pen->Mode = pmNotXor;
        Image1->Canvas->MoveTo(begX, begY);
        Image1->Canvas->LineTo(X, Y);
        endX = X;
        endY = Y;
        Image1->Canvas->MoveTo(endX, endY);
        Image1->Canvas->LineTo(endX, endY-5);
        Image1->Canvas->MoveTo(endX, endY);
        Image1->Canvas->LineTo(endX, endY+5);
        Image1->Canvas->MoveTo(endX, endY);
        Image1->Canvas->LineTo(endX-5, endY);
        Image1->Canvas->MoveTo(endX, endY);
        Image1->Canvas->LineTo(endX+5, endY);

        distance = sqrt(((float)endX-(float)begX)*((float)endX-(float)begX)
            +((float)endY-(float)begY)*((float)endY-(float)begY));
        distance *= KM_PER_PIX;
        sprintf(str_temp, "% 6.3f",distance);
        str = str_temp;
        InfoForm->DistanceText->Caption = str+" km ";
    }

}
//---------------------------------------------------------------------------

void __fastcall TForm1::Image1MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if (Button==mbLeft)
    {
        if (!(mouse_down))
        {
            endX = X;
            endY = Y;
        }
        mouse_down = true;
        begX = X;
        begY = Y;
        // draw cross
        Image1->Canvas->Pen->Color = clBlack;
        Image1->Canvas->Pen->Style = psSolid;
        Image1->Canvas->Pen->Mode = pmNotXor;
        Image1->Canvas->MoveTo(begX, begY);
        Image1->Canvas->LineTo(begX, begY-5);
        Image1->Canvas->MoveTo(begX, begY);
        Image1->Canvas->LineTo(begX, begY+5);
        Image1->Canvas->MoveTo(begX, begY);
        Image1->Canvas->LineTo(begX-5, begY);
        Image1->Canvas->MoveTo(begX, begY);
        Image1->Canvas->LineTo(begX+5, begY);
    }

}
//---------------------------------------------------------------------------

void __fastcall TForm1::Image1MouseUp(TObject *Sender, TMouseButton Button,
      TShiftState Shift, int X, int Y)
{
    if ((mouse_down) && (Button==mbLeft))
    {
        mouse_down = false;
        Image1->Canvas->Pen->Color = clBlack;
        Image1->Canvas->Pen->Style = psSolid;
        Image1->Canvas->Pen->Mode = pmNotXor;
        Image1->Canvas->MoveTo(begX, begY);
        Image1->Canvas->LineTo(endX, endY);

        Image1->Canvas->MoveTo(endX, endY);
        Image1->Canvas->LineTo(endX, endY-5);
        Image1->Canvas->MoveTo(endX, endY);
        Image1->Canvas->LineTo(endX, endY+5);
        Image1->Canvas->MoveTo(endX, endY);
        Image1->Canvas->LineTo(endX-5, endY);
        Image1->Canvas->MoveTo(endX, endY);
        Image1->Canvas->LineTo(endX+5, endY);
    }
}
//---------------------------------------------------------------------------


