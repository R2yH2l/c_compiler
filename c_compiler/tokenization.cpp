/*
* File: tokenization.cpp
* Author: R2yH2l
* Purpose: This file contains the implementation of a lexer for a C compiler.
* The lexer takes source code as input and breaks it down into individual tokens.
*/
#include "tokenization.h"

// This map stores the string representation for each 'token_type'
extern std::unordered_map<token_type, std::string, token_type_hash> token_type_string {
	{ token_type::unknown, "unknown" },
	{ token_type::integer_literal, "int" },
	{ token_type::identifier, "identifier" },
	{ token_type::keyword, "keyword" },
	{ token_type::type, "type" },
	{ token_type::symbol, "symbol" },
	{ token_type::semicolon, "semicolon" },
	{ token_type::open_paren, "open_paren" },
	{ token_type::close_paren, "close_paren" },
	{ token_type::open_brace, "open_brace" },
	{ token_type::close_brace, "close_brace" },
	{ token_type::equals, "equals" },
};

// This map stores the recognized keywords of the language and their corresponding token types.
extern std::unordered_map<std::string, token> keyword_tokens = {
	{ "int", { token_type::type, "int" }},
	{ "return", { token_type::keyword, "return" }}
};

// - TODO - Handle more specific use cases.
// create_token() is a function used to create tokens based on various conditions.
// It takes a token type, a string value, and the line and column number as parameters.
// It checks for various conditions based on the given input and returns a token.
token create_token(token_type type, std::string value, unsigned int line_num, unsigned int column_num) {
	if (type == token_type::unknown && !value.empty()) {
		token_type temp_type{ switch_punctuation(value[0]) };
		if (temp_type != token_type::unknown) {
			return { temp_type, value, line_num, column_num };
		}
		if (std::isdigit(value[0])) {
			return { token_type::integer_literal, value, line_num, column_num };
		}
		// Check if value is a keyword.
		auto iterator{ keyword_tokens.find(value) };
		if (iterator != keyword_tokens.end()) {
			iterator->second.line_num = line_num;
			iterator->second.column_num = column_num;
			return iterator->second;
		}
		else {
			// - TODO - more data validation.
			return { token_type::identifier, value, line_num, column_num };
		}
	}
	else {
		return { type, value, line_num, column_num };
	}
}

