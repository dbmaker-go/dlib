
#include "dface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
	int rc = 0, i;
	dface dh;
	facebox fbox;
	facevector fvec;
	char imgfn1[256], imgfn2[256];
	double dist;
	
	if (argc <= 2){
		printf("%s image_file1 image_file2\n", argv[0]);
		return 0;
	}
	strcpy(imgfn1, argv[1]);
	strcpy(imgfn2, argv[2]);

	if (rc = OpenDface(&dh, DFACE_SHAPE68)) {
		printf("open dface engine error: %d\n", rc);
		return 0;
	}
	
	if (rc = GetFaceBox(dh, imgfn1, &fbox)){
		printf("get face box error: %d\n", rc);
		goto cleanup;
	}
	
	printf("got face1 at [%d, %d, %d, %d]\n", fbox.top, fbox.left, 
			fbox.top+fbox.width, fbox.left+fbox.height);
	
	if (rc = GetFaceBox(dh, imgfn2, &fbox)){
		printf("get face box error: %d\n", rc);
		goto cleanup;
	}
	
	printf("got face2 at [%d, %d, %d, %d]\n", fbox.top, fbox.left, 
			fbox.top+fbox.width, fbox.left+fbox.height);
	
	if (rc = GetFaceVector(dh, imgfn1, &fvec)) {
		printf("get face1 vector error: %d\n", rc);
		goto cleanup;
	}
	
	printf("got face1 vector:\n");
	for (i=0; i<128; i++) {
		printf("%lf, ", fvec[i]);
		if ((i+1) % 8 == 0)
			printf("\n");
	}
	printf("\n");
	
	if (rc = GetFaceVector(dh, imgfn2, &fvec)) {
		printf("get face2 vector error: %d\n", rc);
		goto cleanup;
	}
	
	printf("got face2 vector:\n");
	for (i=0; i<128; i++) {
		printf("%lf, ", fvec[i]);
		if ((i+1) % 8 == 0)
			printf("\n");
	}
	printf("\n");
	
	if (rc = GetFaceDistance(dh, imgfn1, imgfn2, &dist)) {
		printf("get face distance error: %d\n", rc);
		goto cleanup;
	}
	
	printf("distance = %lf\n", dist);

cleanup:
	if (rc = CloseDface(dh)){
		printf("close dface engine error: %d\n", rc);
	}

	return 0;
}

