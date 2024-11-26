#include "ir.hpp"
#include <iostream>

Instruction::Instruction(Type type, const std::string& dest,
    const std::string& left, const std::string& right)
    : type(type), dest(dest), lop(left), rop(right) {
}

std::string Instruction::to_string() const {
    switch (type) {
    case ADD:
        return dest + " = add " + lop + ", " + rop;
    case SUB:
        return dest + " = sub " + lop + ", " + rop;
    case RETURN:
        return "return " + dest;
    default:
        return "unk?";
    }
}

Function::Function(const std::string& name) : name(name) {}

void Function::add_instruction(const Instruction& inst) {
    instructions.push_back(inst);
}

std::string Function::to_string() const {
    std::stringstream ss;
    ss << "define " << name << " {\n";
    for (const auto& inst : instructions) {
        ss << "    " << inst.to_string() << "\n";
    }
    ss << "}\n";
    return ss.str();
}

Module::Module(const std::string& name) : name(name) {}

Function* Module::create_function(const std::string& func_name) {
    functions.emplace_back(func_name);
    return &functions.back();
}

std::string Module::to_string() const {
    std::stringstream ss;
    ss << "module " << name << "\n";
    for (const auto& func : functions) {
        ss << func.to_string();
    }
    return ss.str();
}

bool Module::serialize(const std::string& path) const {
    try {
        std::ofstream file(path);
        if (!file.is_open()) return false;

        file << to_string();
        file.close();
        return true;
    }
    catch (std::exception&) { return false; }
}

std::unique_ptr<Module> Module::loadf(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) return nullptr;

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        auto mod = std::make_unique<Module>("");
        *mod = IRInterpreter::parse_module(buffer.str());
        return mod;
    }
    catch (const std::exception&) { return nullptr; }
}

Module& Module::operator=(const IRInterpreter::Module& other) {
    name = other.name;
    functions.clear();

    for (const auto& ir_func : other.functions) {
        Function func(ir_func.name);

        for (const auto& ir_inst : ir_func.instructions) {
            if (ir_inst.op == "add") {
                func.add_instruction(Instruction(
                    Instruction::ADD,
                    ir_inst.dest,
                    ir_inst.args[0],
                    ir_inst.args[1]
                ));
            }
            else if (ir_inst.op == "return") {
                func.add_instruction(Instruction(
                    Instruction::RETURN,
                    ir_inst.args[0],
                    "",
                    ""
                ));
            }
        }
        functions.push_back(func);
    }
    return *this;
}

void IRInterpreter::Context::set_value(const std::string& id, int value) {
    variables[id] = value;
}

int IRInterpreter::Context::get_value(const std::string& id) const {
    auto it = variables.find(id);
    if (it == variables.end()) {
        throw std::runtime_error("undefined variable " + id);
    }
    return it->second;
}

IRInterpreter::Module IRInterpreter::parse_module(const std::string& mod_ir) {
    Module mod;
    std::istringstream iss(mod_ir);
    std::string line;

    std::getline(iss, line);
    if (line.substr(0, 7) != "module ") {
        throw std::runtime_error("invalid module declaration @" + line);
    }

    mod.name = line.substr(7);

    Function cur_func;
    bool in_func = false;

    while (std::getline(iss, line)) {
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (line.empty()) continue;

        if (line.substr(0, 6) == "define") {
            if (in_func) {
                mod.functions.push_back(cur_func);
            }
            cur_func = Function{ line.substr(7, line.length() - 9)};
            in_func = true;
        }
        else if (line == "}") {
            mod.functions.push_back(cur_func);
            in_func = false;
        }
        else if (in_func) {
            Instruction inst = parse_inst(line);
            cur_func.instructions.push_back(inst);
        }
    }
    return mod;
}

IRInterpreter::Instruction IRInterpreter::parse_inst(const std::string& inst_ir) {
    Instruction inst;
    std::istringstream iss(inst_ir);
    std::string token;

    std::string first_word;
    iss >> first_word;

    if (first_word == "return") {
        inst.op = "return";
        std::string return_value;
        iss >> return_value;
        inst.args.push_back(return_value);
        return inst;
    }

    iss.clear();
    iss.seekg(0);

    std::getline(iss, inst.dest, '=');
    inst.dest.erase(0, inst.dest.find_first_not_of(" \t"));
    inst.dest.erase(inst.dest.find_last_not_of(" \t") + 1);

    iss >> inst.op;
    std::string arg_str;
    std::getline(iss, arg_str);

    std::istringstream arg_iss(arg_str);
    std::string arg;
    while (arg_iss >> arg) {
        if (!arg.empty() && arg.back() == ',') {
            arg.pop_back();
        }

        inst.args.push_back(arg);
    }
    return inst;
}

int IRInterpreter::interpret_func(const Function& func, Context& ctx) {
    for (const auto& inst : func.instructions) {
        if (inst.op == "add") {
            int lhs = ctx.get_value(inst.args[0]);
            int rhs = ctx.get_value(inst.args[1]);
            ctx.set_value(inst.dest, lhs + rhs);
        } else if (inst.op == "return") {
            return ctx.get_value(inst.args[0]);
        }
    }
    throw std::runtime_error("function did not return a value.");
}

int IRInterpreter::run(const Module& mod, const std::string& func_name,
    const std::unordered_map<std::string, int>& i_vars)
{
    std::cout << "Searching for function: " << func_name << std::endl;
    std::cout << "Available functions:" << std::endl;
    for (const auto& func : mod.functions) {
        std::cout << "- " << func.name << std::endl;
    }

    for (const auto& func : mod.functions) {
        std::cout << "Comparing: '" << func.name << "' == '" << func_name << "'" << std::endl;
        if (func.name == func_name) {
            Context ctx;
            for (const auto& [vr, v] : i_vars) {
                ctx.set_value(vr, v);
            }
            return interpret_func(func, ctx);
        }
    }
    throw std::runtime_error("function not found " + func_name);
}