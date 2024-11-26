# kitlr

A simple IR interpreter for a WIP language; Kit.

# preview
This defines a module which holds a function labelled `add`, it returns the result of `x` and `y` added.

```
module example
define add {
    %result = add %x, %y
    return %result
}
```

Output (debug)

```
Successfully serialized the module as /example.ir
Successfully loaded module from /example.ir
module example
define add {
    %result = add %x, %y
    return %result
}

add defined
Searching for function: add
Available functions:
- add
Comparing: 'add' == 'add'
result = 8
```

# example

```cpp
Module mod("example");

Function* add = mod.create_function("add");

add->add_instruction({
	Instruction::ADD,
	"%result",
	"%x",
	"%y"
});

add->add_instruction({
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
```