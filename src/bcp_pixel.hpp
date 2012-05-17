#ifndef __BCP_PIXEL_CONVERTION_HEADER__
#define __BCP_PIXEL_CONVERTION_HEADER__


#include "bcp_base.hpp"

__BCP_BEGIN_NAMESPACE


// RGB pixel: Red-Green-Blue
struct pixel_RGB
{
    byte r, g, b;         // RGB components
    pixel_RGB(byte R = 0, byte G = 0, byte B = 0)
        : r(R), g(G), b(B) {}

    // it is already an RGB object, just simply return itself.
    pixel_RGB RGB(void) const {
        return *this;
    }
};

// Monochrome image: black(0) and white(1)
struct pixel_Monochrome
{
    int val;
    pixel_Monochrome(int Val = 0): val(Val) {}

    // black or white, quite simple rule.
    pixel_RGB RGB(void) const {
        return pixel_RGB(val*255, val*255, val*255);
    }
};

// Grayscale image
struct pixel_Grayscale
{
    int val;
    pixel_Grayscale(int Val = 0): val(Val) {}

    // converting a grayscale pixel to an RGB one is quite straight forward.
    pixel_RGB RGB(void) const {
        return pixel_RGB(val, val, val);
    }
};


// RGB ==> Grayscale
inline pixel_Grayscale 
__convert_from_RGB_to(const pixel_RGB &px, pixel_Grayscale)
{
    /* The transition from RGB value to grayscale value is based on an 
       empirical equation: 

           brightness = 0.3*red + 0.59*green + 0.11*blue
    */
    return pixel_Grayscale(
        px.r * 0.30 + px.g * 0.59 + px.b * 0.11 + 0.5);
}

// RGB ==> Monochrome
inline pixel_Monochrome
__convert_from_RGB_to(const pixel_RGB &px, pixel_Monochrome)
{
    pixel_Grayscale px_g = __convert_from_RGB_to(px, pixel_Grayscale());
    return px_g.val > 127? pixel_Monochrome(1): pixel_Monochrome(0);
}

// RGB ==> RGB
inline pixel_RGB
__convert_from_RGB_to(const pixel_RGB &px, pixel_RGB)
{
    return px;   // there's no need to do any conversion here, it just acts like
                 // a placeholder.
}


/* Convert pixel type from _pixel_ty_from to _pixel_ty_to. We always convert 
   _pixel_ty_from to pixel_RGB first and then convert the RGB pixel to 
   _pixel_ty_to.

   This function only works as a proxy/interface function, implementation of
   the actual converting procedure are dispatched to various overloaded versions
   of __convert_from_RGB_to.
*/
template <typename _pixel_type>
_pixel_type ConvertPixel(const _pixel_type &px_src, const _pixel_type &) {
    return px_src;   /* ordinary "identify" conversion, do nothing */
}

template <typename _pixel_ty_from, typename _pixel_ty_to>
_pixel_ty_to ConvertPixel(const _pixel_ty_from &px_src, const _pixel_ty_to &)
{
    return __convert_from_RGB_to(px_src.RGB(), _pixel_ty_to());
}


/* Threshold a pixel according to its grayscale value */
template <typename _pixel_type>
pixel_Monochrome ThresholdPixel(const _pixel_type &px, int threshold)
{
    pixel_Grayscale gpx = ConvertPixel(px, pixel_Grayscale());
    return gpx.val >= threshold? pixel_Monochrome(1): pixel_Monochrome(0);
}


__BCP_END_NAMESPACE


#endif /* __BCP_PIXEL_CONVERTION_HEADER__ */
