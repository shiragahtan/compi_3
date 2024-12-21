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
        shared_ptr<ast::Formals> formals; // Replace paramTypes with Formals

        FunctionEntry(string name, type_t returnType, shared_ptr<ast::Formals> formals)
            : name(name), returnType(returnType), formals(formals) {}
    };

private:
    unordered_map<string, FunctionEntry> functionMap;

public:
    bool insertFunction(const string& name, type_t returnType, shared_ptr<ast::Formals> formals) {
        if (functionMap.find(name) != functionMap.end()) {
            return false; // Function already exists
        }
        functionMap[name] = FunctionEntry(name, returnType, formals);
        return true;
    }

    const FunctionEntry* lookupFunction(const string& name) const {
        auto it = functionMap.find(name);
        if (it != functionMap.end()) {
            return &it->second;
        }
        return nullptr;
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
        return -1; // Indicate failure - TODO: that's not true! it can be negative
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
    
    bool validateFunctionCall(const string& name, shared_ptr<ast::FormalsList> actualParams) {
        const auto* function = functionTable.lookupFunction(name);
        if (!function) {
            errorUndefFunc(0, name); // Adjust `lineno` appropriately
            return false;
        }
        
        const auto& expectedParams = function->formals;
        if (expectedParams->size() != actualParams->size()) {
            errorPrototypeMismatch(0, name, actualParams); // Adjust args
            return false;
        }

        auto expectedIt = expectedParams->begin();
        auto actualIt = actualParams->begin();

        while (expectedIt != expectedParams->end() && actualIt != actualParams->end()) {
            if ((*expectedIt)->type != (*actualIt)->type) {
                errorMismatch(0); // Adjust `lineno` appropriately
                return false;
            }
            ++expectedIt;
            ++actualIt;
        }
        return true;
    }
};
