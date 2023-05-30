/* c_compiler
* Author: R2yH2l
* 
*/

#include "tokenization.h"
#include "parser.h"

#include <fstream>

void print_tokens(std::vector<token> tokens);

int main() {
	std::fstream file("test.c");
	if (!file.is_open()) {
		return 1;
	}

	std::string source{
		std::istreambuf_iterator<char>(file),
			std::istreambuf_iterator<char>()};

	file.close();

	std::vector<token> tokens{ tokenize_source(source) };

	print_tokens(tokens);

	token_parser tp{ tokens };

	auto prog{ tp.parse_program() };

	return 0;
}

void print_tokens(std::vector<token> tokens) {
	for (auto& tok : tokens) {
		std::printf("{ '%s', '%u', Ln: '%u', Ch: '%u' }\n", tok.value.c_str(), tok.type, tok.line_num, tok.column_num);
	}
}