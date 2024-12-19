#include <iostream>
#include "output.hpp"
#include "nodes.hpp"

// Extern from the bison-generated parser
extern int yyparse();

extern std::shared_ptr<ast::Node> program;

int main() {
    // Parse the input. The result is stored in the global variable `program`
    yyparse();

    // TODO: shira - needs to use the class the we defined. we need to create an object from the type that we defined

    // Print the AST using the PrintVisitor
    output::PrintVisitor printVisitor;
    program->accept(printVisitor);
}
