#include <filesystem>
#include <iostream>

#ifdef WIN32
#define NOMINMAX

#include <windows.h>

#else

# Needed for shouldReadFromStdIn()
#include <stdio.h>
#include <unistd.h>

#endif

#include <nowide/args.hpp>
#include <nowide/iostream.hpp>
#include <nowide/convert.hpp>
#include <inja/inja.hpp>
#include <io.h>

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

boolean shouldReadFromStdIn() {
#ifdef WIN32
    return !_isatty(_fileno(stdin));
#else
    return !isatty(fileno(stdin)); // TODO: verify this implementation
#endif
}

int main(int argc, char *argv[]) {
    try {
        nowide::args args(argc, argv);

#ifdef WIN32
        SetConsoleCP(65001); // UTF-8 on Windows
#endif

        /*
         * - Check if stdin is a tty.
         * - If it is, read both input and output args.
         * - If it's not, treat stdin as input and stdout as output.
         */
        if (shouldReadFromStdIn()) {
            /**
             * Load the whole input stream into memory. Inja only works with strings.
             */
            std::string templateString {};
            std::cin >> templateString;

            /**
             * Render the template to a string.
             */
            inja::Environment env;
            inja::json data; // dummy data
            inja::Template aTemplate = env.parse_template(templateString);
            std::string result = env.render(templateString, data);
            nowide::cout << result << std::endl;
        } else if (argc < 3) {
            nowide::cerr << "Please pass the input and output file paths.";
            return static_cast<int>(ExitCode::MissingArg);
        } else {
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
        }

        return static_cast<int>(ExitCode::Okay);
    } catch (std::exception &err) {
        nowide::cerr << err.what();
        return static_cast<int>(ExitCode::UnhandledError);
    }
}
