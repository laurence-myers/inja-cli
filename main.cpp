#include <filesystem>
#include <iostream>

#ifdef WIN32
#define NOMINMAX

#include <windows.h>

#endif

#include <nowide/args.hpp>
#include <nowide/iostream.hpp>
#include <nowide/convert.hpp>
#include <inja/inja.hpp>

enum class ExitCode : uint8_t {
    Okay,
    MissingArg,
    InvalidArg,
    UnhandledError
};

boolean isExistingFile(std::filesystem::path &path) {
    if (!std::filesystem::exists(path)) {
        nowide::cerr << "File does not exist: " << nowide::narrow(path.wstring()) << std::endl;
        return false;
    }
    if (!std::filesystem::is_regular_file(path)) {
        nowide::cerr << "Given path is not a file: " << nowide::narrow(path.wstring()) << std::endl;
        return false;
    }
    return true;
}

int main(int argc, char *argv[]) {
    try {
        nowide::args args(argc, argv);

#ifdef WIN32
        SetConsoleCP(65001); // UTF-8 on Windows
#endif

        if (argc < 3) {
            nowide::cerr << "Please pass the input and output file paths.";
            return static_cast<int>(ExitCode::MissingArg);
        }

        std::filesystem::path inputPath {nowide::widen(argv[1])};
        std::filesystem::path outputPath {nowide::widen(argv[2])};

        if (!isExistingFile(inputPath)) {
            nowide::cerr << "Input file not found.";
            return static_cast<int>(ExitCode::InvalidArg);
        }

        std::filesystem::create_directories(outputPath.parent_path());

        inja::Environment env;
        inja::json data; // dummy data

        env.write(inputPath.string(), data, outputPath.string());

        return static_cast<int>(ExitCode::Okay);
    } catch (std::exception &err) {
        nowide::cerr << err.what();
        return static_cast<int>(ExitCode::UnhandledError);
    }
}
