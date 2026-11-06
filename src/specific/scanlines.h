#ifndef SCANLINES_H
#define SCANLINES_H

extern "C" {

extern unsigned short *Output;
extern unsigned short *TextureHandle;
extern long bytepitch;
extern long *YTable;
extern float *WidthTable;
extern long *WidthTablef;

void ScanLinesTAffine(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesT(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesTF(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesTFAffine(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);

}

#endif
