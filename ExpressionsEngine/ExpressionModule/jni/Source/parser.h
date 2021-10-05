#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <string.h>
#include <vector>
#include "errors.h"

double to_number(const std::string &s)
{
    std::istringstream ist{s};
    ist.exceptions(std::ios_base::failbit);
    double x;
    ist >> x;
    return x;
}

std::string to_string(double x)
{
    std::ostringstream ost;
    ost << x;
    return ost.str();
}

template <int N>
std::ostream &operator<<(std::ostream &os, const Error<N> &e)
{
    e.put(os);
    return os;
}

// The basic elements of our expressions.
enum class Token
{
    Id,
    Number,
    Sin,
    Cos,
    Tan,
    Asin,
    Acos,
    Atan,
    Log,
    Exp,
    Log10,
    Exp10,
    Sqrt,
    Int,
    EqualTo,
    GreaterThanEqual,
    LessThanEqual,
    NotEqual,
    OR,
    AND,
    GreaterThan = '>',
    LessThan = '<',
    Assign = '=',
    Plus = '+',
    Minus = '-',
    Mul = '*',
    Div = '/',
    Mod = '%',
    Pow = '^',
    Lp = '(',
    Rp = ')',
    Not = '!',
    Concat = '~',
    Eofsym = -1
};

class Lexer
{
public:
    explicit Lexer(std::istream &is);
    explicit Lexer(std::istream *ps);

    // A Lexer belongs to a parser and shouldn't be copied or moved.

    Lexer(const Lexer &) = delete;
    Lexer &operator=(const Lexer &) = delete;

    Lexer(Lexer &&) = delete;
    Lexer &operator=(Lexer &&) = delete;

    ~Lexer()
    {
        if (owns_input)
            delete p_input;
    }

    Token get_current_token() const { return current_token; }
    std::string get_token_text() const { return current_token_text; }

    void advance(); // Read the next token in the stream.

private:
    std::istream *p_input; // The source stream (a stream of characters).
    bool owns_input;       // True if we can delete p_input, false if we can't.

    Token current_token;
    std::string current_token_text;

    void init(); // Code common to each constructor.

    Token get_token();        // The workhorse. Assembles characters from p_input into tokens.
    std::string token_buffer; // The text of the token that get_token() just found.

    void exponent_part(char &c); // A helper function for get_token() when it is looking for a number.
};

Lexer::Lexer(std::istream &is)
    : p_input{&is}, owns_input{false}
{
    init();
}

Lexer::Lexer(std::istream *ps)
    : p_input{ps}, owns_input{false}
{
    init();
}

void Lexer::init()
{
    current_token = get_token();
    current_token_text = token_buffer;
}

void Lexer::advance()
{
    if (current_token != Token::Eofsym)
    {
        current_token = get_token();
        current_token_text = token_buffer;
    }
}

