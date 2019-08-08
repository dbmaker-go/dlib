
#include "dface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char **argv)
{
	int rc = 0, i;
	dface dh;
	facebox fbox;
	facevector fvec;
	char imgfn[256];
	time_t t1,t2;
	
	if (argc <= 1){
		printf("%s image_file\n", argv[0]);
		return 0;
	}
	strcpy(imgfn, argv[1]);

	t1 = time(0);
	if (rc = OpenDface(&dh, DFACE_SHAPE68)) {
		printf("open dface engine error: %d\n", rc);
		return 0;
	}
	t2 = time(0);
	printf("load dface engine costs %f seconds\n", difftime(t2,t1));
	
	t1 = time(0);
	if (rc = GetFaceBox(dh, imgfn, &fbox)){
		printf("get face box error: %d\n", rc);
		goto cleanup;
	}
	t2 = time(0);
	printf("get face box costs %f seconds\n", difftime(t2,t1));
	
	printf("got face at [%d, %d, %d, %d]\n", fbox.top, fbox.left, 
			fbox.top+fbox.width, fbox.left+fbox.height);
	
	t1 = time(0);
	if (rc = GetFaceVector(dh, imgfn, &fvec)) {
		printf("get face vector error: %d\n", rc);
		goto cleanup;
	}
	t2 = time(0);
	printf("get face vector costs %f seconds\n", difftime(t2,t1));
	
	printf("got face vector:\n");
	for (i=0; i<128; i++) {
		printf("%.15f, ", fvec[i]);
		if ((i+1) % 8 == 0)
			printf("\n");
	}
	printf("\n");

cleanup:
	if (rc = CloseDface(dh)){
		printf("close dface engine error: %d\n", rc);
	}

	return 0;
}

