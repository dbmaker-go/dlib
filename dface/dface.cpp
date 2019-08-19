/*
 * C wrapper for dlib
 */

#include "dface.h"

#include <dlib/dnn.h>
//#include <dlib/gui_widgets.h>
#include <dlib/clustering.h>
#include <dlib/string.h>
#include <dlib/image_io.h>
#include <dlib/image_processing/frontal_face_detector.h>

using namespace dlib;
using namespace std;

// ----------------------------------------------------------------------------------------

// The next bit of code defines a ResNet network.  It's basically copied
// and pasted from the dnn_imagenet_ex.cpp example, except we replaced the loss
// layer with loss_metric and made the network somewhat smaller.  Go read the introductory
// dlib DNN examples to learn what all this stuff means.
//
// Also, the dnn_metric_learning_on_images_ex.cpp example shows how to train this network.
// The dlib_face_recognition_resnet_model_v1 model used by this example was trained using
// essentially the code shown in dnn_metric_learning_on_images_ex.cpp except the
// mini-batches were made larger (35x15 instead of 5x5), the iterations without progress
// was set to 10000, and the training dataset consisted of about 3 million images instead of
// 55.  Also, the input layer was locked to images of size 150.
template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual = add_prev1<block<N,BN,1,tag1<SUBNET>>>;

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual_down = add_prev2<avg_pool<2,2,2,2,skip1<tag2<block<N,BN,2,tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET> 
using block  = BN<con<N,3,3,1,1,relu<BN<con<N,3,3,stride,stride,SUBNET>>>>>;

template <int N, typename SUBNET> using ares      = relu<residual<block,N,affine,SUBNET>>;
template <int N, typename SUBNET> using ares_down = relu<residual_down<block,N,affine,SUBNET>>;

template <typename SUBNET> using alevel0 = ares_down<256,SUBNET>;
template <typename SUBNET> using alevel1 = ares<256,ares<256,ares_down<256,SUBNET>>>;
template <typename SUBNET> using alevel2 = ares<128,ares<128,ares_down<128,SUBNET>>>;
template <typename SUBNET> using alevel3 = ares<64,ares<64,ares<64,ares_down<64,SUBNET>>>>;
template <typename SUBNET> using alevel4 = ares<32,ares<32,ares<32,SUBNET>>>;

using anet_type = loss_metric<fc_no_bias<128,avg_pool_everything<
                            alevel0<
                            alevel1<
                            alevel2<
                            alevel3<
                            alevel4<
                            max_pool<3,3,2,2,relu<affine<con<32,7,7,2,2,
                            input_rgb_image_sized<150>
                            >>>>>>>>>>>>;

// ----------------------------------------------------------------------------------------

std::vector<matrix<rgb_pixel>> jitter_image(
    const matrix<rgb_pixel>& img
);

typedef struct _dface_t {
	void *detector;
	void *sp;
	void *net;
	int  shapemode;
}hdface;

#if defined(WIN32) || defined(_WIN64)
# define PATHSEP '\\'
#else
# define PATHSEP '/'
#endif

// ----------------------------------------------------------------------------------------

int OpenDface(dface *h, int shapemode, char *modpath) try
{
	frontal_face_detector *detector = new frontal_face_detector();
	*detector = get_frontal_face_detector();
	shape_predictor *sp = new shape_predictor();
	char modfile[1024];
	if (modpath){
		sprintf(modfile, "%s%c", modpath, PATHSEP);
	} else {
		modfile[0] = 0;
	}
	if (shapemode == DFACE_SHAPE5){
		strcat(modfile, "shape_predictor_5_face_landmarks.dat");
	    //deserialize("shape_predictor_5_face_landmarks.dat") >> *sp;
	    deserialize(modfile) >> *sp;
	} else if (shapemode == DFACE_SHAPE68){
		strcat(modfile, "shape_predictor_68_face_landmarks.dat");
		//deserialize("shape_predictor_68_face_landmarks.dat") >> *sp;
		deserialize(modfile) >> *sp;
	} else {
		delete detector;
		delete sp;
		return DFACE_ERR_SHAPE;
	}
	
	if (modpath){
		sprintf(modfile, "%s%c", modpath, PATHSEP);
	} else {
		modfile[0] = 0;
	}
	strcat(modfile, "dlib_face_recognition_resnet_model_v1.dat");
	
    anet_type *net = new anet_type();
	//deserialize("dlib_face_recognition_resnet_model_v1.dat") >> *net;
	deserialize(modfile) >> *net;
 
 	hdface *handle = new hdface();	   
    handle->detector = (void *)detector;
    handle->sp = (void *)sp;
    handle->net = (void *)net;
    handle->shapemode = shapemode;
    
    *h = (dface)handle;
    return DFACE_OK;
}
catch (std::exception& e)
{
    cout << e.what() << endl;
    return DFACE_ERR;
}

