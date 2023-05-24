/* c_compiler
* Author: R2yH2l
* 
*/

#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>

enum class token_type {
	keyword_int,
	keyword_return,
	identifier,
	punctuation,
	integer_literal,
};

struct token {
	token_type type{};
	std::string value{};
};

// Base class for all AST nodes
class ast_node {
public:
	virtual ~ast_node() = default;
};

// Represents a constant (like "2")
class constant : public ast_node{
public:
	int value;

	constant(int value) : value(value) {}
};

// Represents a return statement
class return_statement : public ast_node {
public:
	std::unique_ptr<constant> expression;

	return_statement(std::unique_ptr<constant> expression) : expression(std::move(expression)) {}
};

// Represents a block of code
class compound_statement : public ast_node {
public:
	std::unique_ptr<return_statement> statement;

	compound_statement(std::unique_ptr<return_statement> statement) : statement(std::move(statement)) {}
};

// Represents a function definition
class function_definition : public ast_node {
public:
	std::string return_type;
	std::string name;
	std::unique_ptr<compound_statement> body;

	function_definition(std::string return_type, std::string name, std::unique_ptr<compound_statement> body)
		: return_type(return_type), name(name), body(std::move(body)) {}
};

// Represents a program, which consists of a sequence of function definitions
class program : public ast_node {
public:
	std::vector<std::unique_ptr<function_definition>> functions;

	void add_function(std::unique_ptr<function_definition> function) {
		functions.push_back(std::move(function));
	}
};

std::vector<token> lex(const std::string& source);

program parse(std::vector<token> tokens);

int main() {
	std::fstream file("test.c");
	if (!file.good()) {
		return 1;
	}

	std::string source{
		std::istreambuf_iterator<char>(file),
		std::istreambuf_iterator<char>()};

	// source = strip(source);

	std::vector<token> tokens{ lex(source) };

	program prog{ parse(tokens) };

	return 0;
}

// Function that performs lexical analysis on C source code.
std::vector<token> lex(const std::string& source) {
	// Initialize an empty vector to store the tokens.
	std::vector<token> tokens{};

	// Iterate over each character in the source string.
	for (size_t i{}; i < source.size(); ++i) {
		// If the character is a space or a control character, skip it.
		if (std::isspace(source[i])) {
			continue;
		}

		// Check for 'int' keyword.
		if (source.substr(i, 3) == "int") {
			tokens.push_back({ token_type::keyword_int, "int" });
			i += 2;
			continue;
		}

		// Check for 'return' keyword.
		if (source.substr(i, 6) == "return") {
			tokens.push_back({ token_type::keyword_return, "return" });
			i += 5;
			continue;
		}

		// Check for punctuation characters.
		if (source[i] == '(' || source[i] == ')' || source[i] == '{' || source[i] == '}' || source[i] == ';') {
			tokens.push_back({ token_type::punctuation, std::string(1, source[i]) });
			continue;
		}

		// Check for integer literals.
		if (std::isdigit(source[i])) {
			size_t j{ i };
			// Continue until a non-digit character is found.
			while (j < source.size() && std::isdigit(source[j])) {
				j++;
			}
			tokens.push_back({ token_type::integer_literal, source.substr(i, j - i) });
			i = j - 1;
			continue;
		}

		// Check for identifiers (variable names).
		if ((source[i] >= 'a' && source[i] <= 'z') || (source[i] >= 'A' && source[i] <= 'Z') || (source[i] == '_')) {
			size_t j{ i };
			// Continue until a non-alphanumeric and non-underscore character is found.
			while (j < source.size() && std::isalnum(source[j]) || source[j] == '_') {
				j++;
			}
			tokens.push_back({ token_type::identifier, source.substr(i, j - i) });
			i = j - 1;
			continue;
		}
	}

	// Return the list of tokens.
	return tokens;
}

program parse(std::vector<token> tokens) {
	program prog{};

	std::unique_ptr<function_definition> function{
		std::make_unique<function_definition>(
			tokens[0].value,
			tokens[1].value,
			std::make_unique<compound_statement>(
				std::make_unique<return_statement>(
					std::make_unique<constant>(std::stoi(tokens[6].value.c_str()))
				)
			)
		)
	};

	prog.add_function(std::move(function));

	return prog;
}