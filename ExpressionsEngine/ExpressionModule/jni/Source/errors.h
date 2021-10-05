#ifndef ERROR_H
#define ERROR_H

#include <string>

template <int N>
class Error
{

public:
    explicit Error(const std::string s) : message{s} {};

    std::string get_message() const { return message; }
    void put(std::ostream &os) const { os << message; }

private:
    std::string message;
};

using Lexical_error = Error<1>;
using Syntax_error = Error<2>;
using Runtime_error = Error<3>;
using ParsingError = Error<4>;
using CyclicDependencyError = Error<5>;

#endif