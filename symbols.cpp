#include "output.hpp"
#include <unordered_map>
#include <stack>
#include <vector>

using namespace output;
using namespace std;

class Symbol {
public:
    string name;
    type_t type; //TODO: type_t?
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
        vector<type_t> paramTypes;   // Parameter types for validation

        FunctionEntry(string name, type_t returnType, vector<type_t> paramTypes)
            : name(name), returnType(returnType), paramTypes(paramTypes) {}
    };

private:
    unordered_map<string, FunctionEntry> functionMap; // Map of function name to FunctionEntry

public: //shira
    // Insert a new function into the table
    bool insertFunction(const string& name, type_t returnType, const vector<type_t>& paramTypes) {
        if (functionMap.find(name) != functionMap.end()) {
            // Comment: Function name must be unique. If the name already exists, return false.
            return false; 
        }
        functionMap[name] = FunctionEntry(name, returnType, paramTypes);
        return true;
    }

    //shira
    // Lookup a function by name
    const FunctionEntry* lookupFunction(const string& name) const {
        auto it = functionMap.find(name);
        if (it != functionMap.end()) {
            return &it->second;
        }
        return nullptr; // Function not found
    }
};


class SymbolTable {
public:
    stack<Scope> symbols_stack;                  // Stack of scopes (with symbols & offset)
    stack<int> offsets_stack;                   // Stack of offsets
    int currentOffset = 0;

    // Begin a new scope
    void beginScope() { //TODO: shira - need to create scope also for the while and for conditions (2 scopes in total)
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
