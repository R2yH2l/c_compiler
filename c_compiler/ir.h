#ifndef IR_H
#define IR_H

#include "tokenization.h"

#include <unordered_map>

std::unordered_map<std::string, std::vector<std::string>> ir_map{
	{"keyword_return", { "mov , %eax", "ret" }}
};

#endif // !IR_H