Token Lexer::get_token()
{
    std::istream &input = *p_input; // Shorthand to make the notation convenient.

    token_buffer.clear(); // Clear the buffer for the new token.

    char c = input.get(); // A priming read on the stream.

    // Skip whitespace.
    while (isspace(c))
        c = input.get();

    // If there are no characters, we're at the end of the stream.
    if (!input)
        return Token::Eofsym;

    // Look for an identifier or function name.
    if (isalpha(c))
    { // alphanumeric{
        token_buffer = c;
        c = input.get();

        // Look for zero or more letters or digits.
        while (isalnum(c))
        {
            token_buffer += c;
            c = input.get();
        }

        // The current character doesn' belong to our identifier.
        input.putback(c);
        // dont want to support this
        // Check for a function name.
        if (token_buffer == "and")
            return Token::AND;
        if (token_buffer == "or")
            return Token::OR;
        if (token_buffer == "sin")
            return Token::Sin;
        if (token_buffer == "cos")
            return Token::Cos;
        if (token_buffer == "tan")
            return Token::Tan;
        if (token_buffer == "asin")
            return Token::Asin;
        if (token_buffer == "acos")
            return Token::Acos;
        if (token_buffer == "atan")
            return Token::Atan;
        if (token_buffer == "log")
            return Token::Log;
        if (token_buffer == "exp")
            return Token::Exp;
        if (token_buffer == "log10")
            return Token::Log10;
        if (token_buffer == "exp10")
            return Token::Exp10;
        if (token_buffer == "sqrt")
            return Token::Sqrt;
        if (token_buffer == "int")
            return Token::Int;

        //Whatever is not a function name must be an identifier.
        return Token::Id;
    }

    // Look for a number beginning with a digit.
    if (isdigit(c))
    {
        token_buffer = c;
        c = input.get();

        // Look for other digits.
        while (isdigit(c))
        {
            token_buffer += c;
            c = input.get();
        }

        // Look for an optional decimal point.
        // If there is one, it can be followed by zero or more digits.
        if (c == '.')
        {
            token_buffer += c;
            c = input.get();

            while (isdigit(c))
            {
                token_buffer += c;
                c = input.get();
            }
        }

        // Look for an optional exponent part.
        exponent_part(c);

        input.putback(c);
        return Token::Number;
    }

    // Look for a number beginning with a decimal point.
    if (c == '.')
    {
        token_buffer = c;
        c = input.get();

        // A decimal point must be followed by a digit. Otherwise we have an error.
        if (!isdigit(c))
        {
            throw Lexical_error{token_buffer += c};
        }
        while (isdigit(c))
        {
            token_buffer += c;
            c = input.get();
        }

        // Check for the optional exponent part.
        exponent_part(c);

        input.putback(c);
        return Token::Number;
    }

    // Check for a single character token.
    token_buffer = c;
    switch (c)
    {
    // Note: fallthrough intentional.
    case '=':
        c = input.get();
        if (c == '=')
            return Token::EqualTo;
        input.putback(c);
        throw Lexical_error{token_buffer};
    case '<':
        c = input.get();
        if (c == '=')
            return Token::LessThanEqual;
        input.putback(c);
        return Token::LessThan;
    case '>':
        c = input.get();
        if (c == '=')
            return Token::GreaterThanEqual;
        input.putback(c);
        return Token::GreaterThan;
    case '!':
        c = input.get();
        if (c == '=')
            return Token::NotEqual;
        input.putback(c);
        throw Lexical_error{token_buffer};
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '^':
    case '(':
    case ')':
    case '~':
        return Token(c);
    }

    // Anything else is an error.
    throw Lexical_error{token_buffer};
}

void Lexer::exponent_part(char &c)
{
    std::istream &input = *p_input;

    if (c != 'e' || c != 'E')
        return;

    token_buffer += c;
    c = input.get();

    // Check for an optional sign.
    if (c == '+' || c == '-')
    {
        token_buffer += c;
        c = input.get();
    }

    // We must have a digit. Otherwise, we have an error.
    if (!isdigit(c))
        throw Lexical_error{token_buffer += c};
    while (isdigit(c))
    {
        token_buffer += c;
        c = input.get();
    }
}

// std::map<std::string, double> symbol_table;

double join_number(double x, double y)
{
    if (trunc(x) != x || trunc(y) != y)
    {
        // do some joining
        throw Runtime_error{"Attempting to Join two non-integer numbers ( Integer~Integer )"};
    };
    // also we will limit join to max
    std::string first = to_string(int(x));
    std::string second = to_string(int(y));
    std::string result = first + second;
    return double(stoi(result));
}

class Parser
{
public:
    Parser();
    double operator()(const std::string &s);
    void add_symbol(const std::string &s, double v);
    void get_ids(const std::string &s, std::vector<std::string> &list);

private:
    Lexer *p_lexer;

    std::map<std::string, double> symbol_table;
    double choice_expr();
    double combination_expr();
    double equality_expr();
    double assign_expr();
    double add_expr();
    double mul_expr();
    double pow_expr();
    double unary_expr();
    double concat();
    double primary();

    double get_argument();

    // Check for root of a negative number.
    static void check_domain(double x, double y);
};

