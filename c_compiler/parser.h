#ifndef PARSER_H
#define PARSER_H

#include "tokenization.h"
#include "ast.h"

class token_parser {
	const std::vector<token> tokens{};
	size_t current_index{};

public:
	token_parser(std::vector<token>& tokens)
		: tokens(tokens) {}

	std::optional<std::unique_ptr<program>> parse_program();

	std::optional<std::unique_ptr<function_definition>> parse_function_definition();

	std::optional<std::unique_ptr<compound_statement>> parse_compound_statement();

	std::optional<std::unique_ptr<declaration_statement>> parse_declaration_statement();

	std::optional<std::unique_ptr<return_statement>> parse_return_statement();

	std::optional<std::unique_ptr<expression>> parse_expression();

	std::pair<size_t, token> find_next_token(const size_t& start_index, const std::vector<token_type>& types);

	bool token_type_matches(const token_type& type, const std::vector<token_type>& match_types);
};

#endif // !PARSER_H