int CloseDface(dface h) try
{
	hdface *handle = (hdface *)h;
	frontal_face_detector *detector = (frontal_face_detector *)handle->detector;
	shape_predictor *sp = (shape_predictor *)handle->sp;
	anet_type *net = (anet_type *)handle->net;
	
	delete net;
	delete sp;
	delete detector;
	delete handle;
	return DFACE_OK;
}
catch (std::exception& e)
{
    cout << e.what() << endl;
    return DFACE_ERR;
}

int GetFaceBox(dface h, char *imgfn, facebox *box) try
{
	hdface *handle = (hdface *)h;
	frontal_face_detector *detector = (frontal_face_detector *)handle->detector;
	matrix<rgb_pixel> img;
	load_image(img, imgfn);
	std::vector<rectangle> dets = (*detector)(img);
	if (dets.size() > 0){
		box->left = dets[0].left();
		box->top = dets[0].top();
		box->width = dets[0].width();
		box->height = dets[0].height();
	} else {
		return DFACE_ERR_NO_FACE;
	}
	return DFACE_OK;
}
catch (std::exception& e)
{
    cout << e.what() << endl;
    return DFACE_ERR;
}

int GetFaceShape(dface dh, char *imgfn, int shapemode, shapepoint *shape) try
{
	hdface *handle = (hdface *)dh;
	if (handle->shapemode != shapemode){
		return DFACE_ERR_SHAPE;
	}
	frontal_face_detector *detector = (frontal_face_detector *)handle->detector;
	shape_predictor *sp = (shape_predictor *)handle->sp;
	
	matrix<rgb_pixel> img;
	load_image(img, imgfn);
	std::vector<rectangle> dets = (*detector)(img);
	if (dets.size() <= 0){
		return DFACE_ERR_NO_FACE;
	}
	
	full_object_detection tshape = (*sp)(img, dets[0]);
	for (int i=0; i<shapemode; i++){
		shape[i].X = tshape.part(i).x();
		shape[i].Y = tshape.part(i).y();
	}
	
	return DFACE_OK;
}
catch (std::exception& e)
{
    cout << e.what() << endl;
    return DFACE_ERR;
}


static int getVector(dface h, char *imgfn, matrix<float,0,1> *vec) try
{
	hdface *handle = (hdface *)h;
	frontal_face_detector *detector = (frontal_face_detector *)handle->detector;
	shape_predictor *sp = (shape_predictor *)handle->sp;
	anet_type *net = (anet_type *)handle->net;
	
	matrix<rgb_pixel> img;
    load_image(img, imgfn);
    
    std::vector<rectangle> dets = (*detector)(img);
    if (dets.size() <= 0) {
    	return DFACE_ERR_NO_FACE;
    }
    
    auto shape = (*sp)(img, dets[0]);
    matrix<rgb_pixel> face_chip;
    extract_image_chip(img, get_face_chip_details(shape,150,0.25), face_chip);
    
    std::vector<matrix<rgb_pixel>> faces;
    faces.push_back(move(face_chip));
    
    std::vector<matrix<float,0,1>> face_descriptors = (*net)(faces);
    if (face_descriptors.size() <= 0){
    	return DFACE_ERR_NO_FACE;
    }
    
    *vec = face_descriptors[0];
    
    return DFACE_OK;
}
catch (std::exception& e)
{
    cout << e.what() << endl;
    return DFACE_ERR;
}

