#ifndef __PPM_IO_HEADER__
#define __PPM_IO_HEADER__


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bcp_image_def.hpp"
#include "bcp_exception.hpp"


__BCP_BEGIN_NAMESPACE


enum __PPM_FILE_FORMAT_type
{
    PPM_FORMAT_PPM3,   /* ascii text stream format */
    PPM_FORMAT_PPM6    /* binary format */
};

/* Pick up a single pixel from specified file stream, according to the PPM file
   format. PPM3 stores pixel informations in text format while PPM6 is in compact
   binary format.

   This function reads the pixel in RGB format and returns the color component 
   expansions through parameters red, green and blue.
*/
void __load_ppm_pixel_data(
    FILE *fp, int &red, int &green, int &blue, __PPM_FILE_FORMAT_type format)
{
    switch (format)
    {
    case PPM_FORMAT_PPM3:   // text format
        if (fscanf(fp, "%d %d %d", &red, &green, &blue) != 3) {
            throw invalid_ppm_image();
        };
        break;
        
    case PPM_FORMAT_PPM6:   // binary format
        byte b_red, b_green, b_blue;
        // fread(&red, 1,1, fp); would not work on big endian machines
        fread(&b_red,   1, 1, fp);   red   = (int)b_red;
        fread(&b_green, 1, 1, fp);   green = (int)b_green;
        fread(&b_blue,  1, 1, fp);   blue  = (int)b_blue;
        break;
        
    default:
        throw unrecognized_ppm_format();
    }
}

/* PPM header has been parsed by __load_ppm_image, PPM file format and image size has 
   already been determined. Now this function is invoked to read all actual pixel data
   into the image object.
*/
template <typename _pixel_type>
void __load_ppm_image_data(
    FILE *fp, Image<_pixel_type> &image, __PPM_FILE_FORMAT_type format)
{
    for (index_type y = 0; y < image.get_height(); y++)
    {
        for (index_type x = 0; x < image.get_width(); x++)
        {
            // pick up the pixel in RGB format
            int  red, green, blue;
            __load_ppm_pixel_data(fp, red, green, blue, format);

            // convert the RGB pixel to what we really want
            image(x,y) = ConvertPixel(pixel_RGB(red, green, blue), image(0,0));
        }
    }

    fclose(fp);
}

// open specified .ppm image file and load all pixel informations to *image 
template <typename _pixel_type>
void __load_ppm_image(const char *filename, Image<_pixel_type> &image)
{
#define PPM_LINE_WIDTH 71  /* max number of characters in a line */

    FILE *fp = fopen(filename, "rb");
    char tmp_buf[PPM_LINE_WIDTH];  // temporary buffer for fd

    if (fp == NULL) 
        throw cannot_open_file(filename);

    // read ppm header, the first line of a .ppm file should be the magic number: "PX"
    __PPM_FILE_FORMAT_type file_format;
    fscanf(fp, "%s", tmp_buf);

    if (strcmp(tmp_buf, "P3") == 0)       file_format = PPM_FORMAT_PPM3;
    else if (strcmp(tmp_buf, "P6") == 0)  file_format = PPM_FORMAT_PPM6;
    else {
        // the header of this .ppm file is unexpected
        fclose(fp);
        throw unrecognized_ppm_format();
    }

    /* skip comments: comments should appear only at top of the file and starts
       with a #, only line comments were taken into consideration (I don't know
       if .ppm format supports block comments >_<
    */
    fgets(tmp_buf, PPM_LINE_WIDTH, fp);  // skip the comming CR/LF
    fscanf(fp, "%c", tmp_buf);
    while (*tmp_buf == '#') {
        // skip the comment line
        fgets(tmp_buf, PPM_LINE_WIDTH, fp); fscanf(fp, "%c", tmp_buf);
    }
    ungetc(*tmp_buf, fp);

    // image size (width and height in px) and max pixel were given right after
    // all comments
    size_type width, height;
    fscanf(fp, "%d %d %*d", &width, &height);
    image = Image<_pixel_type>(width, height);

    /* read all pixels according to the .ppm file format */
    fgets(tmp_buf, PPM_LINE_WIDTH, fp); /* skip the comming whitespace */
    __load_ppm_image_data(fp, image, file_format);


#undef PPM_LINE_WIDTH
}


// Save the image object to a PPM6 file (binary format)
template <typename _pixel_type>
void __save_ppm_image(const char *ppm_filename, const Image<_pixel_type> &img)
{
    FILE *fp = fopen(ppm_filename, "wb");
    if (fp == NULL)
        throw cannot_open_file(ppm_filename);

    // write PPM6 header
    fprintf(fp, "P6\n");
    fprintf(fp, "%d %d %d\n", img.get_width(), img.get_height(), 255);

    for (index_type y = 0; y < img.get_height(); y++)
    {
        for (index_type x = 0; x < img.get_width(); x++)
        {
            // get RGB format pixel from the image
            pixel_RGB rgb = ConvertPixel(img(x,y), pixel_RGB());
            // write to file
            fwrite(&(rgb.r), 1,1, fp);
            fwrite(&(rgb.g), 1,1, fp);
            fwrite(&(rgb.b), 1,1, fp);
        }
    }

    fclose(fp);   // done
}


__BCP_END_NAMESPACE


#endif /* __PPM_IO_HEADER__ */
