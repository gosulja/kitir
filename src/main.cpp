#include "ir.hpp"
#include <iostream>

int main() {
	Module mod("example");

	Function* main_func = mod.create_function("add");

	main_func->add_instruction({
		Instruction::ADD,
		"%result",
		"%x",
		"%y"
	});

	main_func->add_instruction({
		Instruction::RETURN,
		"%result",
		"?",
		"?",
	});

	std::string filename = "example.ir";
	if (mod.serialize(filename)) std::cout << "Successfully serialized the module as /" + filename + "\n";

	auto loadmd = Module::loadf(filename);
	if (loadmd) {
		std::cout << "Successfully loaded module from /" + filename + "\n";
		std::cout << loadmd->to_string() << "\n";
	}

	try {
		IRInterpreter::Module ir_mod = IRInterpreter::parse_module(loadmd->to_string());

		for (const auto& f : ir_mod.functions) {
			std::cout << f.name << " defined\n";
		}

		int result = IRInterpreter::run(ir_mod, "add", {
			{"%x", 5},
			{"%y", 3}
		});

		std::cout << "result = " << result << "\n";
	} catch (const std::exception& exc) {
		std::cerr << "error = " << exc.what() << "\n";
		return 1;
	}

	return 0;
}