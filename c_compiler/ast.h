#ifndef AST_H
#define AST_H

#include <optional>
#include <vector>
#include <string>
#include <memory>

// Base class for all AST nodes
class ast_node {
public:
	virtual ~ast_node() = default;
};

// Represents a constant (like "2")
class expression : public ast_node {
public:
	std::string
		type{},
		value{};

	expression(std::string type = "", std::string value = "")
		: type(type), value(value) {}
};

// Represents a return statement
class return_statement : public ast_node {
public:
	std::unique_ptr<expression> return_value{};

	return_statement(std::unique_ptr<expression> return_value = nullptr)
		: return_value(std::move(return_value)) {}
};

class declaration_statement : public ast_node {
public:
	std::string
		type{},
		identifier{};
	std::optional<std::string> assignment_operation{};
	std::optional<std::unique_ptr<expression>> value{};

	declaration_statement(std::string type = "", std::string identifier = "")
		: type(type), identifier(identifier) {}
};

// Represents a block of code
class compound_statement : public ast_node {
public:
	std::vector<std::unique_ptr<declaration_statement>> declarations{};
	std::unique_ptr<return_statement> ret{};

	compound_statement(std::unique_ptr<return_statement> ret = nullptr)
		: ret(std::move(ret)) {}

	void add_declaration(std::unique_ptr<declaration_statement> declaration) {
		declarations.push_back(std::move(declaration));
	}
};

// Represents a function definition
class function_definition : public ast_node {
public:
	std::string return_type{};
	std::string name{};
	std::unique_ptr<compound_statement> body{};

	function_definition(std::string return_type = "", std::string name = "", std::unique_ptr<compound_statement> body = nullptr)
		: return_type(return_type), name(name), body(std::move(body)) {}
};

// Represents a program, which consists of a sequence of function definitions
class program : public ast_node {
public:
	std::vector<std::unique_ptr<declaration_statement>> global_declarations{};
	std::vector<std::unique_ptr<function_definition>> functions{};

	void add_declaration(std::unique_ptr<declaration_statement> global_declaration) {
		global_declarations.push_back(std::move(global_declaration));
	}

	void add_function(std::unique_ptr<function_definition> function) {
		functions.push_back(std::move(function));
	}
};

#endif // !AST_H