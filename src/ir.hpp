#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <stdexcept>

class IRInterpreter {
public:
    struct Instruction {
        std::string op;
        std::string dest;
        std::vector<std::string> args;
    };

    struct Function {
        std::string name;
        std::vector<Instruction> instructions;
    };

    struct Module {
        std::string name;
        std::vector<Function> functions;
    };

    class Context {
    public:
        void set_value(const std::string& id, int value);
        int get_value(const std::string& id) const;
    private:
        std::unordered_map<std::string, int> variables;
    };

    static Module parse_module(const std::string& mod_ir);
    static Instruction parse_inst(const std::string& inst_ir);
    static int interpret_func(const Function& func, Context& ctx);
    static int run(const Module& mod, const std::string& func_name,
        const std::unordered_map<std::string, int>& i_vars);
};

class Instruction {
public:
    enum Type { ADD, SUB, RETURN };

    Instruction(Type type, const std::string& dest,
        const std::string& left, const std::string& right = "");

    Type type;
    std::string dest;
    std::string lop;
    std::string rop;

    std::string to_string() const;
};

class Function {
public:
    explicit Function(const std::string& name);
    void add_instruction(const Instruction& inst);
    std::string to_string() const;

    std::string name;
    std::vector<Instruction> instructions;
};

class Module {
public:
    explicit Module(const std::string& name);

    Function* create_function(const std::string& func_name);
    std::string to_string() const;
    bool serialize(const std::string& path) const;
    static std::unique_ptr<Module> loadf(const std::string& path);
    Module& operator=(const IRInterpreter::Module& other);

    std::string name;
    std::vector<Function> functions;
};