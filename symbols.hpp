#ifndef SYMBOLS_HPP
#define SYMBOLS_HPP

#include "output.hpp"
#include "nodes.hpp"
#include <unordered_map>
#include <stack>
#include <vector>
#include <memory>
#include <string>


using namespace std;

enum class type_t {
    INT,
    FLOAT,
    STRING,
    VOID
};

class Symbol {
public:
    string name;
    type_t type; // TODO: Define or include type_t
    int offset;

    Symbol(string name, type_t type, int offset);
    Symbol();
};

class Scope {
public:
    vector<Symbol> symbols;
    int current_negative_offset;
    int current_positive_offset;
    int initial_positive_offset;
    int initial_negative_offset;

    Scope(int initialPositiveOffset, int initialNegativeOffset);
    int addArg(const string& name, type_t type);
    int addVariable(const string& name, type_t type);
};

class FunctionSymbolTable {
public:
    class FunctionEntry {
    public:
        string name;
        type_t returnType;
        shared_ptr<ast::Formals> formals; // Replace paramTypes with Formals

        FunctionEntry(string name, type_t returnType, shared_ptr<ast::Formals> formals);
    };

private:
    unordered_map<string, FunctionEntry> functionMap;

public:
    bool insertFunction(const string& name, type_t returnType, shared_ptr<ast::Formals> formals);
    const FunctionEntry* lookupFunction(const string& name) const;
};

class SymbolTable {
public:
    stack<Scope> symbols_stack;
    int current_positive_offset;
    int current_negative_offset;

    SymbolTable();
    void beginScope();
    void endScope();
    int addArg(const string& name, type_t type);
    int addVariable(const string& name, type_t type);
    Symbol* lookup(const string& name);
    bool validateFunctionCall(const string& name, shared_ptr<ast::FormalsList> actualParams);
};

#endif // SYMBOLS_HPP
