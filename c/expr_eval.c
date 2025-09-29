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

void skip_space() {
    while (src[pos] == ' ') pos++;
}

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

double parse_primary() {
    if (curtok.type == T_NUMBER) { double val = curtok.value; next_token(); return val; }
    if (curtok.type == T_LPAREN) { next_token(); double val = parse_expression(); if (curtok.type != T_RPAREN) { error_flag = 1; return 0; } next_token(); return val; }
    error_flag = 1;
    return 0;
}

double parse_unary() {
    if (curtok.type == T_PLUS) { next_token(); return parse_unary(); }
    if (curtok.type == T_MINUS) { next_token(); return -parse_unary(); }
    return parse_primary();
}

double parse_pow() {
    double left = parse_unary();
    while (curtok.type == T_POW) {
        next_token();
        double right = parse_unary();
        left = pow(left, right);
    }
    return left;
}

double parse_term() {
    double left = parse_pow();
    while (curtok.type == T_MULT || curtok.type == T_DIV) {
        TokenType op = curtok.type;
        next_token();
        double right = parse_pow();
        if (op == T_MULT) left *= right;
        else left /= right;
    }
    return left;
}

double parse_expression() {
    double left = parse_term();
    while (curtok.type == T_PLUS || curtok.type == T_MINUS) {
        TokenType op = curtok.type;
        next_token();
        double right = parse_term();
        if (op == T_PLUS) left += right;
        else left -= right;
    }
    return left;
}

int main() {
    char expression[256];
    printf("Enter arithmetic expression: ");
    fgets(expression, sizeof(expression), stdin);
    expression[strcspn(expression, "\n")] = 0; // remove newline

    src = expression;
    pos = 0;
    error_flag = 0;
    next_token();
    double result = parse_expression();

    if (error_flag || curtok.type != T_END) {
        printf("Syntax error in expression.\n");
        return 1;
    }

    printf("Result: %.10g\n", result);
    return 0;
}
