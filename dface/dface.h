#ifndef __DFACE_H
#define __DFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#define DFACE_OK (0)
#define DFACE_ERR (1)
#define DFACE_ERR_NO_FACE   (100)
#define DFACE_ERR_SHAPE     (101)

// shape predict face landmarks type:
#define DFACE_SHAPE5  (5)
#define DFACE_SHAPE68 (68)

typedef void * dface;

typedef struct _face_box {
	int top;
	int left;
	int width;
	int height;
}facebox;

typedef double facevector[128];

typedef struct _point{
	int X;
	int Y;
}shapepoint;


int OpenDface(dface *handle, int shapemode);
int CloseDface(dface handle);
int GetFaceBox(dface dh, char *imgfn, facebox *box);
int GetFaceShape(dface dh, char *imgfn, int shapemode, shapepoint *shape);
int GetFaceVector(dface dh, char *imgfn, facevector *vector);
int GetFaceDistance(dface dh, char *img1fn, char *img2fn, double *distance);

int DfaceGetVector(dface dh, char *imgbuf, facevector *vector);

#ifdef __cplusplus
}
#endif

#endif // __HFACE_H
