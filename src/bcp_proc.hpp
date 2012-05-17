#ifndef __BCP_IMGPROCESSING_HEADER__
#define __BCP_IMGPROCESSING_HEADER__


#include <assert.h>
#include <vector>
#include <cmath>

#include "bcp_image_def.hpp"
#include "ppm_io.hpp"

__BCP_BEGIN_NAMESPACE


/* Convert the image from one type to another, it works through converting the 
   type of pixels using ConvertPixel().
*/
template <typename _image_ty_from, typename _image_ty_to>
_image_ty_to ConvertImage(const _image_ty_from &img, const _image_ty_to &)
{
    // allocate for the resulting image
    _image_ty_to ret_img(img.get_width(), img.get_height());

    // convert each pixel to the destination type.
    for (index_type y = 0; y < img.get_height(); y++)
    {
        for (index_type x = 0; x < img.get_width(); x++) {
            ret_img(x,y) = ConvertPixel(img(x,y), ret_img(0,0));
        }
    }

    return ret_img;
}


/* Read an image from PPM archive to the referenced Image object */
template <typename _pixel_type>
void LoadPPMImage(const char *ppm_filename, Image<_pixel_type> &img)
{
    __load_ppm_image(ppm_filename, img);
}

/* An overloaded version which returns an Image object */
template <typename _image_type>
_image_type LoadPPMImage(const char *ppm_filename)
{
    Image<pixel_RGB> rgb_img;
    LoadPPMImage(ppm_filename, rgb_img);

    return ConvertImage(rgb_img, _image_type());
}

/* Save specified Image object to an PPM6 archive */
template <typename _pixel_type>
void SavePPM6Image(const char *ppm_filename, const Image<_pixel_type> &img)
{
    __save_ppm_image(ppm_filename, img);
}


/* Create a transposed version of the image */
template <typename _pixel_type>
Image<_pixel_type> TransposeImage(const Image<_pixel_type> &img)
{
    Image<_pixel_type> img_trans(img.get_height(), img.get_width());
    
    for (index_type x = 0; x < img.get_width(); x++)
    {
        for (index_type y = 0; y < img.get_height(); y++) {
            img_trans(y,x) = img(x,y);    // transpose
        }
    }

    return img_trans;
}


/* Rotate the image certain rads around the specified point */
template <typename _pixel_type>
Image<_pixel_type> RotateImage(const Image<_pixel_type> &img, 
    double rad, index_type cx, index_type cy)
{
    Image<_pixel_type> img_rot(img.get_width(), img.get_height());
    double sin_phi = std::sin(rad), cos_phi = std::cos(rad);

    /* prepare a white pixel for furture use. any parts rotated in from the
       outside world are filled with white pixels. */
    _pixel_type white_pixel = 
        ConvertPixel(pixel_RGB(255,255,255), _pixel_type());

    for (index_type y = 0; y < img.get_height(); y++)
    {
        for (index_type x = 0; x < img.get_width(); x++)
        {
            /* translated pixel coordinate after moving the center to the 
               original point*/
            int tx = x - cx, ty = y - cy;

            // rotate around the original point
            double rx_d = tx * cos_phi - ty * sin_phi, 
                   ry_d = tx * sin_phi + ty * cos_phi;

            // round to nearest integer and translate back
            index_type rx = (index_type)(rx_d + 0.5) + cx,
                       ry = (index_type)(ry_d + 0.5) + cy;

            if (rx >= 0 && rx < img.get_width() && 
                ry >= 0 && ry < img.get_height())
            {
                img_rot(x,y) = img(rx,ry);    // in bound
            }
            else {
                img_rot(x,y) = white_pixel;   // outside world rolled in
            }
        }
    }

    return img_rot;
}


/* Crop img to specified rect, return the cropped image. This function will
   not hurt the original image specified in parameter list.
*/
template <typename _pixel_type>
Image<_pixel_type> CropImage(
    const Image<_pixel_type> &img,
    index_type left, index_type right, index_type top, index_type bottom)
{
    assert(left < right && top < bottom);

    // create a cropped image
    Image<_pixel_type> piece(right - left + 1, bottom - top + 1);

    // copy pixels in the specified rect to the cropped piece.
    for (index_type y = top, py = 0; y <= bottom; y++, py++)
    {
        for (index_type x = left, px = 0; x <= right; x++, px++) {
            piece(px,py) = img(x,y);
        }
    }

    return piece;
}


