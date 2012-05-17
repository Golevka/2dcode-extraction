#ifndef __BCP_LOCATE_BARCODE_HEADER__
#define __BCP_LOCATE_BARCODE_HEADER__


#include <algorithm>
#include <functional>
#include <numeric>
#include "bcp_proc.hpp"


__BCP_BEGIN_NAMESPACE


/* Structure for keeping the result of a tomography projection analysis. */
struct __2Dcode_Location
{
    index_type y0;     // estimated vertical location of the 2D code
    double tilt;       // tilt/oblique level
    int confidence;    // confidence of the estimation result
};


/* Calculates a series of integrations on discf(discrete function: f(x) = 
   discf[x]) and stores the result in place.

       discf[0] = sum(discf, [0, interval-1]), 
       discf[1] = sum(discf, [1, inteval]),
       ......
       discf[k] = sum(discf, [k, interval+k-1])
       ......
*/
template <typename _iterator_type>
void __PiecewiseIntegration(
    _iterator_type discf_begin, _iterator_type discf_end, 
    size_type interval)
{
    typedef 
        typename std::iterator_traits<_iterator_type>::value_type 
        value_type;

    /* we adopt a trick to calculate this series of continuous sums. first 
       do an accumulation at the array */
    std::partial_sum(discf_begin, discf_end, discf_begin);

    /* then we just need to calculate the difference of discf[k + interval -1] 
       and discf[k - 1] to get the integration result at discf[k]. */
    _iterator_type 
        int_a   = discf_begin,
        int_b   = discf_begin + (interval - 1);
    
    value_type val = 0;
    for ( ; int_b != discf_end; ++int_a, ++int_b)
    {
        value_type val2 = *int_b - val;
        val    = *int_a;
        *int_a = val2;
    }
}

/* Estimate the vertical location of the 2D code using a given 1D tomography
   projection data, this data can be obtained with TomographyProjection().

   This function calculates the dicrete integration of the 1D tomography 
   projection function `tomo_array' on [y0, y0+code_height] and pick up the 
   integration interval which obtains maximum integration value as result.

   A __2Dcode_Location structure is returned as the result, y0 indicates the 
   startpoint of the integration interval, as well as the vertical location 
   of the 2D code; confidence is the integration value on this interval, it
   indicates the confidence of this estimation result.
*/
template <typename _iterator_type>
__2Dcode_Location __Estimate2DcodeLocation(
    _iterator_type tomo_begin, _iterator_type tomo_end, size_type code_height)
{
    __PiecewiseIntegration(tomo_begin, tomo_end, code_height);

    // we need to narrow the range to where the integration values are.
    _iterator_type 
        int_begin = tomo_begin,
        int_end   = tomo_end - code_height + 1;

    _iterator_type itr_max = std::max_element(int_begin, int_end);

    __2Dcode_Location loc;
    loc.y0         = (index_type)(itr_max - int_begin);
    loc.confidence = (int)(*itr_max);

    return loc;
}

/* An easy to use overload version, "inline" just intent to fit it into a 
   header file or it would produce a link-time error when header dependencies
   were messed up 
*/
inline __2Dcode_Location __Estimate2DcodeLocation(
    std::vector<int> &tomo_array, size_type code_height)
{    
    return __Estimate2DcodeLocation(
        tomo_array.begin(), tomo_array.end(), code_height);
}


/* Try tomography projection on different oblique levels and pick up the best 
   one, we can get the horizonal tilt and vertical location of the 2D code 
   through this procedure.
*/
template <typename _pixel_type>
__2Dcode_Location Locate2DCode(
    const Image<_pixel_type> &img, int max_oblique, size_type code_height)
{
    std::vector<int> tomo_array(img.get_height());
    __2Dcode_Location best_so_far = {0, 0, 0};

    for (int h_oblique = -max_oblique; h_oblique < max_oblique; h_oblique++)
    {
        // adjust the sweep angle and make the tomography projection
        double k = (double)h_oblique / (double)img.get_width();
        TomographyProjection(img, k, tomo_array);

        // estimate location on this oblique level
        __2Dcode_Location loc = 
            __Estimate2DcodeLocation(tomo_array, code_height);

        if (loc.confidence > best_so_far.confidence) {
            best_so_far = loc; 
            best_so_far.tilt = k;
        }
    }

    return best_so_far;
}

__BCP_END_NAMESPACE


#endif /* __BCP_LOCATE_BARCODE_HEADER__ */