int GetFaceVector(dface h, char *imgfn, facevector *vector) try
{
	hdface *handle = (hdface *)h;
	frontal_face_detector *detector = (frontal_face_detector *)handle->detector;
	shape_predictor *sp = (shape_predictor *)handle->sp;
	anet_type *net = (anet_type *)handle->net;
	
	matrix<rgb_pixel> img;
    load_image(img, imgfn);
    
    std::vector<rectangle> dets = (*detector)(img);
    if (dets.size() <= 0) {
    	return DFACE_ERR_NO_FACE;
    }
    
    auto shape = (*sp)(img, dets[0]);
    matrix<rgb_pixel> face_chip;
    extract_image_chip(img, get_face_chip_details(shape,150,0.25), face_chip);
    
    std::vector<matrix<rgb_pixel>> faces;
    faces.push_back(move(face_chip));
    
    std::vector<matrix<float,0,1>> face_descriptors = (*net)(faces);
    if (face_descriptors.size() <= 0){
    	return DFACE_ERR_NO_FACE;
    }
    for (int i=0; i<128; i++){
    	(*vector)[i] = face_descriptors[0](i);
    }
    return DFACE_OK;
}
catch (std::exception& e)
{
    cout << e.what() << endl;
    return DFACE_ERR;
}

int DfaceGetVector(dface h, char *imgbuf, facevector *vector) try
{
	hdface *handle = (hdface *)h;
	frontal_face_detector *detector = (frontal_face_detector *)handle->detector;
	shape_predictor *sp = (shape_predictor *)handle->sp;
	anet_type *net = (anet_type *)handle->net;
	
	// how to read img from a buffer ???
	matrix<rgb_pixel> img;
    load_image(img, imgbuf);
    
    std::vector<rectangle> dets = (*detector)(img);
    if (dets.size() <= 0) {
    	return DFACE_ERR_NO_FACE;
    }
    
    auto shape = (*sp)(img, dets[0]);
    matrix<rgb_pixel> face_chip;
    extract_image_chip(img, get_face_chip_details(shape,150,0.25), face_chip);
    
    std::vector<matrix<rgb_pixel>> faces;
    faces.push_back(move(face_chip));
    
    std::vector<matrix<float,0,1>> face_descriptors = (*net)(faces);
    if (face_descriptors.size() <= 0){
    	return DFACE_ERR_NO_FACE;
    }
    for (int i=0; i<128; i++){
    	(*vector)[i] = face_descriptors[0](i);
    }
    return DFACE_OK;
}
catch (std::exception& e)
{
    cout << e.what() << endl;
    return DFACE_ERR;
}

int GetFaceDistance(dface dh, char *img1fn, char *img2fn, double *distance) try
{
	matrix<float,0,1> vec1, vec2;
	int rc = 0;
	
	if (rc = getVector(dh, img1fn, &vec1)){
		return rc;	
	}
	
	if (rc = getVector(dh, img2fn, &vec2)){
		return rc;	
	}

	*distance = length(vec1 - vec2);
	
	return DFACE_OK;
}
catch (std::exception& e)
{
    cout << e.what() << endl;
    return DFACE_ERR;
}


// ----------------------------------------------------------------------------------------

std::vector<matrix<rgb_pixel>> jitter_image(
    const matrix<rgb_pixel>& img
)
{
    // All this function does is make 100 copies of img, all slightly jittered by being
    // zoomed, rotated, and translated a little bit differently. They are also randomly
    // mirrored left to right.
    thread_local dlib::rand rnd;

    std::vector<matrix<rgb_pixel>> crops; 
    for (int i = 0; i < 100; ++i)
        crops.push_back(jitter_image(img,rnd));

    return crops;
}

// ----------------------------------------------------------------------------------------

