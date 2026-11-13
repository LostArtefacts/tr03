extern "C" {

#include "../3dglodef.h"

typedef struct vertex_info {
    float x, y;
    float ooz;
    float u, v, g;
    int vr, vg, vb;
} VERTEX_INFO;

extern float phd_leftfloat, phd_topfloat, phd_rightfloat, phd_bottomfloat;

int XYClipper(int number, VERTEX_INFO *input)
{
    int i, j;
    VERTEX_INFO output[20], *A, *B;
    float clipper;

    B = &input[number - 1];
    for (i = j = 0; i < number; i++) {
        A = B;
        B = &input[i];
        if (A->x < phd_leftfloat) {
            if (B->x < phd_leftfloat)
                continue;
            clipper = (phd_leftfloat - B->x) / (A->x - B->x);
            output[j].x = phd_leftfloat;
            output[j++].y = B->y + (A->y - B->y) * clipper;
        } else if (A->x > phd_rightfloat) {
            if (B->x > phd_rightfloat)
                continue;
            clipper = (phd_rightfloat - B->x) / (A->x - B->x);
            output[j].x = phd_rightfloat;
            output[j++].y = B->y + (A->y - B->y) * clipper;
        }
        if (B->x < phd_leftfloat) {
            clipper = (phd_leftfloat - B->x) / (A->x - B->x);
            output[j].x = phd_leftfloat;
            output[j++].y = B->y + (A->y - B->y) * clipper;
        } else if (B->x > phd_rightfloat) {
            clipper = (phd_rightfloat - B->x) / (A->x - B->x);
            output[j].x = phd_rightfloat;
            output[j++].y = B->y + (A->y - B->y) * clipper;
        } else {
            output[j].x = B->x;
            output[j++].y = B->y;
        }
    }
    if (j < 3)
        return 0;
    number = j;
    B = &output[number - 1];
    for (i = j = 0; i < number; i++) {
        A = B;
        B = &output[i];
        if (A->y < phd_topfloat) {
            if (B->y < phd_topfloat)
                continue;
            clipper = (phd_topfloat - B->y) / (A->y - B->y);
            input[j].x = B->x + (A->x - B->x) * clipper;
            input[j++].y = phd_topfloat;
        } else if (A->y > phd_bottomfloat) {
            if (B->y > phd_bottomfloat)
                continue;
            clipper = (phd_bottomfloat - B->y) / (A->y - B->y);
            input[j].x = B->x + (A->x - B->x) * clipper;
            input[j++].y = phd_bottomfloat;
        }
        if (B->y < phd_topfloat) {
            clipper = (phd_topfloat - B->y) / (A->y - B->y);
            input[j].x = B->x + (A->x - B->x) * clipper;
            input[j++].y = phd_topfloat;
        } else if (B->y > phd_bottomfloat) {
            clipper = (phd_bottomfloat - B->y) / (A->y - B->y);
            input[j].x = B->x + (A->x - B->x) * clipper;
            input[j++].y = phd_bottomfloat;
        } else {
            input[j].x = B->x;
            input[j++].y = B->y;
        }
    }
    if (j < 3)
        return 0;
    return j;
}


#define MAGICNUM float((1 << 26) * ((1 << 26) * 1.5))
#define FTOL(x) ((fconv = (x) + (MAGICNUM)), *(int *)(&fconv))

static VERTEX_INFO XYG_output[8];

int XYGClipper(int n, VERTEX_INFO *in)
{
    VERTEX_INFO *v1;
    VERTEX_INFO *v2;
    float clipper;
    int nPoints;
    int i;
    double fconv;

    v2 = &in[n - 1];
    nPoints = 0;
    for (i = 0; i < n; i++) {
        v1 = v2;
        v2 = &in[i];
        if (v1->x < phd_leftfloat) {
            if (v2->x < phd_leftfloat)
                continue;
            clipper = (phd_leftfloat - v2->x) / (v1->x - v2->x);
            XYG_output[nPoints].ooz = (v1->ooz - v2->ooz) * clipper + v2->ooz;
            XYG_output[nPoints].vr = FTOL(v2->vr + (v1->vr - v2->vr) * clipper);
            XYG_output[nPoints].vg = FTOL(v2->vg + (v1->vg - v2->vg) * clipper);
            XYG_output[nPoints].vb = FTOL(v2->vb + (v1->vb - v2->vb) * clipper);
            XYG_output[nPoints].x = phd_leftfloat;
            XYG_output[nPoints].y = (v1->y - v2->y) * clipper + v2->y;
            nPoints++;
        } else if (v1->x > phd_rightfloat) {
            if (v2->x > phd_rightfloat)
                continue;
            clipper = (phd_rightfloat - v2->x) / (v1->x - v2->x);
            XYG_output[nPoints].ooz = (v1->ooz - v2->ooz) * clipper + v2->ooz;
            XYG_output[nPoints].vr = FTOL(v2->vr + (v1->vr - v2->vr) * clipper);
            XYG_output[nPoints].vg = FTOL(v2->vg + (v1->vg - v2->vg) * clipper);
            XYG_output[nPoints].vb = FTOL(v2->vb + (v1->vb - v2->vb) * clipper);
            XYG_output[nPoints].x = phd_rightfloat;
            XYG_output[nPoints].y = (v1->y - v2->y) * clipper + v2->y;
            nPoints++;
        }
        if (v2->x < phd_leftfloat) {
            clipper = (phd_leftfloat - v2->x) / (v1->x - v2->x);
            XYG_output[nPoints].ooz = (v1->ooz - v2->ooz) * clipper + v2->ooz;
            XYG_output[nPoints].vr = FTOL(v2->vr + (v1->vr - v2->vr) * clipper);
            XYG_output[nPoints].vg = FTOL(v2->vg + (v1->vg - v2->vg) * clipper);
            XYG_output[nPoints].vb = FTOL(v2->vb + (v1->vb - v2->vb) * clipper);
            XYG_output[nPoints].x = phd_leftfloat;
            XYG_output[nPoints].y = (v1->y - v2->y) * clipper + v2->y;
            nPoints++;
        } else if (v2->x > phd_rightfloat) {
            clipper = (phd_rightfloat - v2->x) / (v1->x - v2->x);
            XYG_output[nPoints].ooz = (v1->ooz - v2->ooz) * clipper + v2->ooz;
            XYG_output[nPoints].vr = FTOL(v2->vr + (v1->vr - v2->vr) * clipper);
            XYG_output[nPoints].vg = FTOL(v2->vg + (v1->vg - v2->vg) * clipper);
            XYG_output[nPoints].vb = FTOL(v2->vb + (v1->vb - v2->vb) * clipper);
            XYG_output[nPoints].x = phd_rightfloat;
            XYG_output[nPoints].y = (v1->y - v2->y) * clipper + v2->y;
            nPoints++;
        } else {
            XYG_output[nPoints].x = v2->x;
            XYG_output[nPoints].y = v2->y;
            XYG_output[nPoints].ooz = v2->ooz;
            XYG_output[nPoints].vr = v2->vr;
            XYG_output[nPoints].vg = v2->vg;
            XYG_output[nPoints].vb = v2->vb;
            nPoints++;
        }
    }
    if (nPoints < 3)
        return 0;
    n = nPoints;
    v2 = &XYG_output[n - 1];
    nPoints = 0;
    for (i = 0; i < n; i++) {
        v1 = v2;
        v2 = &XYG_output[i];
        if (v1->y < phd_topfloat) {
            if (v2->y < phd_topfloat)
                continue;
            clipper = (phd_topfloat - v2->y) / (v1->y - v2->y);
            in[nPoints].ooz = (v1->ooz - v2->ooz) * clipper + v2->ooz;
            in[nPoints].vr = FTOL(v2->vr + (v1->vr - v2->vr) * clipper);
            in[nPoints].vg = FTOL(v2->vg + (v1->vg - v2->vg) * clipper);
            in[nPoints].vb = FTOL(v2->vb + (v1->vb - v2->vb) * clipper);
            in[nPoints].x = (v1->x - v2->x) * clipper + v2->x;
            in[nPoints].y = phd_topfloat;
            nPoints++;
        } else if (v1->y > phd_bottomfloat) {
            if (v2->y > phd_bottomfloat)
                continue;
            clipper = (phd_bottomfloat - v2->y) / (v1->y - v2->y);
            in[nPoints].ooz = (v1->ooz - v2->ooz) * clipper + v2->ooz;
            in[nPoints].vr = FTOL(v2->vr + (v1->vr - v2->vr) * clipper);
            in[nPoints].vg = FTOL(v2->vg + (v1->vg - v2->vg) * clipper);
            in[nPoints].vb = FTOL(v2->vb + (v1->vb - v2->vb) * clipper);
            in[nPoints].x = (v1->x - v2->x) * clipper + v2->x;
            in[nPoints].y = phd_bottomfloat;
            nPoints++;
        }
        if (v2->y < phd_topfloat) {
            clipper = (phd_topfloat - v2->y) / (v1->y - v2->y);
            in[nPoints].ooz = (v1->ooz - v2->ooz) * clipper + v2->ooz;
            in[nPoints].vr = FTOL(v2->vr + (v1->vr - v2->vr) * clipper);
            in[nPoints].vg = FTOL(v2->vg + (v1->vg - v2->vg) * clipper);
            in[nPoints].vb = FTOL(v2->vb + (v1->vb - v2->vb) * clipper);
            in[nPoints].x = (v1->x - v2->x) * clipper + v2->x;
            in[nPoints].y = phd_topfloat;
            nPoints++;
        } else if (v2->y > phd_bottomfloat) {
            clipper = (phd_bottomfloat - v2->y) / (v1->y - v2->y);
            in[nPoints].ooz = (v1->ooz - v2->ooz) * clipper + v2->ooz;
            in[nPoints].vr = FTOL(v2->vr + (v1->vr - v2->vr) * clipper);
            in[nPoints].vg = FTOL(v2->vg + (v1->vg - v2->vg) * clipper);
            in[nPoints].vb = FTOL(v2->vb + (v1->vb - v2->vb) * clipper);
            in[nPoints].x = (v1->x - v2->x) * clipper + v2->x;
            in[nPoints].y = phd_bottomfloat;
            nPoints++;
        } else {
            in[nPoints].x = v2->x;
            in[nPoints].y = v2->y;
            in[nPoints].ooz = v2->ooz;
            in[nPoints].ooz = v2->ooz;
            in[nPoints].vr = v2->vr;
            in[nPoints].vg = v2->vg;
            in[nPoints].vb = v2->vb;
            nPoints++;
        }
    }
    if (nPoints < 3)
        return 0;
    return nPoints;
}

}

#ifdef TR3_EXE
extern "C" {

float one = 33554432.0F;

}
#endif
