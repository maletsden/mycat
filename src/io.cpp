#include "operations/io.hpp"

#include <cerrno>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>

int io::open_file(const char *filename, int oflag, int *status) {
    int fd = open(filename, oflag);

    if (fd < 0) {
        if (status) {
            *status = errno;
        }
        return IO_FAILURE;
    }
    return fd;
}

int io::open_file(const std::string& filename, int oflag, int* status) {
    return io::open_file(filename.data(), oflag, status);
}

int io::open_file(const std::string_view& filename, int oflag, int* status) {
    return io::open_file(filename.data(), oflag, status);
}

int io::close_file(int fd, int *status) {
    int res = close(fd);
    if (res == IO_FAILURE) {
        if (status) {
            *status = errno;
        }
        return IO_FAILURE;
    }
    return IO_SUCCESS;
}

int io::write_buffer(int fd, const char *buffer, ssize_t size, int *status) {
    ssize_t written_bytes = 0;
    while (written_bytes < size) {
        ssize_t written_now = write(fd, buffer + written_bytes,
                                    size - written_bytes);
        if (written_now == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                if (status) {
                    *status = errno;
                }
                return IO_FAILURE;
            }
        } else {
            written_bytes += written_now;
        }
    }
    return IO_SUCCESS;
}

int io::write_buffer(int fd, const std::string& buffer, int* status) {
    return io::write_buffer(fd, buffer.data(), buffer.size(), status);
}

int io::read_buffer(int fd, char *buffer, ssize_t size, ssize_t *bytes_read, int *status) {
    ssize_t read_bytes = 0;

    while (read_bytes < size) {
        ssize_t read_now = read(fd, buffer + read_bytes,
                                size - read_bytes);

        if (read_now == 0) { // end-of-file
            break;
        } else if (read_now == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                if (status) {
                    *status = errno;
                }
                return IO_FAILURE;
            }
        } else {
            read_bytes += read_now;
        }
    }

    if (bytes_read) {
        *bytes_read = read_bytes;
    }
    return IO_SUCCESS;
}

int io::read_buffer(int fd, std::string &buffer, ssize_t *bytes_read, int *status) {
    return io::read_buffer(fd, buffer.data(), buffer.size(), bytes_read, status);
}

ssize_t io::invisible_to_hex(const std::string& buffer, std::string& new_buff, ssize_t buff_size) {
    static const std::string digits = "0123456789ABCDEF";

    ssize_t len = 0;
    char* buff_data = new_buff.data();
    for (ssize_t i = 0; i < buff_size; ++i) {
        auto c = static_cast<unsigned char>(buffer[i]);
        if (isprint(c) || isspace(c)) {
            buff_data[len++] = c;
        } else {
            buff_data[len++] = '\\';
            buff_data[len++] = 'x';
            buff_data[len++] = digits[c >> 4u];
            buff_data[len++] = digits[c & 15u];
        }
    }
    return len;
}

int io::cat_file(io::CatParams& params) {
    ssize_t bytes_read = 0;

    do {
        int res = io::read_buffer(params.fd, params.buffer, &bytes_read, params.status);

        if (res == IO_FAILURE) {
            std::stringstream ss;
            ss << "Error while reading file '" << params.filename << "' (status: " << *params.status << params.fd <<")\n";

            io::write_buffer(params.err_fd, ss.str(), params.status);

            return IO_FAILURE;
        }

        if (!bytes_read) break;

        if (params.print_invisible) {
            auto visible_bytes = io::invisible_to_hex(params.buffer, params.buffer_visible, bytes_read);
            res = io::write_buffer(params.out_fd, params.buffer_visible.data(), visible_bytes, params.status);
        } else {
            res = io::write_buffer(params.out_fd, params.buffer.data(), bytes_read, params.status);
        }

        if (res == IO_FAILURE) {
            std::stringstream ss;
            ss << "Error while printing content of file '" << params.filename << "' (status: " << *params.status << ")\n";

            io::write_buffer(params.err_fd, ss.str(), params.status);

            return IO_FAILURE;
        }
    } while (bytes_read);

    return IO_SUCCESS;
}
