#ifndef LIBRARY_EXCEPTION_H
#define LIBRARY_EXCEPTION_H

#include <stdexcept>
#include <string>

class LibraryException : public std::runtime_error {
public:
    explicit LibraryException(const std::string &errorMsg, const std::string &fun = "", unsigned int line = 0)
            : std::runtime_error(what2(errorMsg, fun, line)) {}

    static std::string what2(const std::string &errorMsg, const std::string &fun = "", unsigned int line = 0) {
        std::string error;
        error += errorMsg;
        error += "\n\tAT LINE: ";
        error += std::to_string(line);
        error += "\n\tIN FUNCTION: ";
        error += fun;

        return error;
    }
};

#endif //LIBRARY_EXCEPTION_H