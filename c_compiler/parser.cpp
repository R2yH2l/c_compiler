#include "parser.h"

std::optional<std::unique_ptr<program>> token_parser::parse_program() {
	auto prog{ std::make_unique<program>() };

	std::optional<std::unique_ptr<function_definition>> func{ std::move(parse_function_definition()) };

	while (func != nullptr) {
		prog->add_function(std::move(func.value()));
	}

	// Check for main function.
	for (auto function_iterator{ prog->functions.begin() }; function_iterator != prog->functions.end(); ++function_iterator) {
		if (function_iterator->get()->name == "main") {
			return prog;
		}
	}

	return {};
}

std::optional<std::unique_ptr<function_definition>> token_parser::parse_function_definition() {
	size_t index_offset{ current_index };
	function_definition func{};

	if (tokens[index_offset].type != token_type::type) {
		return {};
	}

	func.return_type = tokens[index_offset].value;
	++index_offset;

	if (tokens[index_offset].type != token_type::identifier) {
		return {};
	}
	
	func.name = tokens[index_offset].value;
	++index_offset;

	if (tokens[index_offset].type != token_type::open_paren) {
		return {};
	}

	std::pair<size_t, token> found_token{ find_next_token(index_offset, { token_type::close_paren }) };

	if (found_token.first >= tokens.size()) {
		std::cerr << "[!] Unexpected end of tokens: expected ')'\n";
		return {};
	}

	index_offset = found_token.first + 1;

	// There is a function body
	if (tokens[index_offset].type == token_type::open_brace) {
		found_token = find_next_token(index_offset, { token_type::close_brace });

		if (found_token.first >= tokens.size()) {
			std::cerr << "[!] Unexpected end of tokens: expected ';' or '{'\n";
			return {};
		}

		current_index = index_offset;

		std::optional<std::unique_ptr<compound_statement>> compund{ parse_compound_statement() };

		if (!compund) {
			return {};
		}

		func.body = std::move(compund.value());

		return std::make_unique<function_definition>(std::move(func));
	}

	// - TODO - Handle function definition without a body.

	return {};
}

std::optional<std::unique_ptr<compound_statement>> token_parser::parse_compound_statement() {
	std::pair<size_t, token> found_token{ find_next_token(current_index, { token_type::keyword, token_type::type }) };

	if (found_token.first > tokens.size()) {
		std::cerr << "[!] Unexpected end of tokens: expected a keyword\n";
		return {};
	}

	size_t
		index_offset{ found_token.first },
		end{ find_next_token(current_index, { token_type::close_brace }).first };
	compound_statement compound{};

	while (index_offset < end) {
		if (tokens[index_offset].value == "int") {
			current_index = index_offset;

			std::optional<std::unique_ptr<declaration_statement>> declaration{ parse_declaration_statement() };

			index_offset = current_index;

			if (!declaration) {
				return {};
			}

			compound.add_declaration(std::move(declaration.value()));

			continue;
		}
		if (tokens[index_offset].value == "return") {
			current_index = index_offset;

			std::optional<std::unique_ptr<return_statement>> ret{ parse_return_statement() };

			if (!ret) {
				return {};
			}

			compound.ret = std::move(ret.value());

			index_offset = end;
		}

		++index_offset;
	}

	return std::make_unique<compound_statement>(std::move(compound));
}

std::optional<std::unique_ptr<declaration_statement>> token_parser::parse_declaration_statement() {
	size_t index_offset{ current_index };
	declaration_statement declaration{};

	if (tokens[index_offset].type != token_type::type) {
		std::cerr << "[!] Syntax error: expected type in declaration statement at token " << index_offset << "\n";
		return {};
	}

	declaration.type = tokens[index_offset].value;

	++index_offset;

	if (tokens[index_offset].type != token_type::identifier) {
		std::cerr << "[!] Syntax error: expected identifier in declaration statement at token " << index_offset << "\n";
		return {};
	}

	declaration.identifier = tokens[index_offset].value;

	++index_offset;

	if (tokens[index_offset].type == token_type::equals) {
		declaration.assignment_operation = tokens[index_offset].value;

		++index_offset;

		if (!token_type_matches(tokens[index_offset].type, { token_type::integer_literal })) {
			std::cerr << "[!] Syntax error: expected initializer in declaration statement at token " << index_offset << "\n";
			return {};
		}

		current_index = index_offset;

		std::optional<std::unique_ptr<expression>> expr{ parse_expression() };

		if (!expr) {
			return {};
		}

		if (declaration.type != expr.value().get()->type) {
			std::cerr << "[!] Syntax error: initializer and identifier type dose not match line " << index_offset << "\n";
		}

		declaration.value = std::move(expr.value());

		++index_offset;
	}

	if (tokens[index_offset].type != token_type::semicolon) {
		std::cerr << "[!] Syntax error: expected semicolon at the end of declaration statement at token " << index_offset << "\n";
		return {};
	}

	current_index = index_offset + 1;

	return std::make_unique<declaration_statement>(std::move(declaration));
}

std::optional<std::unique_ptr<return_statement>> token_parser::parse_return_statement() {
	size_t index_offset{ current_index };
	return_statement ret{};

	if (tokens[index_offset].value != "return") {
		std::cerr << "[!] Syntax error: expected keyword 'return' at token " << index_offset << "\n";
		return {};
	}

	++index_offset;

	if (!token_type_matches(tokens[index_offset].type, { token_type::integer_literal, token_type::identifier })) {
		std::cerr << "[!] Syntax error: expected initializer in declaration statement at token " << index_offset << "\n";
		return {};
	}

	current_index = index_offset;

	std::optional<std::unique_ptr<expression>> expr{ parse_expression() };

	if (!expr) {
		return {};
	}

	++index_offset;

	if (tokens[index_offset].type != token_type::semicolon) {
		std::cerr << "[!] Syntax error: expected semicolon at the end of declaration statement at token " << index_offset << "\n";
		return {};
	}

	ret.return_value = std::move(expr.value());

	return std::make_unique<return_statement>(std::move(ret));
}

std::optional<std::unique_ptr<expression>> token_parser::parse_expression() {
	size_t index_offset{ current_index };
	expression expr{};

	if (!token_type_matches(tokens[index_offset].type, { token_type::integer_literal, token_type::identifier })) {
		std::cerr << "[!] Syntax error: expected initializer in declaration statement at token " << index_offset << "\n";
		return {};
	}

	auto iterator{ token_type_string.find(tokens[index_offset].type) };
	if (iterator == token_type_string.end()) {
		std::cerr << "[!] Syntax error: no string conversion for 'token_type' at token " << index_offset << "\n";
		return {};
	}

	expr.type = iterator->second;
	expr.value = tokens[index_offset].value;

	return std::make_unique<expression>(expr);
}

std::pair<size_t, token> token_parser::find_next_token(const size_t& start_index, const std::vector<token_type>& types) {
	for (size_t i{ start_index }; i < tokens.size(); i++) {
		if (std::find(types.begin(), types.end(), tokens[i].type) != types.end()) {
			return { i, tokens[i] };
		}
	}

	return { tokens.size(), {} };
}

bool token_parser::token_type_matches(const token_type& type, const std::vector<token_type>& match_types) {
	return std::find(match_types.begin(), match_types.end(), type) != match_types.end();
}