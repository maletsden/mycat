#ifndef IO_HPP
#define IO_HPP

#include <unistd.h>
#include <string>
#include <sstream>

#define IO_FAILURE  -1
#define IO_SUCCESS  0

namespace io {
    struct CatParams {
        int fd;
        int out_fd;
        int err_fd;
        std::string_view filename;
        // strings to unsure freeing memory
        std::string buffer;
        std::string buffer_visible;
        ssize_t buff_size;
        ssize_t buff_v_size;
        int* status;
        bool print_invisible;
    };

    int open_file(const char*, int, int*);
    int open_file(const std::string&, int, int*);
    int open_file(const std::string_view&, int, int*);
    int write_buffer(int, const char*, ssize_t, int*);
    int write_buffer(int, const std::string&, int*);
    int read_buffer(int, char*, ssize_t, ssize_t*, int*);
    int read_buffer(int, std::string&, ssize_t*, int*);
    int close_file(int, int*);
    int cat_file(CatParams&);
    ssize_t invisible_to_hex(const std::string&, std::string&, ssize_t);
}

#endif //IO_HPP
