//
// Created by Omer Oz on 19/12/2024.
//

#include "visitor.hpp"

#include "output.hpp"
#include <iostream>

namespace output {
    static const std::string child_prefix = "├──";
    static const std::string last_child_prefix = "└──";
    static const std::string child_indent = "│   ";
    static const std::string last_child_indent = "    ";

    /* Helper functions */

    static std::string toString(ast::BuiltInType type) {
        switch (type) {
            case ast::BuiltInType::INT:
                return "int";
            case ast::BuiltInType::BOOL:
                return "bool";
            case ast::BuiltInType::BYTE:
                return "byte";
            case ast::BuiltInType::VOID:
                return "void";
            case ast::BuiltInType::STRING:
                return "string";
            default:
                return "unknown";
        }
    }
    /* SemanticVisitor implementation */

    SemanticVisitor::SemanticVisitor() : indents({last_child_indent}), prefixes({last_child_prefix}) {}

    void SemanticVisitor::print_indented(const std::string &str) {
        for (auto it = indents.begin(); it != indents.end() - 1; ++it) {
            std::cout << *it;
        }
        std::cout << prefixes.back() << str << std::endl;
    }

    void SemanticVisitor::enter_child() {
        indents.emplace_back(child_indent);
        prefixes.emplace_back(child_prefix);
    }

    void SemanticVisitor::enter_last_child() {
        indents.emplace_back(last_child_indent);
        prefixes.emplace_back(last_child_prefix);
    }

    void SemanticVisitor::leave_child() {
        indents.pop_back();
        prefixes.pop_back();
    }

    void SemanticVisitor::visit(ast::Num &node) {
        print_indented("Num: " + std::to_string(node.value));
    }

    void SemanticVisitor::visit(ast::NumB &node) {
        print_indented("NumB: " + std::to_string(node.value));
    }

    void SemanticVisitor::visit(ast::String &node) {
        print_indented("String: " + node.value);
    }

    void SemanticVisitor::visit(ast::Bool &node) {
        print_indented("Bool: " + std::string((node.value ? "true" : "false")));
    }

    void SemanticVisitor::visit(ast::ID &node) {
        print_indented("ID: " + node.value);
    }

    void SemanticVisitor::visit(ast::BinOp &node) {
        std::string op;

        switch (node.op) {
            case ast::BinOpType::ADD:
                op = "+";
                break;
            case ast::BinOpType::SUB:
                op = "-";
                break;
            case ast::BinOpType::MUL:
                op = "*";
                break;
            case ast::BinOpType::DIV:
                op = "/";
                break;
        }

        print_indented("BinOp: " + op);

        enter_child();
        node.left->accept(*this);
        leave_child();

        enter_last_child();
        node.right->accept(*this);
        leave_child();
    }

    void SemanticVisitor::visit(ast::RelOp &node) {
        std::string op;

        switch (node.op) {
            case ast::RelOpType::EQ:
                op = "==";
                break;
            case ast::RelOpType::NE:
                op = "!=";
                break;
            case ast::RelOpType::LT:
                op = "<";
                break;
            case ast::RelOpType::LE:
                op = "<=";
                break;
            case ast::RelOpType::GT:
                op = ">";
                break;
            case ast::RelOpType::GE:
                op = ">=";
                break;
        }

        print_indented("RelOp: " + op);

        enter_child();
        node.left->accept(*this);
        leave_child();


        enter_last_child();
        node.right->accept(*this);
        leave_child();
    }

    void SemanticVisitor::visit(ast::Type &node) {
        print_indented("Type: " + toString(node.type));
    }

    void SemanticVisitor::visit(ast::Cast &node) {
        print_indented("Cast");

        enter_child();
        node.exp->accept(*this);
        leave_child();

        enter_last_child();
        node.target_type->accept(*this);
        leave_child();
    }

    void SemanticVisitor::visit(ast::Not &node) {
        print_indented("Not");

        enter_last_child();
        node.exp->accept(*this);
        leave_child();
    }

    void SemanticVisitor::visit(ast::And &node) {
        print_indented("And");

        enter_child();
        node.left->accept(*this);
        leave_child();

        enter_last_child();
        node.right->accept(*this);
        leave_child();
    }

    void SemanticVisitor::visit(ast::Or &node) {
        print_indented("Or");

        enter_child();
        node.left->accept(*this);
        leave_child();

        enter_last_child();
        node.right->accept(*this);
        leave_child();
    }