// tokenize_source() is the main lexer function that takes source code as input and breaks it down into individual tokens.
std::vector<token> tokenize_source(const std::string& source) {
	// Variable initializations
	std::vector<token> tokens{};         // Stores tokens read from the sorce code.
	state current_state{ state::start }; // Last know state before transition() call. Begin from state::start.
	std::string buffer{};                // Used to store chars read from source will forming a token.
	unsigned int
		line_num{ 1 },                   // Current line.
		column_num{},                    // Current column. Starts at 0 as its incramented before it's used.
		token_start_column{ 1 };         // I want tokens to have the column number of the char at value[0].

	// Process each character in the source code
	for (size_t index{}; index < source.size(); ++index) {
		char current_char{ source[index] };

		switch (current_state) {
		case state::start: {
			// ---- START ----
			// The Begining of an identifier.
			if (std::isalpha(current_char) || current_char == '_') {
				current_state = state::letter_encountered;
				buffer.push_back(current_char);
				break;
			}
			// Begining of a number.
			if (std::isdigit(current_char)) {
				current_state = state::number_encountered;
				buffer.push_back(current_char);
				break;
			}
			// Break and get next char.
			if (isspace(current_char)) {
				current_state = state::start;
				break;
			}
			// Begining of a symbol.
			if (std::ispunct(current_char)) {
				if (switch_punctuation(current_char) == token_type::unknown) {
					current_state = state::unknown;
					break;
				}
				current_state = state::punctuation_encountered;
				buffer.push_back(current_char);
				break;
			}
			// An error has occured
			current_state = state::unknown;
			break;
		}
		case state::letter_encountered: {
			// ---- LETTER ----
			// Still an identifier.
			if (std::isalnum(current_char) || current_char == '_') {
				current_state = state::letter_encountered;
				buffer.push_back(current_char);
				break;
			}
			// Complete identifier.
			if (isspace(current_char)) {
				tokens.push_back(create_token(token_type::unknown, buffer, line_num, token_start_column));
				buffer.clear();
				current_state = state::start;
				break;
			}
			// Complete identifier.
			if (std::ispunct(current_char)) {
				tokens.push_back(create_token(token_type::unknown, buffer, line_num, token_start_column));
				buffer.clear();
				if (switch_punctuation(current_char) == token_type::unknown) {
					current_state = state::unknown;
					break;
				}
				current_state = state::punctuation_encountered;
				buffer.push_back(current_char);
				break;
			}
			// An error has occured.
			current_state = state::unknown;
			break;
		}
		case state::number_encountered: {
			// ---- NUMBER ----
			// Still a number.
			if (std::isdigit(current_char)) {
				current_state = state::number_encountered;
				buffer.push_back(current_char);
				break;
			}
			// Complete number.
			if (std::isalpha(current_char)) {
				tokens.push_back({ token_type::integer_literal, buffer, line_num, token_start_column });
				buffer.clear();
				current_state = state::letter_encountered;
				buffer.push_back(current_char);
			}
			// Complete number.
			if (isspace(current_char)) {
				tokens.push_back({ token_type::integer_literal, buffer, line_num, token_start_column });
				buffer.clear();
				current_state = state::start;
				break;
			}
			// Complete number. - TODO - Add support for decimals.
			if (std::ispunct(current_char)) {
				tokens.push_back({ token_type::integer_literal, buffer, line_num, token_start_column });
				buffer.clear();
				if (switch_punctuation(current_char) == token_type::unknown) {
					current_state = state::unknown;
					break;
				}
				current_state = state::punctuation_encountered;
				buffer.push_back(current_char);
				break;
			}
			// An error has occured.
			current_state = state::unknown;
			break;
		}
		case::state::punctuation_encountered: {
			// ----  PUNCTUATION ----
			// - TODO - Add support for multi char symbols such as ++ | -- | == | ect.
			// Complete symbol.
			if (std::ispunct(current_char)) {
				tokens.push_back({ switch_punctuation(buffer[0]), buffer, line_num, token_start_column });
				buffer.clear();
				if (switch_punctuation(current_char) == token_type::unknown) {
					current_state = state::unknown;
					break;
				}
				current_state = state::punctuation_encountered;
				buffer.push_back(current_char);
				break;
			}
			// Complete symbol.
			if (std::isalpha(current_char)) {
				tokens.push_back({ switch_punctuation(buffer[0]), buffer, line_num, token_start_column });
				buffer.clear();
				current_state = state::letter_encountered;
				buffer.push_back(current_char);
				break;
			}
			// Complete symbol.
			if (std::isdigit(current_char)) {
				tokens.push_back({ switch_punctuation(buffer[0]), buffer, line_num, token_start_column });
				buffer.clear();
				current_state = state::number_encountered;
				buffer.push_back(current_char);
				break;
			}
			// Complete symbol.
			if (std::isspace(current_char)) {
				tokens.push_back({ switch_punctuation(buffer[0]), buffer, line_num, token_start_column });
				buffer.clear();
				current_state = state::start;
				break;
			}
			// An error has occured.
			current_state = state::unknown;
			break;
		}
		case state::unknown: {
			// ---- UNKNOW ----
			// Error handling for unknown state
			std::cerr << "[!] The lexer is in an unknow state. This was caused by the charecter '" << current_char << "' on line: " << line_num << " column: " << column_num << ".\n";
			tokens.erase(tokens.begin(), tokens.end());
			return tokens;
		}
		}

		// Line and column number tracking
		if (current_char == '\n') {
			++line_num;
			column_num = 1;
		}
		else {
			++column_num;
		}

		// Check for start of a new token
		if (current_state == state::start && !isspace(current_char)) {
			token_start_column = column_num;
		}
	}

	if (!buffer.empty()) {
		tokens.push_back(create_token(token_type::unknown, buffer, line_num, token_start_column));
	}

	// Return final list of tokens
	return tokens;
}

// switch_punctuation() is a function that maps punctuation characters to their corresponding token types.
// It takes a character as input and returns the corresponding token type.
token_type switch_punctuation(const char input) {
	switch (input) {
	case ';':
		return token_type::semicolon;
	case '(':
		return token_type::open_paren;
	case ')':
		return token_type::close_paren;
	case '{':
		return token_type::open_brace;
	case '}':
		return token_type::close_brace;
	case '=':
		return token_type::equals;
	}
	return token_type::unknown;
}