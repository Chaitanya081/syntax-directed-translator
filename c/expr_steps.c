#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef enum { T_END, T_NUMBER, T_PLUS, T_MINUS, T_MULT, T_DIV, T_POW, T_LPAREN, T_RPAREN } TokenType;

typedef struct {
    TokenType type;
    double value;
} Token;

const char *src;
int pos = 0;
Token curtok;
int error_flag = 0;

// Skip spaces
void skip_space() {
    while (src[pos] == ' ') pos++;
}

// Read next token
void next_token() {
    skip_space();
    char c = src[pos];
    if (c == '\0') { curtok.type = T_END; return; }
    if ((c >= '0' && c <= '9') || c == '.') {
        curtok.type = T_NUMBER;
        curtok.value = strtod(&src[pos], NULL);
        while ((src[pos] >= '0' && src[pos] <= '9') || src[pos] == '.') pos++;
        return;
    }
    pos++;
    switch (c) {
        case '+': curtok.type = T_PLUS; break;
        case '-': curtok.type = T_MINUS; break;
        case '*': curtok.type = T_MULT; break;
        case '/': curtok.type = T_DIV; break;
        case '^': curtok.type = T_POW; break;
        case '(': curtok.type = T_LPAREN; break;
        case ')': curtok.type = T_RPAREN; break;
        default: error_flag = 1; curtok.type = T_END; break;
    }
}

/* Forward declarations */
double parse_expression();

// Primary: numbers or parentheses
double parse_primary() {
    if (curtok.type == T_NUMBER) {
        double val = curtok.value;
        printf("Step: Number = %.10g\n", val);
        next_token();
        return val;
    }
    if (curtok.type == T_LPAREN) {
        next_token();
        double val = parse_expression();
        if (curtok.type != T_RPAREN) {
            error_flag = 1;
            return 0;
        }
        next_token();
        return val;
    }
    error_flag = 1;
    return 0;
}

// Unary +/-
double parse_unary() {
    if (curtok.type == T_PLUS) { next_token(); return parse_unary(); }
    if (curtok.type == T_MINUS) {
        next_token();
        double val = -parse_unary();
        printf("Step: Unary minus applied = %.10g\n", val);
        return val;
    }
    return parse_primary();
}

// Exponentiation
double parse_pow() {
    double left = parse_unary();
    while (curtok.type == T_POW) {
        next_token();
        double right = parse_unary();
        double result = pow(left, right);
        printf("Step: %.10g ^ %.10g = %.10g\n", left, right, result);
        left = result;
    }
    return left;
}

// Multiplication / Division
double parse_term() {
    double left = parse_pow();
    while (curtok.type == T_MULT || curtok.type == T_DIV) {
        TokenType op = curtok.type;
        next_token();
        double right = parse_pow();
        double result = (op == T_MULT) ? (left * right) : (left / right);
        printf("Step: %.10g %c %.10g = %.10g\n", left, (op==T_MULT?'*':'/'), right, result);
        left = result;
    }
    return left;
}

// Addition / Subtraction
double parse_expression() {
    double left = parse_term();
    while (curtok.type == T_PLUS || curtok.type == T_MINUS) {
        TokenType op = curtok.type;
        next_token();
        double right = parse_term();
        double result = (op == T_PLUS) ? (left + right) : (left - right);
        printf("Step: %.10g %c %.10g = %.10g\n", left, (op==T_PLUS?'+':'-'), right, result);
        left = result;
    }
    return left;
}

// Main loop
int main() {
    char expression[256];
    char choice;

    printf("=== Syntax-Directed Arithmetic Evaluator ===\n\n");

    do {
        printf("Enter arithmetic expression (or 'q' to quit):\n> ");
        fgets(expression, sizeof(expression), stdin);
        expression[strcspn(expression, "\n")] = 0; // remove newline

        if (strlen(expression) == 0) {
            printf("No expression entered. Please try again.\n\n");
            continue;
        }

        if (strcmp(expression, "q") == 0 || strcmp(expression, "Q") == 0) break;

        printf("\nQuestion: %s\n\n", expression);

        src = expression;
        pos = 0;
        error_flag = 0;
        next_token();
        double result = parse_expression();

        if (error_flag || curtok.type != T_END) {
            printf("Syntax error in expression.\n\n");
        } else {
            printf("\nFinal Result: %.10g\n\n", result);
        }

        printf("Do you want to enter another expression? (y/n): ");
        choice = getchar();
        while(getchar() != '\n'); // clear input buffer
        printf("\n");

    } while (choice == 'y' || choice == 'Y');

    printf("Thank you for using the evaluator!\n");
    return 0;
}
