/* c_compiler
* Author: R2yH2l
* 
*/

#include "tokenization.h"
#include "parser.h"

#include <fstream>

void generate_asm(std::optional<std::unique_ptr<program>> prog);

void print_tokens(std::vector<token> tokens);

int main() {
	std::fstream file("C:\\Users\\roryh\\source\\repos\\R2yH2l\\c_compiler\\c_compiler\\test.c");

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

	if (!prog) {
		return 1;
	}

	generate_asm(std::move(prog));

	// TODO: Remove use of system by using LLVM librarys.
	std::system("\"C:\\Program Files\\LLVM\\bin\\clang\" -v -x assembler test.asm -c -o test.o");
	std::system("\"C:\\Program Files\\LLVM\\bin\\clang\" test.o -o test.exe -lkernel32 -Wl,/ENTRY:_main,/SUBSYSTEM:CONSOLE");

	return 0;
}

void generate_asm(std::optional<std::unique_ptr<program>> prog) {
	std::fstream file("test.asm", std::ios_base::out);

	if (!file.is_open()) {
		std::cerr << "[!] file not open.\n";
		return;
	}

	// if (prog->global_declarations.size() != 0)

	file << ".section .data\n";

	for (size_t index{}; index < prog.value().get()->functions.size(); ++index) {
		for (size_t decl{}; decl < prog.value().get()->functions[index]->body->declarations.size(); ++decl) {
			if (prog.value().get()->functions[index]->body->declarations[decl]->type == "int") {
				file << prog.value().get()->functions[index]->body->declarations[decl]->identifier << ": .quad " << prog.value().get()->functions[index]->body->declarations[decl]->value.value().get()->value << '\n';
			}
		}
	}

	file << "\n.section .text\n.globl _" << prog.value().get()->functions[0]->name << "\n_" << prog.value().get()->functions[0]->name << ":\n";

	for (size_t index{}; index < prog.value().get()->functions.size(); ++index) {
		file << "    movq " << prog.value().get()->functions[index]->body->ret->return_value->value << "(%rip), %rax\n";
	}

	file
		<< "    movq %rax, %rcx      # Use the value in RAX as the exit code\n"
		<< "    call ExitProcess     # Call ExitProcess";
}

void print_tokens(std::vector<token> tokens) {
	for (auto& tok : tokens) {
		std::printf("{ '%s', '%u', Ln: '%u', Ch: '%u' }\n", tok.value.c_str(), tok.type, tok.line_num, tok.column_num);
	}
}