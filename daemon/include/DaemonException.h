#ifndef LIBRARY_EXCEPTION_H
#define LIBRARY_EXCEPTION_H

#include <stdexcept>
#include <string>
#include <cstring>

class DaemonException : public std::runtime_error {
public:
    explicit DaemonException(const std::string &errorMsg, int erno, const std::string &fun = "", unsigned int line = 0)
            : std::runtime_error(what2(errorMsg, erno, fun, line)) {}

    static std::string what2(const std::string &errorMsg, int erno, const std::string &fun = "", unsigned int line = 0) {
        std::string error;
        error += errorMsg;
        error += "\n\tTYPE: ";
        error += strerror(erno);
        error += "\n\tAT LINE: ";
        error += std::to_string(line);
        error += "\n\tIN FUNCTION: ";
        error += fun;

        return error;
    }
};

#endif //LIBRARY_EXCEPTION_H