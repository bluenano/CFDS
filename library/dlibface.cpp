#include "pch.h"

#include <new>//placement new, call constructor at preallocated memory

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>

using namespace dlib;

template<class A, class B>
struct Fits
{
    enum{value = sizeof(A) >= sizeof(B) && alignof(A) >= alignof(B) };
};

typedef object_detector<scan_fhog_pyramid<pyramid_down<2> > > Ffd_2_1;
static_assert(Fits<DLibFaceDetector_2_1, Ffd_2_1>::value, "please increase size of byte array in header by a multiple of 16");

typedef dlib::shape_predictor Sp;
static_assert(Fits<DLibLandMarkDetector, Sp>::value, "please increase size of byte array in header by a multiple of 16");


#ifdef WANT_6_5
//typedef object_detector<scan_fhog_pyramid<pyramid_down<6> > > frontal_face_detector;
typedef dlib::frontal_face_detector Ffd;
static_assert(Fits<DLibFaceDetector, Ffd>::value, "please increase size of byte array in header by a multiple of 16");

/*ctor*/ DLibFaceDetector::DLibFaceDetector()
{
    ::new (this) Ffd(dlib::get_frontal_face_detector());
}

/*dtor*/ DLibFaceDetector::~DLibFaceDetector()
{
    reinterpret_cast<Ffd *>(this)->~Ffd();
}

std::vector<dlib::rectangle> DLibFaceDetector::findFaceRects(const dlib::cv_image<dlib::bgr_pixel>& img)//NOT CONST, BEWARE MT
{
    return (*reinterpret_cast<Ffd *>(this))(img);//operator()
}
#endif

//more downscaled version *************************************
/*ctor*/ DLibFaceDetector_2_1::DLibFaceDetector_2_1()
{
    Ffd_2_1& r = *(::new (this) Ffd_2_1());
    std::istringstream sin(dlib::get_serialized_frontal_faces());
	deserialize(r, sin);
}

/*dtor*/ DLibFaceDetector_2_1::~DLibFaceDetector_2_1() 
{
    reinterpret_cast<Ffd_2_1 *>(this)->~Ffd_2_1();
}

std::vector<dlib::rectangle> DLibFaceDetector_2_1::findFaceRects(const dlib::cv_image<dlib::bgr_pixel>& img)//NOT CONST, BEWARE MT
{
    return (*reinterpret_cast<Ffd_2_1 *>(this))(img);//operator()
}
//* ************************************

/*ctor*/ DLibLandMarkDetector::DLibLandMarkDetector()
{
    ::new (this) Sp();
}

/*dtor*/ DLibLandMarkDetector::~DLibLandMarkDetector()
{
    reinterpret_cast<Sp *>(this)->~Sp();
}

int DLibLandMarkDetector::init(const char *szPath)
{
	try
	{
		dlib::deserialize(szPath) >> *reinterpret_cast<Sp *>(this);
		return 0;
	}
	catch (const dlib::serialization_error& e)
	{
		fputs("failed to init shape predictor from file dlib exception:\n", stderr);
		fputs(e.what(), stderr);
		return -1;
	}
}

//this one is a doozy to compile
dlib::full_object_detection DLibLandMarkDetector::detectMarks(const dlib::cv_image<dlib::bgr_pixel>& img, const dlib::rectangle& rec) const
{
    return (*reinterpret_cast<const Sp *>(this))(img, rec);
}

