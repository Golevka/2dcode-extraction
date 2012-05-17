#ifndef __BCP_EXCEPTIONS_HEADER__
#define __BCP_EXCEPTIONS_HEADER__


#include <string>
#include "bcp_base.hpp"

__BCP_BEGIN_NAMESPACE


class exception
{
public:
    exception(const char *message): msg(message) {}
    virtual ~exception(void) {}   // required by Weffc++

    std::string message(void) const {
        return msg;
    }

protected:
    std::string msg;
};


// template for exeptions with monolithic messege
#define __BCP_DECLARE_EXCEPTION(name, message)  \
    class name: public exception { public:      \
        name(void): exception(message) {        \
        }                                       \
    }

__BCP_DECLARE_EXCEPTION(invalid_ppm_image,          "Invalid PPM image");
__BCP_DECLARE_EXCEPTION(unrecognized_ppm_format,    "Unrecognized PPM file format");
__BCP_DECLARE_EXCEPTION(cannot_open_specified_file, "Cannot open specified file");


class cannot_open_file: public exception
{
public:
    cannot_open_file(const char *filename)
        : exception(std::string(
                "Cannot open file: " + std::string(filename)).c_str()) {
    }
};



__BCP_END_NAMESPACE


#endif /* __BCP_EXCEPTIONS_HEADER__ */