Parser::Parser()
{
    symbol_table["pi"] = 4.0 * atan(1.0);
    symbol_table["e"] = exp(1.0);
}

double Parser::operator()(const std::string &s)
{
    std::istringstream ist{s};
    p_lexer = new Lexer{ist};
    double result = choice_expr();
    delete p_lexer;
    return result;
}

void Parser::get_ids(const std::string &s, std::vector<std::string> &list)
{
    std::istringstream ist{s};
    p_lexer = new Lexer{ist};
    std::string text;
    for (;;)
    {

        text = p_lexer->get_token_text();

        switch (p_lexer->get_current_token())
        {
        case Token::Id:
            p_lexer->advance();

            list.push_back(text);
            break;
        case Token::Eofsym:
            return;
        default:
            p_lexer->advance();
        }
    }
}

void Parser::add_symbol(const std::string &s, double value)
{
    symbol_table[s] = value;
}

double Parser::assign_expr()
{
    Token t = p_lexer->get_current_token();
    std::string text = p_lexer->get_token_text();

    double result = add_expr();

    if (p_lexer->get_current_token() == Token::Assign)
    {
        if (t != Token::Id)
            throw Syntax_error{"target of assignment must be an identifier"};

        if (text == "pi" || text == "e")
            throw Syntax_error{"attempt to modify the constant " + text};

        p_lexer->advance();
        return symbol_table[text] = add_expr();
    }

    return result;
}

double Parser::choice_expr()
{
    double result = combination_expr();
    for (;;)
    {
        switch (p_lexer->get_current_token())
        {
        case Token::AND:
            p_lexer->advance();
            result = result || combination_expr();
            break;
        default:
            return result;
        }
    }
}

double Parser::combination_expr()
{
    double result = equality_expr();
    for (;;)
    {
        switch (p_lexer->get_current_token())
        {
        case Token::AND:
            p_lexer->advance();
            result = result && equality_expr();
            break;
        default:
            return result;
        }
    }
}

double Parser::equality_expr()
{
    double result = add_expr();
    for (;;)
    {
        switch (p_lexer->get_current_token())
        {
        case Token::EqualTo:
            p_lexer->advance();
            result = result == add_expr();
            break;
        case Token::GreaterThanEqual:
            p_lexer->advance();
            result = result >= add_expr();
            break;
        case Token::LessThanEqual:
            p_lexer->advance();
            result = result <= add_expr();
            break;
        case Token::GreaterThan:
            p_lexer->advance();
            result = result > add_expr();
            break;
        case Token::LessThan:
            p_lexer->advance();
            result = result < add_expr();
            break;
        case Token::NotEqual:
            p_lexer->advance();
            result = result != add_expr();
            break;
        default:
            return result;
        }
    }
}

double Parser::add_expr()
{
    double result = mul_expr();

    for (;;)
    {
        switch (p_lexer->get_current_token())
        {
        case Token::Plus:
            p_lexer->advance();
            result += mul_expr();
            break;
        case Token::Minus:
            p_lexer->advance();
            result -= mul_expr();
        default:
            return result;
        }
    }
}

double Parser::mul_expr()
{
    double result = pow_expr();
    double x;

    for (;;)
    {
        switch (p_lexer->get_current_token())
        {
        case Token::Mul:
            p_lexer->advance();
            result *= pow_expr();
            break;
        case Token::Div:
            p_lexer->advance();
            x = pow_expr();
            if (x == 0)
                throw Runtime_error{"attempt to divide by zero"};
            result /= x;
            break;
        case Token::Mod:
            p_lexer->advance();
            x = pow_expr();
            if (x == 0)
                throw Runtime_error{"attempt to divide by zero"};
            result = fmod(result, x);
            break;
        default:
            return result;
        }
    }
}

double Parser::pow_expr()
{
    double result = unary_expr();

    if (p_lexer->get_current_token() == Token::Pow)
    {
        p_lexer->advance();
        double x = unary_expr();
        check_domain(result, x);
        return pow(result, x);
    }

    return result;
}

