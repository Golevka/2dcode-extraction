#include <iostream>
#include "bcp_image.hpp"
#include "ppm_io.hpp"

#include <algorithm>
#include <iterator>

#include "bcp_locate.hpp"



int main(int argc, char *argv[])
{
    const bcp::size_type
        target_x0    = 1350, target_y0     = 232,
        target_width = 1212, target_height = 428,
        code_size = 216, code_left = 250, code_padding = 10;

    if (argc == 2)
    {
        try
        {
            std::cout << "Loading image..." << std::endl;
            bcp::Image<> ppm_img(argv[1]);

            // Crop (roughly) and threshold the image
            std::cout << "Thresholding..." << std::endl;
            bcp::Image<bcp::pixel_Monochrome> ppm_mono = 
                ppm_img.crop(
                    target_x0, target_x0 + target_width, 
                    target_y0, target_y0 + target_height).threshold();

            ppm_mono.save_ppm("thresholded.ppm");

            // Horizonal tilt calibration and crop precisely
            std::cout << "Tomography projection..." << std::endl;
            bcp::__2Dcode_Location loc = 
                bcp::Locate2DCode(ppm_mono, 50, code_size);

            bcp::Image<bcp::pixel_Monochrome> img_crop_h = 
                ppm_mono.rotate(std::atan(loc.tilt), 0, 0).
                crop(0, 0 + target_width,
                    loc.y0, loc.y0 + code_size);

            // img_crop_h.save_ppm("cropped.ppm");


            std::cout << "Splitting 2D codes..." << std::endl;

            // Vertical crop
            bcp::Image<bcp::pixel_Monochrome> img_trans = img_crop_h.transpose();
            std::vector<int> tomo_array = bcp::TomographyProjection(img_trans, 0);
            bcp::__PiecewiseIntegration(tomo_array.begin(), tomo_array.end(), code_size);

            // estimate the position of the first 2D code
            std::vector<int>::iterator itr = 
                std::max_element(tomo_array.begin(), tomo_array.begin() + code_left);

            bcp::index_type  top = (bcp::index_type)(itr - tomo_array.begin());
            std::cout << "position: " << top << std::endl;


            // Saving splitted 2D codes
            img_trans.crop(0, code_size, top, top + code_size).transpose().save_ppm("part_1.ppm"); 
            top += (code_size + code_padding);

            img_trans.crop(0, code_size, top, top + code_size).transpose().save_ppm("part_2.ppm");
            top += (code_size + code_padding);

            img_trans.crop(0, code_size, top, top + code_size).transpose().save_ppm("part_3.ppm");
            top += (code_size + code_padding);

            img_trans.crop(0, code_size, top, top + code_size).transpose().save_ppm("part_4.ppm");
            top += (code_size + code_padding);
        }
        catch(bcp::exception &e) {
            std::cout << e.message() << std::endl;
        }
    }
    else {
        std::cout << "usage: " << argv[0] << " ppm_filename" << std::endl;
    }

    return 0;
}

