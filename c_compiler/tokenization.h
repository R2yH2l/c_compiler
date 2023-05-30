/*
* File: tokenization.h
* Author: R2yH2l
* Purpose: This header file contains the definition of tokens, states, and related functions for a C compiler.
* These are essential components of the lexical analyzer (lexer) of the compiler.
*/

#ifndef TOKENIZATION_H
#define TOKENIZATION_H

#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <stack>

// token_type is an enumeration of all possible token types that the lexer can identify in the source code.
// This includes special symbols, identifiers, keywords, and literals.
enum class token_type {
    unknown,           // For unknown token types
    integer_literal,   // For integer literals
    identifier,        // For identifiers (variable names, function names, etc.)
    keyword,           // For programming language keywords
    type,              // For variables (int, float, char, ect.)
    symbol,            // For special symbols
    semicolon,         // For semicolon (;)
    open_paren,        // For open parenthesis (()
    close_paren,       // For close parenthesis ())
    open_brace,        // For open brace ({)
    close_brace,       // For close brace (})
    equals,            // For equals (=)
};

// state is an enumeration of all possible states that the lexer can be in while analyzing the source code.
enum class state {
    start,                     // Initial state
    letter_encountered,        // After encountering a letter
    number_encountered,        // After encountering a number
    punctuation_encountered,   // After encountering a punctuation
    unknown,                   // For any unhandled state or condition
};

// The struct token represents a token in the source code, which includes its type, value, and location (line and column numbers).
struct token {
    token_type type{};          // The type of token
    std::string value{};        // The string value of token
    unsigned int
        line_num{},
        column_num{};

    // Constructor for token with default values
    token(token_type type = token_type::unknown, std::string value = "", unsigned int line_num = 0, unsigned int column_num = 0)
        : type(type), value(value), line_num(line_num), column_num(column_num) {}
};

// Declare a custom hash function for the token_type enum class
struct token_type_hash {
    std::size_t operator()(token_type const& type) const noexcept {
        return static_cast<std::size_t>(type);
    }
};

// This map stores the string representation for each 'token_type'
extern std::unordered_map<token_type, std::string, token_type_hash> token_type_string;

// This map stores the recognized keywords of the language and their corresponding token types.
extern std::unordered_map<std::string, token> keyword_tokens;

// Function that creates a new token with the provided type, value, and location.
// Default values are 'unknown' for type, empty string for value, and 0 for both line and column numbers.
extern token create_token(token_type type = token_type::unknown, std::string value = "", unsigned int line_num = 0, unsigned int column_num = 0);

// Function that takes source code as input and returns a vector of tokens.
// This function performs the main task of the lexer: analyzing the source code and breaking it down into individual tokens.
extern std::vector<token> tokenize_source(const std::string& source);

// Function that takes a punctuation character as input and returns its corresponding token type.
// This function is used by the lexer to classify punctuation characters in the source code.
extern token_type switch_punctuation(const char input);

#endif // !TOKENIZATION_H
