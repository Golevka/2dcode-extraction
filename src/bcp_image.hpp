#ifndef __BCP_IMAGE_HEADER__
#define __BCP_IMAGE_HEADER__


#include <assert.h>
#include <algorithm>

#include "bcp_image_def.hpp"
#include "bcp_proc.hpp"


__BCP_BEGIN_NAMESPACE


// (Default) constructor of the image class
template <typename _pixel_type>
Image<_pixel_type>::Image(size_type image_width, size_type image_height)
    : width(image_width),
      height(image_height),
      px(new Image<_pixel_type>::pixel_type[
              image_width * image_height])
{
    // initialize each pixel as its default color, resulting an "blank" image
    std::fill_n(px, width * height, Image<_pixel_type>::pixel_type());
}

// Initialize an image object with a PPM image file.
template <typename _pixel_type>
Image<_pixel_type>::Image(const char *ppm_filename)
    : width(0), height(0), px(NULL)
{
    LoadPPMImage(ppm_filename, *this);
}

// Copy constructor
template <typename _pixel_type>
Image<_pixel_type>::Image(const Image<_pixel_type> &img)
    : width(img.width), height(img.height),
      px(new Image<_pixel_type>::pixel_type[img.width * img.height])
{
    // initialize this image with img
    std::copy(img.px, img.px + width * height, this->px);
}

// Destructor of the image class
template <typename _pixel_type>
Image<_pixel_type>::~Image(void) {
    delete [] px;    // release allocated memory for pixel array
}


// operator = 
template <typename _pixel_type>
const Image<_pixel_type> & Image<_pixel_type>::operator = (
    const Image<pixel_type> &img)
{
    this->~Image<_pixel_type>();     // destroy the old one

    // construct the current one
    this->width  = img.width;
    this->height = img.height;
    this->px = new Image<_pixel_type>::pixel_type[width * height];
    std::copy(img.px, img.px + width * height, this->px);

    return *this;  // convention of operator = ()
}


// get image metrics
template <typename _pixel_type>
size_type Image<_pixel_type>::get_width(void) const {
    return this->width;
}

template <typename _pixel_type>
size_type Image<_pixel_type>::get_height(void) const {
    return this->height;
}


// obtain a single pixel of the image with px coordinates
template <typename _pixel_type>
typename Image<_pixel_type>::pixel_type & 
Image<_pixel_type>::get_pixel(index_type m, index_type n)
{
    // (m,n) cannot fall beyond the image metrics.
    assert(m >= 0 && m < width && n >= 0 && n < height);
    return this->px[n * width + m];
}

// obtain a single pixel of the image with px coordinates
template <typename _pixel_type>
const typename Image<_pixel_type>::pixel_type & 
Image<_pixel_type>::get_pixel_const(index_type m, index_type n) const
{
    // (m,n) cannot fall beyond the image metrics.
    assert(m >= 0 && m < width && n >= 0 && n < height);
    return this->px[n * width + m];
}

// A short hand of get_pixel and get_pixel_const
template <typename _pixel_type>
typename Image<_pixel_type>::pixel_type &
Image<_pixel_type>::operator () (index_type m, index_type n) {
    return this->get_pixel(m, n);
}

template <typename _pixel_type>
const typename Image<_pixel_type>::pixel_type &
Image<_pixel_type>::operator () (index_type m, index_type n) const {
    return this->get_pixel_const(m, n);
}


// Create a transposed version of this image
template <typename _pixel_type>
Image<_pixel_type> Image<_pixel_type>::transpose() const
{
    return TransposeImage(*this);
}

// Rotate this image using specified angel(rad) and center point
template <typename _pixel_type>
Image<_pixel_type> Image<_pixel_type>::rotate(
    double rad, index_type cx, index_type cy) const
{
    return RotateImage(*this, rad, cx, cy);
}

/* Crop an image, the cropped part is returned as a new image object, and
   this image was not hurt. */
template <typename _pixel_type> 
Image<_pixel_type> Image<_pixel_type>::crop(
    index_type left, index_type right, 
    index_type top,  index_type bottom) const
{
    return CropImage(*this, left,right, top,bottom);
}

// Generate a monochrome version of this image using specified threshold
template <typename _pixel_type>
Image<pixel_Monochrome> Image<_pixel_type>::threshold(int thrld) const
{
    return ThresholdImage(*this, thrld);
}

/* This version invokes OtsuThresholdSelector() which automatically select
   a threshold value according to the histogram of the image. */
template <typename _pixel_type>
Image<pixel_Monochrome> Image<_pixel_type>::threshold(void) const
{
    return this->threshold(OtsuThresholdSelector(*this));
}

// Save the Image object as a PPM6 image
template <typename _pixel_type>
void Image<_pixel_type>::save_ppm(const char *ppm_filename) const
{
    SavePPM6Image(ppm_filename, *this);
}



__BCP_END_NAMESPACE


#endif /* __BCP_IMAGE_HEADER__ */