double Parser::unary_expr()
{
    switch (p_lexer->get_current_token())
    {
    case Token::Plus:
        p_lexer->advance();
        return +concat();
    case Token::Minus:
        p_lexer->advance();
        return -concat();
    default:
        return concat();
    }
}

double Parser::concat()
{
    double result = primary();
    for (;;)
    {
        switch (p_lexer->get_current_token())
        {
        case Token::Concat:
            // is result a valid number ( // concat can only be performed on two numbers )
            // first we should check that the token before the operator was an id
            p_lexer->advance();
            result = join_number(result, primary());
            break;
        default:
            return result;
        }
    }
}

double Parser::primary()
{
    std::string text = p_lexer->get_token_text();
    double arg;

    switch (p_lexer->get_current_token())
    {
    case Token::Id:
        p_lexer->advance();
        double symbol;
        try
        {
            symbol = symbol_table.at(text); //symbol_table[text];
        }
        catch (std::out_of_range exception)
        {
            throw Syntax_error{"Symbol " + text + " not found"};
        }
        return symbol;
    case Token::Number:
        p_lexer->advance();
        return to_number(text);
    case Token::Lp:
        p_lexer->advance();
        arg = add_expr();
        if (p_lexer->get_current_token() != Token::Rp)
            throw Syntax_error{"missing ) after subexpression"};
        p_lexer->advance();
        return arg;
    case Token::Sin:
        return sin(get_argument());
    case Token::Cos:
        return cos(get_argument());
    case Token::Tan:
        arg = get_argument();
        if (cos(arg) == 0)
            throw Runtime_error{"invalid argument to tan: " + to_string(arg)};
        return tan(arg);
    case Token::Asin:
        return asin(get_argument());
    case Token::Acos:
        return acos(get_argument());
    case Token::Atan:
        return atan(get_argument());
    case Token::Log:
        arg = get_argument();
        if (arg < 1)
            throw Runtime_error{"invalid argument to log: " + to_string(arg)};
        return log(arg);
    case Token::Exp:
        return exp(get_argument());
    case Token::Log10:
        arg = get_argument();
        if (arg < 1)
            throw Runtime_error{"invalid argument to log10: " + to_string(arg)};
        return log10(arg);
    // case Token::Exp10:
    //     return exp10(get_argument());
    case Token::Sqrt:
        arg = get_argument();
        if (arg < 0)
            throw Runtime_error{"attempt to take square root of negative number"};
        return sqrt(arg);
    case Token::Int:
        arg = get_argument();
        if (arg < 0)
            return ceil(arg);
        else
            return floor(arg);
    default:
        throw Syntax_error{"invalid primary expression"};
    }
}

void Parser::check_domain(double x, double y)
{
    // There is no error if x is nonnegative.
    if (x >= 0)
        return;

    // There is no error unless 0 < abs(y) < 1.
    double e = std::abs(y);
    if (e <= 0 || e >= 1)
        return;

    // We have an error.
    throw Runtime_error{"attempt to take root of a negative number"};
}

double Parser::get_argument()
{
    p_lexer->advance();
    if (p_lexer->get_current_token() != Token::Lp)
        throw Syntax_error{"missing ( after function name"};
    p_lexer->advance();
    double arg = add_expr();
    if (p_lexer->get_current_token() != Token::Rp)
        throw Syntax_error{"missing ) after function argument"};
    p_lexer->advance();
    return arg;
}

double evaluate(const std::string &s)
{
    Parser parser;
    // try
    // {
    std::cout << "string:" << s << "\n";
    return parser(s);
    // }
    // catch (const Lexical_error &e)
    // {
    //     std::cerr << "Lexical error: " << e << '\n';
    // }
    // catch (const Syntax_error &e)
    // {
    //     std::cerr << "Syntax error: " << s << e << '\n';
    // }
    // catch (const Runtime_error &e)
    // {
    //     std::cerr << "Runtime error: " << e << '\n';
    //     throw e;
    // }
    return 0;
}

#endif