    void SemanticVisitor::visit(ast::ExpList &node) {
        print_indented("ExpList");

        for (auto it = node.exps.rbegin(); it != node.exps.rend(); ++it) {
            if (it != node.exps.rend() - 1) {
                enter_child();
            } else {
                enter_last_child();
            }
            (*it)->accept(*this);
            leave_child();
        }
    }

    void SemanticVisitor::visit(ast::Call &node) {
        //print_indented("Call");
        printer.beginScope();
        symTab.beginScope();

        enter_child();
        node.func_id->accept(*this);
        leave_child();

        enter_last_child();
        node.args->accept(*this);
        leave_child();
        printer.endScope();
        symTab.endScope();
    }

    void SemanticVisitor::visit(ast::Statements &node) {
        //print_indented("Statements");
        printer.beginScope();
        symTab.beginScope();

        for (auto it = node.statements.begin(); it != node.statements.end(); ++it) {
            if (it != node.statements.end() - 1) {
                enter_child();
            } else {
                enter_last_child();
            }
            (*it)->accept(*this);
            leave_child();
        }
        printer.endScope();
        symTab.endScope();
    }

    void SemanticVisitor::visit(ast::Break &node) {
        print_indented("Break");
    }

    void SemanticVisitor::visit(ast::Continue &node) {
        print_indented("Continue");
    }

    void SemanticVisitor::visit(ast::Return &node) {
        print_indented("Return");

        if (node.exp) {
            enter_last_child();
            node.exp->accept(*this);
            leave_child();
        }
    }

    void SemanticVisitor::visit(ast::If &node) {
        //print_indented("If");
        printer.beginScope();
        symTab.beginScope();

        enter_child();
        node.condition->accept(*this);
        leave_child();

        if (node.otherwise) {
            enter_child();
        } else {
            enter_last_child();
        }
        node.then->accept(*this);
        leave_child();

        if (node.otherwise) {
            enter_last_child();
            node.otherwise->accept(*this);
            leave_child();
        }
        printer.endScope();
        symTab.endScope();
    }

    void SemanticVisitor::visit(ast::While &node) {
        //print_indented("While");
        printer.beginScope();
        symTab.beginScope();
        enter_child();
        node.condition->accept(*this);
        leave_child();

        enter_last_child();
        node.body->accept(*this);
        leave_child();
        printer.endScope();
        symTab.endScope();
    }

    void SemanticVisitor::visit(ast::VarDecl &node) {
        print_indented("VarDecl");

        enter_child();
        node.id->accept(*this);
        leave_child();

        if (node.init_exp) {
            enter_child();
        } else {
            enter_last_child();
        }
        node.type->accept(*this);
        leave_child();

        if (node.init_exp) {
            enter_last_child();
            node.init_exp->accept(*this);
            leave_child();
        }
    }

    void SemanticVisitor::visit(ast::Assign &node) {
        print_indented("Assign");

        enter_child();
        node.id->accept(*this);
        leave_child();
        if (symTab.lookup(node.id)== nullptr){
            //error
        }
    }

        enter_last_child();
        node.exp->accept(*this);
        leave_child();
    }

    void SemanticVisitor::visit(ast::Formal &node) {
        print_indented("Formal");

        enter_child();
        node.id->accept(*this);
        leave_child();

        enter_last_child();
        node.type->accept(*this);
        leave_child();

        //symTab.symbols_stack.top().symbols->push_back(node.id,node.type);
    }

    void SemanticVisitor::visit(ast::Formals &node) {
        print_indented("Formals");

        for (auto it = node.formals.rbegin(); it != node.formals.rend(); ++it) {
            if (it != node.formals.rend() - 1) {
                enter_child();
            } else {
                enter_last_child();
            }
            (*it)->accept(*this);
            leave_child();
        }
    }

    void SemanticVisitor::visit(ast::FuncDecl &node) {
        print_indented("FuncDecl");

        enter_child();
        node.id->accept(*this);
        leave_child();

        enter_child();
        node.return_type->accept(*this);
        leave_child();

        enter_child();
        node.formals->accept(*this);
        leave_child();

        enter_last_child();
        node.body->accept(*this);
        leave_child();
    }

    void SemanticVisitor::visit(ast::Funcs &node) {
        print_indented("Funcs");

        for (auto it = node.funcs.begin(); it != node.funcs.end(); ++it) {
            if (it != node.funcs.end() - 1) {
                enter_child();
            } else {
                enter_last_child();
            }
            (*it)->accept(*this);
            leave_child();
        }
    }



