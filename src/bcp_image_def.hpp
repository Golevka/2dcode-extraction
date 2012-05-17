#ifndef __BCP_PIXEL_IMAGE_DEF_HEADER__
#define __BCP_PIXEL_IMAGE_DEF_HEADER__


#include "bcp_pixel.hpp"

/*
  This header defines some basic pixel types (RGB, RGBA, grayscale and monochrome) for 
  bitmap image and provided the definition of a (bitmap) image template class.
*/

__BCP_BEGIN_NAMESPACE


// Image class, pixel type of the image should be specified as template parameter.
template <typename _pixel_type = pixel_RGB>
class Image
{
public:
    typedef _pixel_type pixel_type;  // type of pixels

    // initialize image with specified size
    Image(size_type image_width = 0, size_type image_height = 0);
    Image(const char* ppm_filename);   // initialize with a PPM image file
    Image(const Image &img);           // copy constructor
    ~Image(void);                      // destructor

    const Image<pixel_type> & operator = (const Image<pixel_type> &img);


    // get image metrics
    size_type  get_width(void)  const;
    size_type  get_height(void) const;
    
    // obtain a single pixel of the image with px coordinates
    pixel_type & get_pixel(index_type m, index_type n);
    const pixel_type & get_pixel_const(index_type m, index_type n) const;

    // easy to use short hand form of get_pixel*()
    pixel_type & operator () (index_type m, index_type n);
    const pixel_type & operator () (index_type m, index_type n) const;


    // create a transposed version of this image
    Image<pixel_type> transpose(void) const;

    // rotate this image using specified angel(rad) and center point
    Image<pixel_type> rotate(double rad, index_type cx, index_type cy) const;

    // crop an image, the cropped part is returned as a new image object, and
    // this image was not hurt.
    Image<pixel_type> crop(index_type left, index_type right, 
        index_type top, index_type bottom) const;

    // generate a monochrome version of this image using specified threshold
    Image<pixel_Monochrome> threshold(int thrld) const;

    // use Otsu's method to determine the threshold value
    Image<pixel_Monochrome> threshold(void) const;

    // save the Image object as a PPM6 image
    void save_ppm(const char *ppm_filename) const;


protected:
    size_type  width, height;  // size of the image: width x height
    pixel_type *px;            // an array of pixels
};


__BCP_END_NAMESPACE



#endif /* __BCP_PIXEL_IMAGE_DEF_HEADER__ */
