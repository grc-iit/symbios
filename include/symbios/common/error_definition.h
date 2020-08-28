//
// Created by Jie on 8/27/20.
//

#ifndef SYMBIOS_ERROR_CODE_H
#define SYMBIOS_ERROR_CODE_H

#include <string>
#include <exception>

typedef struct ErrorCode{
private:
    int code_;
    std::string message_;

public:
    ErrorCode(int code, std::string message): code_(code), message_(message){}

    int getCode() const {return code_;}
    std::string getMessage() const {return message_;}
} ErrorCode;

class ErrorException: public std::exception{
public:
    ErrorException(ErrorCode errorcode): errorCode_(errorcode){}

    const char* what() const throw() {
        std:: string error_message_ = std::to_string(errorCode_.getCode());
        error_message_ += "\n\t";
        error_message_ += errorCode_.getMessage();

        return error_message_.c_str();
    }

private:
    ErrorCode errorCode_;
};

#endif //SYMBIOS_ERROR_CODE_H

