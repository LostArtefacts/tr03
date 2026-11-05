#include "standard.h"
#include "specific.h"

extern "C" {

extern int CurPicTexIndices[5];
extern float f_zfar;
extern int nLoadedPictures;

void TRDrawPicture(int col, int *indices, float zpos);
void FadePictureDown(int steps);
void FreePictureTextures(int *indices);

void DrawMonoScreen(int r, int g, int b)
{
    TRDrawPicture(0, CurPicTexIndices, f_zfar);
}

}
