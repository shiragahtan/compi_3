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

class Scope {
public:
    vector<Symbol> symbols;
    int current_negative_offset;
    int current_positive_offset;
    int initial_positive_offset;
    int initial_negative_offset;

    Scope(int initialPositiveOffset, int initialNegativeOffset)
        : current_negative_offset(initialNegativeOffset),
          current_positive_offset(initialPositiveOffset),
          initial_positive_offset(initialPositiveOffset),
          initial_negative_offset(initialNegativeOffset) {}

    int addArg(const string& name, type_t type) {
        int offset = current_negative_offset;
        symbols.push_back(Symbol(name, type, current_negative_offset--));
        return offset;
    }

    int addVariable(const string& name, type_t type) {
        int offset = current_positive_offset;
        symbols.push_back(Symbol(name, type, current_positive_offset++));
        return offset;
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

public:
    // Insert a new function into the table
    bool insertFunction(const string& name, type_t returnType, const vector<type_t>& paramTypes) { //TODO: shira that it will get a variable from formal type that will have the id's and types - we will have a vector of formals
        if (functionMap.find(name) != functionMap.end()) {
            // Comment: Function name must be unique. If the name already exists, return false.
            return false; 
        }
        functionMap[name] = FunctionEntry(name, returnType, paramTypes);
        return true;
    }

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
    stack<Scope> symbols_stack;
    int current_positive_offset = 0;
    int current_negative_offset = -1;

    void beginScope() {
        symbols_stack.push(Scope(current_positive_offset, current_negative_offset));
    }

    void endScope() {
        if (!symbols_stack.empty()) {
            current_positive_offset = symbols_stack.top().current_positive_offset;
            current_negative_offset = symbols_stack.top().current_negative_offset;
            symbols_stack.pop();
        }
    }

    int addArg(const string& name, type_t type) {
        if (!symbols_stack.empty()) {
            int offset = symbols_stack.top().addArg(name, type);
            current_negative_offset--;
            return offset;
        }
        return -1; // Indicate failure
    }

    int addVariable(const string& name, type_t type) {
        if (!symbols_stack.empty()) {
            int offset = symbols_stack.top().addVariable(name, type);
            current_positive_offset++;
            return offset;
        }
        return -1; // Indicate failure
    }

    Symbol* lookup(const string& name) {
        for (auto it = symbols_stack.rbegin(); it != symbols_stack.rend(); ++it) {
            for (const auto& symbol : it->symbols) {
                if (symbol.name == name) {
                    return &const_cast<Symbol&>(symbol);
                }
            }
        }
        return nullptr;
    }
};
