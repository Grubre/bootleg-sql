#include <iostream>

int yyparse();

void yyerror(const char* s) {
    fprintf(stderr, "Parse error: %s\n", s);
}

int main() {
    std::cout << "Enter an expression: ";
    yyparse();
    return 0;
}

