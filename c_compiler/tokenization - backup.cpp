/* C Compiler
* Author: R2yH2l
* Purpose: Lexer for C
*/

#include "tokenization.h"

// Define map of recognized keyword tokens.
std::unordered_map<std::string, token> keyword_tokens = {
	{ "int", { token_type::keyword, "int" }},
	{ "return", { token_type::keyword, "return" }}
};

/* Function to tokenize source code
*  This is a lexer function that reads an input source code and breaks it down into identifiable tokens.
*/
std::vector<token> tokenize_source(const std::string& source) {
	// Initializations
	std::vector<token> tokens{};         // Stores tokens read from the sorce code.
	token punctuation_token{};           // Temporary storage for punctuation.
	state current_state{ state::start }; // Last know state before transition() call. Begin from state::start.
	std::stack<state> states{};          // Holds states retured form transition().
	std::string buffer{};                // Used to store chars read from source will forming a token.
	unsigned int
		line_num{ 1 },                   // Current line.
		column_num{},                    // Current column. Starts at 0 as its incramented before it's used.
		token_start_column{ 1 };         // I want tokens to have the colmn for it's values starting char.
	bool insert_punctuation{ false };    // Dose a punctuation need to be added?

	// Process each character in the source code
	for (size_t i{}; i < source.size(); ++i) {
		char c{ source[i] };

		// Line and column number tracking
		if (c == '\n') {
			++line_num;
			column_num = 1;
		}
		else {
			++column_num;
		}

		// Check for start of a new token
		if (current_state == state::start && !isspace(c)) {
			token_start_column = column_num;
		}

		buffer.push_back(source[i]);
		states = transition(current_state, source[i]);

		// Process each state in the state stack
		for (unsigned int size{}; size < states.size();) {
			switch (states.top()) {
			case state::letter_encountered:
				current_state = states.top();
				break;
			case state::number_encountered:
				current_state = states.top();
				break;
			case::state::punctuation_encountered:
			{
				std::string punctuation{ buffer[buffer.size() - 1] };
				if (switch_punctuation(punctuation[0]) == token_type::unknown) {
					states.push(state::unknown);
					break;
				}
				/* Punctuation is inserted into tokens after the stack is cleared.
				*  Punctuation is used as a delimiter so it's the last char in buffer and the first state evaluated.
				*  To remedy this punctuation is temporarily stored and inserted once all states are cleared.
				*/
				insert_punctuation = true;
				punctuation_token.type = switch_punctuation(punctuation[0]);
				punctuation_token.value = punctuation;
				punctuation_token.line_num = line_num;
				punctuation_token.column_num = column_num;
				buffer.pop_back();
				current_state = state::start;
				break;
			}
			case::state::space_encountered:
			{
				buffer.pop_back();
				current_state = state::start;
				break;
			}
			case state::integer_literal:
				tokens.push_back({ token_type::integer_literal, buffer, line_num, token_start_column });
				buffer.clear();
				current_state = state::start;
				break;
			case state::identifier:
			{
				auto it{ keyword_tokens.find(buffer.c_str()) };
				if (it != keyword_tokens.end()) {
					it->second.line_num = line_num;
					it->second.column_num = token_start_column;
					tokens.push_back(it->second);
				}
				else {
					tokens.push_back({ token_type::identifier, buffer, line_num, token_start_column });
				}
				buffer.clear();
				current_state = state::start;
				break;
			}
			case state::unknown:
				// Error handling for unknown state
				if (!buffer.empty()) {
					std::printf("[!] The lexer is in an unknow state. This was caused by the charecter '%c' on line: '%u' column: %u.\n", buffer[buffer.size() - 1], line_num, column_num);
				}
				else {
					std::printf("[!] The lexer is in an unknown state due to an unexpected character at line: '%u' column: %u.\n", line_num, column_num);
				}
				tokens.erase(tokens.begin(), tokens.end());
				return tokens;
			}
			states.pop();
		}

		/* Correct the order of punctuation.
		*  Punctuation gets evaluated before keywords or identifiers.
		*  This is so it can be removed from the buffer before buffer is used as a value.
		*/
		if (insert_punctuation) {
			insert_punctuation = false;
			tokens.push_back(punctuation_token);
		}
	}

	// Return final list of tokens
	return tokens;
}

// State transition function
std::stack<state> transition(state current_state, const char input) {
	std::stack<state> states{};
	switch (current_state) {
		// Different cases based on current state and input
	case state::start:
		if (std::isalpha(input)) {
			states.push(state::letter_encountered);
			return states;
		}
		if (std::isdigit(input)) {
			states.push(state::number_encountered);
			return states;
		}
		if (isspace(input)) {
			states.push(state::space_encountered);
			return states;
		}
		if (std::ispunct(input)) {
			states.push(state::punctuation_encountered);
			return states;
		}
		break;
		// Additional cases based on next possible states
	case state::letter_encountered:
		if (std::isalnum(input)) {
			states.push(state::letter_encountered);
			return states;
		}
		if (isspace(input)) {
			states.push(state::identifier);
			states.push(state::space_encountered);
			return states;
		}
		if (std::ispunct(input)) {
			states.push(state::identifier);
			states.push(state::punctuation_encountered);
			return states;
		}
		break;
	case state::number_encountered:
		if (std::isdigit(input)) {
			states.push(state::number_encountered);
			return states;
		}
		if (isspace(input)) {
			states.push(state::integer_literal);
			states.push(state::space_encountered);
			return states;
		}
		if (std::ispunct(input)) {
			states.push(state::integer_literal);
			states.push(state::punctuation_encountered);
			return states;
		}
		break;
		states.push(current_state);
		return states;
	}
	// Default: unknown state
	states.push(state::unknown);
	return states;
}

// Function to map punctuation characters to token types
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