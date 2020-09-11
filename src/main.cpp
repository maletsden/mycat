#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include <fcntl.h>
#include <vector>
#include <sstream>

#include "operations/io.hpp"

int main(int argc, char **argv) {
    namespace po = boost::program_options;

    po::options_description visible("Supported options");
    visible.add_options()
            ("help,h", "Print this help message.");
    visible.add_options()
            (",A", "Print invisible characters.");

    po::options_description all("All options");
    all.add(visible);


    po::variables_map vm;
    auto parsed = po::command_line_parser(argc, argv)
            .options(all)
            .run();

    std::vector<std::string_view> filenames{};
    filenames.reserve(argc);

    for (auto& option: parsed.options) {
        if (option.string_key.empty()) { // true if this is not an option
            // [0] - since if this is not option than it can have just 1 value
            filenames.emplace_back(option.value[0]);
        }
    }
    po::store(parsed, vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage:\n  mycat [-h|--help] [-A] <file1> <file2> ... <fileN>\n" << visible << std::endl;
    }

    bool print_invisible = vm.count("-A") == 1;

    std::vector<int> fds {};
    fds.reserve(filenames.size());

    int status = 0;

    int exit_code = EXIT_SUCCESS;

    // open files
    for (const auto& filename: filenames) {

        int fd = io::open_file(filename, O_RDONLY, &status);
        if (fd == IO_FAILURE) {
            std::stringstream ss;
            ss << "File '" << filename << "' cannot be open (status: " << status << ")\n";

            io::write_buffer(STDERR_FILENO, ss.str(), &status);

            exit_code = EXIT_FAILURE;
            break;
        }

        fds.push_back(fd);
    }

    // cat files
    if (!exit_code) {

        const ssize_t buff_size = 1u << 20u; // 1MB

        io::CatParams params {
                .fd = IO_FAILURE,
                .out_fd = STDOUT_FILENO,
                .err_fd = STDERR_FILENO,
                .filename = "",
                .buffer = std::string(buff_size, '\0'),
                .buffer_visible = print_invisible ? std::string(buff_size * 4, '\0') : "",
                .buff_size = buff_size,
                .buff_v_size = print_invisible ? buff_size * 4 : 0,
                .status = &status,
                .print_invisible = print_invisible
        };

        for (size_t i = 0; i < fds.size(); ++i) {
            params.fd = fds[i];
            params.filename = filenames[i];

            int res = io::cat_file(params);

            if (res == IO_FAILURE) {
                exit_code = EXIT_FAILURE;
                break;
            }

        }
    }

    // close all files
    for (size_t i = 0; i < fds.size(); ++i) {
        int res = io::close_file(fds[i], &status);

        if (res == IO_FAILURE) {
            std::stringstream ss;
            ss << "Error while closing file '" << filenames[i] << "' (status: " << status << ")\n";

            io::write_buffer(STDERR_FILENO, ss.str(), &status);
        }
    }
    
    return exit_code;
}
