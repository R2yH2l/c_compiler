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
	int value{};

	constant(int value) : value(value) {}
};

// Represents a return statement
class return_statement : public ast_node {
public:
	std::unique_ptr<constant> expression{};

	return_statement(std::unique_ptr<constant> expression) : expression(std::move(expression)) {}
};

// Represents a block of code
class compound_statement : public ast_node {
public:
	std::unique_ptr<return_statement> statement{};

	compound_statement(std::unique_ptr<return_statement> statement) : statement(std::move(statement)) {}
};

// Represents a function definition
class function_definition : public ast_node {
public:
	std::string return_type{};
	std::string name{};
	std::unique_ptr<compound_statement> body{};

	function_definition(std::string return_type, std::string name, std::unique_ptr<compound_statement> body)
		: return_type(return_type), name(name), body(std::move(body)) {}
};

// Represents a program, which consists of a sequence of function definitions
class program : public ast_node {
public:
	std::vector<std::unique_ptr<function_definition>> functions{};

	void add_function(std::unique_ptr<function_definition> function) {
		functions.push_back(std::move(function));
	}
};

class token_parser {
	const std::vector<token> tokens{};
	size_t current_index{};

public:
	token_parser(std::vector<token> tokens) : tokens(tokens) {}

	std::unique_ptr<program> parse() {
		// Parse a program...
		auto prog{ std::make_unique<program>() };

		std::unique_ptr<function_definition> func = parse_function_definition();

		prog->add_function(std::move(func));

		return prog;
	}

	std::unique_ptr<function_definition> parse_function_definition() {
		// Parse a function definition...
		size_t index_offset{ current_index };

		token retun_type{ tokens[index_offset] };

		if (!token_type_matches(tokens[index_offset++], { token_type::keyword_int })) {
			return nullptr;
		}

		token name{ tokens[index_offset] };

		if (!token_type_matches(tokens[index_offset++], { token_type::identifier })) {
			return nullptr;
		}

		index_offset = find_next_token(index_offset, { token_type::punctuation });

		if (index_offset == tokens.size()) {
			return nullptr;
		}

		if (tokens[index_offset].value != "(") {
			return nullptr;
		}

		index_offset = find_next_token(++index_offset, { token_type::punctuation });

		if (index_offset == tokens.size()) {
			return nullptr;
		}

		if (tokens[index_offset].value != ")") {
			return nullptr;
		}

		index_offset = find_next_token(++index_offset, { token_type::punctuation });

		if (index_offset == tokens.size()) {
			return nullptr;
		}

		if (tokens[index_offset].value != "{") {
			return nullptr;
		}

		size_t body_start{ index_offset };

		index_offset = find_next_token(++index_offset, { token_type::punctuation });

		if (index_offset == tokens.size()) {
			return nullptr;
		}

		index_offset = find_next_token(++index_offset, { token_type::punctuation });

		if (index_offset == tokens.size()) {
			return nullptr;
		}

		if (tokens[index_offset].value != "}") {
			return nullptr;
		}

		size_t body_end{ index_offset };

		index_offset = find_next_token(body_start, { token_type::keyword_return });

		if (index_offset > body_end) {
			std::printf("Error: %s must return a value.", name.value.c_str());
			return nullptr;
		}

		if (find_next_token(index_offset, { token_type::punctuation }) > body_end) {
			std::printf("Error: missing ';' after %s statement.", tokens[index_offset].value.c_str());
			return nullptr;
		}

		current_index = index_offset;

		return std::make_unique<function_definition>(retun_type.value, name.value, parse_compound_statement());
	}

	std::unique_ptr<compound_statement> parse_compound_statement() {
		// Parse a compound statement...
		size_t index_offset{ current_index };

		if (token_type_matches(tokens[index_offset++], { token_type::keyword_return })) {
			return std::make_unique<compound_statement>(parse_return_statement());
		}

		return nullptr;
	}

	std::unique_ptr<return_statement> parse_return_statement() {
		// Parse a return statement...
		size_t index_offset{ current_index };

		if (!token_type_matches(tokens[index_offset++], { token_type::keyword_return })) {
			return nullptr;
		}

		if (!token_type_matches(tokens[index_offset++], { token_type::integer_literal })) {
			return nullptr;
		}

		if (!(token_type_matches(tokens[index_offset], { token_type::punctuation })) || !(tokens[index_offset].value == ";")) {
			return nullptr;
		}

		current_index = index_offset - 1;

		return std::make_unique<return_statement>(parse_constant());
	}

	std::unique_ptr<constant> parse_constant() {
		// Parse a constant...
		if (!token_type_matches(tokens[current_index], { token_type::integer_literal })) {
			return nullptr;
		}

		return std::make_unique<constant>(std::stoi(tokens[current_index].value));
	}

	size_t find_next_token(size_t start_index, const std::vector<token_type>& types) {
		for (size_t i{ start_index }; i < tokens.size(); i++) {
			if (std::find(types.begin(), types.end(), tokens[i].type) != types.end()) {
				return i;
			}
		}

		return tokens.size();
	}

	bool token_type_matches(const token& tok, const std::vector<token_type>& types) {
		return std::find(types.begin(), types.end(), tok.type) != types.end();
	}
};

std::vector<token> lex(const std::string& source);

void print_tokens(const std::vector<token>& tokens);

int main() {
	std::fstream file("test.c");
	if (!file.good()) {
		return 1;
	}

	std::string source{
		std::istreambuf_iterator<char>(file),
		std::istreambuf_iterator<char>()};

	std::vector<token> tokens{ lex(source) };

	// print_tokens(tokens);

	token_parser parser{ tokens };

	auto prog = parser.parse();

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

void print_tokens(const std::vector<token>& tokens) {
	for (const auto& tok : tokens) {
		std::cout << "{ \"" << tok.value << "\", ";

		switch (tok.type) {
		case token_type::keyword_int:
			std::cout << "keyword_int";
			break;
		case token_type::keyword_return:
			std::cout << "keyword_return";
			break;
		case token_type::identifier:
			std::cout << "identifier";
			break;
		case token_type::punctuation:
			std::cout << "punctuation";
			break;
		case token_type::integer_literal:
			std::cout << "integer_literal";
			break;
		}

		std::cout << " }, ";
	}
	std::cout << std::endl;
}