/* Convert an RGB or grayscale image to a monochrome one using specified 
   threshold 
*/
template <typename _pixel_type>
Image<pixel_Monochrome> ThresholdImage(
    const Image<_pixel_type> &img, int threshold)
{
    Image<pixel_Monochrome> binimg(img.get_width(), img.get_height());

    // thresholding each pixel using ThresholdPixel()
    for (index_type y = 0; y < img.get_height(); y++)
    {
        for (index_type x = 0; x < img.get_width(); x++) {
            binimg(x,y) = ThresholdPixel(img(x,y), threshold);
        }
    }

    return binimg;
}


/* Otsu's algorithm picks up a reasonable threshold value according to the
   histogram of the image. */
template <typename _pixel_type>
int OtsuThresholdSelector(const Image<_pixel_type> &img)
{
    int width  = img.get_width();
    int height = img.get_height();
      
    //histogram
    float histogram[256] = {0};
    for (int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++) {
            int val = ConvertPixel(img(x,y), pixel_Grayscale()).val;
            histogram[val]++;  
        }
    }

    //normalize histogram  
    int size = height * width;
    for(int i = 0; i < 256; i++) {
        histogram[i] = histogram[i] / size;
    }
    
    //average pixel value
    float avgValue = 0;
    for(int i = 0;i < 256; i++) {
        avgValue += i * histogram[i];
    }  
  
    int threshold = 0;
    float maxVariance = 0, w = 0,u = 0;
    for(int i = 0;i < 256; i++)
    {
        w += histogram[i];
        u += i * histogram[i];
  
        float t = avgValue * w - u;
        float variance = t * t / (w*(1 - w));
        if(variance > maxVariance){
            maxVariance = variance, threshold = i;
        }  
    }

    return threshold;
}

/* Use a 1D ray to detect the density of the image on a line, it accumulates all
   black dots on specified monochrome image and return the final sum value. 

   Monochrome image is prefered by this method because it doesn't need to do any
   convertion on pixel formats. However image with any other pixel types is also
   acceptable.
*/
template <typename _pixel_type>
int RayDetection(const Image<_pixel_type> &img, double k, index_type y0)
{
    size_type 
        width = img.get_width(), 
        height = img.get_height();

    int n_black = 0;  // number of black points on this line

    for (index_type x = 0; x < width; x++)
    {
        index_type y = index_type(y0 + x*k);

        /* if (x,y) is out of bounds, we need to break out the loop and 
           return from this function immediately */
        if (y < 0 || y >= height) break;

        // or we have to take this pixel into count
        pixel_Monochrome px = ConvertPixel(img(x,y), pixel_Monochrome());
        n_black += (px.val == 0? 1: 0);
    }
    
    return n_black;
}


/* Project the image onto a 1D "plane" by accumulating black dots on parallel 
   rays using RayDetection method. Monochrome image is strongly recommended 
   because it would make the RayDetection procedure way faster.
*/
template <typename _pixel_type>
void TomographyProjection(
    const Image<_pixel_type> &img, double k, std::vector<int> &tomo_array)
{
    tomo_array.resize(img.get_height());  /* each element contains the ray-
                                             detection value of a line */

    // calculate ray projection value of each parallel line
    for (index_type y = 0; y < img.get_height(); y++) {
        tomo_array[y] = RayDetection(img, k, y);
    }
}

/* Another overload version which feedback the result as return value */
template <typename _pixel_type>
std::vector<int> TomographyProjection(const Image<_pixel_type> &img, double k)
{
    std::vector<int> tomo_array;
    TomographyProjection(img, k, tomo_array);

    return tomo_array;
}


__BCP_END_NAMESPACE


#endif /* __BCP_IMGPROCESSING_HEADER__ */
