#include <filesystem>
#include <io.h>
#include <iostream>

#ifdef WIN32
#define NOMINMAX

#include <windows.h>

#else

// Needed for shouldReadFromStdIn()
#include <stdio.h>
#include <unistd.h>

#endif

#include <nowide/args.hpp>
#include <nowide/iostream.hpp>
#include <nowide/convert.hpp>
#include <inja/inja.hpp>
#include <tclap/CmdLine.h>

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

constexpr boolean stdInOutEnabled { false }; // TODO: re-enable when tested & working

boolean shouldReadFromStdIn() {
    if (stdInOutEnabled) {
#ifdef WIN32
        return !_isatty(_fileno(stdin));
#else
        return !isatty(fileno(stdin)); // TODO: verify this implementation
#endif
    } else {
        return false;
    }
}

int main(int argc, char* argv[]) {
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
        } else {
            try {
                // Parse the command line args
                TCLAP::CmdLine cmd("inja", ' ', "0.1");

                TCLAP::UnlabeledValueArg<std::string> templateArg("template", "File path to template", true, "", "string", cmd);
                TCLAP::UnlabeledValueArg<std::string> outputArg("output", "File path to output", true, "", "string", cmd);

                TCLAP::ValueArg<std::string> dataArg("d", "data", "JSON data file", false, "", "string", cmd);

                cmd.parse(argc, argv);

                std::filesystem::path inputPath {nowide::widen(templateArg.getValue())};
                std::filesystem::path outputPath {nowide::widen(outputArg.getValue())};

                if (!isExistingFile(inputPath)) {
                    nowide::cerr << "Input file not found.";
                    return static_cast<int>(ExitCode::InvalidArg);
                }

                inja::Environment env;

                inja::json data;
                if (dataArg.isSet()) {
                    std::filesystem::path dataPath {nowide::widen(dataArg.getValue())};
                    if (!isExistingFile(dataPath)) {
                        nowide::cerr << "Data JSON file not found.";
                        return static_cast<int>(ExitCode::InvalidArg);
                    }
                    std::filesystem::create_directories(outputPath.parent_path());
                    env.write_with_json_file(inputPath.string(), dataPath.string(), outputPath.string());
                } else {
                    std::filesystem::create_directories(outputPath.parent_path());
                    env.write(inputPath.string(), data, outputPath.string());
                }
            } catch (TCLAP::ArgException &e) {  // catch exceptions
                std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
                return static_cast<int>(ExitCode::UnhandledError);
            }
        }

        return static_cast<int>(ExitCode::Okay);
    } catch (std::exception &err) {
        nowide::cerr << err.what();
        return static_cast<int>(ExitCode::UnhandledError);
    }
}
