#include "output.hpp"
#include <unordered_map>
#include <stack>
#include <vector>

using namespace output;
using namespace std;



class Symbol {
public:
    string name;
    type_t type;
    int offset;

    Symbol(string name, type_t type, int offset) 
    : name(name), type(type), offset(offset) {}
    
    Symbol() = default;
};

class ParamScope {
public:
    vector<Symbol> symbols;
    int offset;

    ParamScope() : offset(0) {}

    void addParam(const string& name, type_t type) {
        symbols.push_back(Symbol(name, type, offset--));
    }
};

class FunctionScope {
public:
    vector<Symbol> symbols;
    int offset;

    FunctionScope() : offset(0) {}

    void addVariable(const string& name, type_t type) {
        symbols.push_back(Symbol(name, type, offset++));
    }
};


class FunctionSymbolTable {
public:
    class FunctionEntry {
    public:
        string name;              
        type_t returnType;          
        vector<type_t> paramTypes; 
        FunctionEntry(string name, type_t returnType, vector<type_t> paramTypes)
            : name(name), returnType(returnType), paramTypes(paramTypes) {}

        bool matches(const vector<type_t>& params) const {
            return paramTypes == params;
        }
    };

private:
    unordered_map<string, vector<FunctionEntry>> functionMap;
public:
    // Insert a new function into the table
    bool insertFunction(const string& name, type_t returnType, const vector<type_t>& paramTypes) {
        auto& functionList = functionMap[name];
        for (const auto& func : functionList) {
            if (func.matches(paramTypes)) {
                return false; // Function with the same signature already exists
            }
        }
        functionList.emplace_back(name, returnType, paramTypes);
        return true;
    }

    // Lookup a function by name and parameter types
    const FunctionEntry* lookupFunction(const string& name, const vector<type_t>& paramTypes) const {
        auto it = functionMap.find(name);
        if (it != functionMap.end()) {
            for (const auto& func : it->second) {
                if (func.matches(paramTypes)) {
                    return &func;
                }
            }
        }
        return nullptr; // Function not found
    }

    // Get all overloads of a function
    const vector<FunctionEntry>* getAllFunctions(const string& name) const {
        auto it = functionMap.find(name);
        if (it != functionMap.end()) {
            return &it->second;
        }
        return nullptr; // No functions with this name
    }
};

class SymbolTable {
public:
    stack<Scope> symbols_stack;                  // Stack of scopes (with symbols & offset)
    stack<int> offsets_stack;                   // Stack of offsets
    int currentOffset = 0;

    // Begin a new scope
    void beginScope() { //TODO: shira - call to void ScopePrinter::beginScope() (and etc)
        scopes.push({});
        offsets_stack.push(currentOffset);
        symbols_stack.push(Scope(currentOffset)); // Push scope with current offset
    }

    // End the current scope
    void endScope() {
        if (!scopes.empty()) scopes.pop();
        if (!offsets_stack.empty()) {
            currentOffset = offsets_stack.top();
            offsets_stack.pop();
        }
        if (!symbols_stack.empty()) symbols_stack.pop();
    }

    // Insert a symbol into the current scope
    bool insertSymbol(const string& name, type_t type) {
        if (scopes.top().count(name)) return false; // Check for shadowing
        Symbol newSymbol(name, type, currentOffset++);
        scopes.top()[name] = newSymbol;
        if (!symbols_stack.empty()) {
            symbols_stack.top().symbols.push_back(newSymbol); // Add symbol to current scope
        }
        return true;
    }

    // Lookup a symbol in all active scopes
    Symbol* lookup(const string& name) {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            if (it->count(name)) return &(*it)[name];
        }
        return nullptr;
    }

    // Insert a new empty scope
    void insert_new_scope() {
        scopes.push({});
        offsets_stack.push(currentOffset);
        symbols_stack.push(Scope(currentOffset));
    }

    // End current scope
    void end_my_scope() {
        if (!scopes.empty()) scopes.pop();
        if (!offsets_stack.empty()) {
            currentOffset = offsets_stack.top();
            offsets_stack.pop();
        }
        if (!symbols_stack.empty()) symbols_stack.pop();
    }
};
