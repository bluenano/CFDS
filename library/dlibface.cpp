#include "pch.h"

#include <new>//placement new, call constructor at preallocated memory

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>

typedef dlib::frontal_face_detector Ffd;
typedef dlib::shape_predictor Sp;

template<class A, class B>
struct Fits
{
    enum{value = sizeof(A) >= sizeof(B) && alignof(A) >= alignof(B) };
};

static_assert(Fits<dlFaceRoiFinder, Ffd>::value, "");
static_assert(Fits<dlFaceMarkDetector, Sp>::value, "");

typedef dlib::frontal_face_detector Ffd;//and this is a typdef of some template thing, with a <6> param
//saw a comment can make that smaller for performance increase

/*ctor*/ dlFaceRoiFinder::dlFaceRoiFinder()
{
    ::new (this) Ffd(dlib::get_frontal_face_detector());
}

/*dtor*/ dlFaceRoiFinder::~dlFaceRoiFinder()
{
    reinterpret_cast<Ffd *>(this)->~Ffd();
}

std::vector<dlib::rectangle> dlFaceRoiFinder::findFaceRects(const dlib::cv_image<dlib::bgr_pixel>& img)
{
    return (*reinterpret_cast<Ffd *>(this))(img);//tasteful operator()
}


/*ctor*/ dlFaceMarkDetector::dlFaceMarkDetector()
{
    ::new (this) Sp();
}

/*dtor*/ dlFaceMarkDetector::~dlFaceMarkDetector()
{
    reinterpret_cast<Sp *>(this)->~Sp();
}

int dlFaceMarkDetector::init(const char *szPath)
{
	try
	{
		dlib::deserialize(szPath) >> *reinterpret_cast<Sp *>(this);
		return 0;
	}
	catch(const dlib::serialization_error& e)
	{
		fprintf(stderr, "failed to init shape predictor from file: %s\n", e.what());
		return -1;
	}
}
//this one is a doozy to compile
dlFaceMarkResults dlFaceMarkDetector::detectMarks(const dlib::cv_image<dlib::bgr_pixel>& img, const dlib::rectangle& rec)
{
    return (*reinterpret_cast<Sp *>(this))(img, rec);
}

