#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT_SIZE 1000

typedef enum {
    NAME, NUMBER, STRING,
    AND, OR, LT, GT, LE, GE, NE, EQ, CONCAT,
    PLUS, MINUS, TIMES, DIVIDE, POWER,
    NOT,
    LPAREN, RPAREN, LBRACKET, RBRACKET, ASSIGN,
    DO, END, WHILE, IF, THEN, ELSE, ELSEIF, FOR, VAR, LOCAL, FUNCTION, RETURN, BREAK, COMMA, SEMICOLON,
    END_OF_INPUT
} TokenType;

typedef struct {
    TokenType type;
    char *lexeme;
    int position;
    int line;
} Token;

Token currentToken;
char input[MAX_INPUT_SIZE];
int currentPosition = 0;
int currentLine = 1;

const char *tokenStrings[] = {
    "NAME", "NUMBER", "STRING",
    "AND", "OR", "LT", "GT", "LE", "GE", "NE", "EQ", "CONCAT",
    "PLUS", "MINUS", "TIMES", "DIVIDE", "POWER",
    "NOT",
    "LPAREN", "RPAREN", "LBRACKET", "RBRACKET", "ASSIGN",
    "DO", "END", "WHILE", "IF", "THEN", "ELSE", "ELSEIF", "FOR", "VAR", "LOCAL", "FUNCTION", "RETURN", "BREAK", "COMMA", "SEMICOLON",
    "END_OF_INPUT"
};

#include <ctype.h>

// Função ObterToken (obtem o próximo token do código fonte)
void advance() {
    char *tokenStr = strtok(input + currentPosition, " \t\n\r()+-*/^{}[],;=");
    if (tokenStr != NULL) {
        currentToken.lexeme = malloc(strlen(tokenStr) + 1);
        strncpy(currentToken.lexeme, tokenStr, strlen(tokenStr));
        currentToken.lexeme[strlen(tokenStr)] = '\0';
        currentToken.position = currentPosition;
        currentToken.line = currentLine;

        // Converta para minúsculas para facilitar a comparação
        for (int i = 0; tokenStr[i]; i++) {
            tokenStr[i] = tolower(tokenStr[i]);
        }

        // Verifique palavras-chave
        if (strcmp(tokenStr, "function") == 0) {
            currentToken.type = FUNCTION;
        } else {
            for (int i = AND; i <= SEMICOLON; i++) {
                if (strcmp(tokenStr, tokenStrings[i]) == 0) {
                    currentToken.type = (TokenType)i;
                    break;
                }
            }
            // Restante do código para atribuir outros tipos de token
        }

        currentPosition = tokenStr - input + strlen(tokenStr);
        while (*tokenStr) {
            if (*tokenStr == '\n') {
                currentLine++;
            }
            tokenStr++;
        }
    } else {
        currentToken.type = END_OF_INPUT;
    }
}


// Função para imprimir o tipo de token como uma string (apenas para depuração)
const char* tokenToString(TokenType type) {
    switch (type) {
        case NAME: return "NAME";
        case NUMBER: return "NUMBER";
        case STRING: return "STRING";
        case AND: return "AND";
        case OR: return "OR";
        case LT: return "LT";
        case GT: return "GT";
        case LE: return "LE";
        case GE: return "GE";
        case NE: return "NE";
        case EQ: return "EQ";
        case CONCAT: return "CONCAT";
        case PLUS: return "PLUS";
        case MINUS: return "MINUS";
        case TIMES: return "TIMES";
        case DIVIDE: return "DIVIDE";
        case POWER: return "POWER";
        case NOT: return "NOT";
        case LPAREN: return "LPAREN";
        case RPAREN: return "RPAREN";
        case LBRACKET: return "LBRACKET";
        case RBRACKET: return "RBRACKET";
        case ASSIGN: return "ASSIGN";
        case DO: return "DO";
        case END: return "END";
        case WHILE: return "WHILE";
        case IF: return "IF";
        case THEN: return "THEN";
        case ELSE: return "ELSE";
        case ELSEIF: return "ELSEIF";
        case FOR: return "FOR";
        case VAR: return "VAR";
        case LOCAL: return "LOCAL";
        case FUNCTION: return "FUNCTION";
        case RETURN: return "RETURN";
        case BREAK: return "BREAK";
        case COMMA: return "COMMA";
        case SEMICOLON: return "SEMICOLON";
        case END_OF_INPUT: return "END_OF_INPUT";
        default: return "UNKNOWN";
    }
}

// Função para emparelhar um token
void match(TokenType expectedToken, FILE *file) {
    if (currentToken.type == expectedToken) {
        advance(file);
    } else {
        fprintf(stderr, "Error: Expected '%s', but found '%s' at position %d\n",
                tokenToString(expectedToken), currentToken.lexeme, currentToken.position);
        exit(EXIT_FAILURE);
    }
}

// Funções para os procedimentos recursivos preditivos
void parse_Block(FILE *file);
void parse_Stmt(FILE *file);
void parse_ElseIfBlocks(FILE *file);
void parse_Exps(FILE *file);
void parse_Exp(FILE *file);
void parse_PrefixExp(FILE *file);
void parse_Field(FILE *file);
void parse_BinOp(FILE *file);
void parse_Vars(FILE *file);
void parse_Var(FILE *file);
void parse_Function(FILE *file);
void parse_FunctionBody(FILE *file);
void parse_Params(FILE *file);
void parse_Names(FILE *file);
void parse_Name(FILE *file);

// Função para entrar em modo de pânico
void panicMode(TokenType syncSet[], int size, FILE *file);

// Implementações dos procedimentos
void parse_Block(FILE *file) {
    while (currentToken.type != END_OF_INPUT && currentToken.type != END && currentToken.type != ELSE) {
        parse_Stmt(file);
        if (currentToken.type == SEMICOLON) {
            match(SEMICOLON, file);
        } else {
            break;
        }
    }
}

void parse_Stmt(FILE *file) {
    switch (currentToken.type) {
        case VAR:
            match(VAR, file);
            match(ASSIGN, file);
            parse_Exps(file);
            break;
        case FUNCTION:
            parse_Function(file);
            break;
        case DO:
            match(DO, file);
            parse_Block(file);
            match(END, file);
            break;
        case WHILE:
            match(WHILE, file);
            parse_Exp(file);
            match(DO, file);
            parse_Block(file);
            match(END, file);
            break;
        case IF:
            match(IF, file);
            parse_Exp(file);
            match(THEN, file);
            parse_Block(file);
            parse_ElseIfBlocks(file);
            if (currentToken.type == ELSE) {
                match(ELSE, file);
                parse_Block(file);
            }
            match(END, file);
            break;
        case RETURN:
            match(RETURN, file);
            if (currentToken.type != SEMICOLON) {
                parse_Exps(file);
            }
            break;
        case BREAK:
            match(BREAK, file);
            break;
        case FOR:
            match(FOR, file);
            parse_Name(file);
            match(ASSIGN, file);
            parse_Exp(file);
            match(COMMA, file);
            parse_Exp(file);
            if (currentToken.type == COMMA) {
                match(COMMA, file);
                parse_Exp(file);
            }
            match(DO, file);
            parse_Block(file);
            match(END, file);
            break;
        case LOCAL:
            match(LOCAL, file);
            if (currentToken.type == FUNCTION) {
                match(FUNCTION, file);
                parse_Name(file);
                parse_FunctionBody(file);
            } else {
                parse_Vars(file);
                match(ASSIGN, file);
                parse_Exps(file);
            }
            break;
        default:
            parse_Vars(file);
            match(ASSIGN, file);
            parse_Exps(file);
            break;
    }
}

void parse_ElseIfBlocks(FILE *file) {
    while (currentToken.type == ELSEIF) {
        match(ELSEIF, file);
        parse_Exp(file);
        match(THEN, file);
        parse_Block(file);
    }
}

void parse_Exps(FILE *file) {
    parse_Exp(file);
    while (currentToken.type == COMMA) {
        match(COMMA, file);
        parse_Exp(file);
    }
}

void parse_Exp(FILE *file) {
    if (currentToken.type == NUMBER || currentToken.type == STRING) {
        match(currentToken.type, file);
    } else if (currentToken.type == NOT || currentToken.type == MINUS) {
        match(currentToken.type, file);
        parse_Exp(file);
    } else {
        parse_PrefixExp(file);
        if (currentToken.type >= OR && currentToken.type <= POWER) {
            parse_BinOp(file);
            parse_Exp(file);
        }
    }
}

void parse_PrefixExp(FILE *file) {
    if (currentToken.type == LPAREN) {
        match(LPAREN, file);
        parse_Exp(file);
        match(RPAREN, file);
    } else {
        parse_Var(file);
    }
}

void parse_Field(FILE *file) {
    if (currentToken.type == LBRACKET) {
        match(LBRACKET, file);
        parse_Exp(file);
        match(RBRACKET, file);
        match(ASSIGN, file);
        parse_Exp(file);
    } else {
        parse_Name(file);
        match(ASSIGN, file);
        parse_Exp(file);
    }
}

void parse_BinOp(FILE *file) {
    switch (currentToken.type) {
        case OR:
        case AND:
        case LT:
        case GT:
        case LE:
        case GE:
        case NE:
        case EQ:
        case CONCAT:
        case PLUS:
        case MINUS:
        case TIMES:
        case DIVIDE:
        case POWER:
            match(currentToken.type, file);
            break;
        default:
            fprintf(stderr, "Error: Expected BinOp, but found '%s'\n", currentToken.lexeme);
            exit(EXIT_FAILURE);
    }
}

void parse_Vars(FILE *file) {
    parse_Var(file);
    while (currentToken.type == COMMA) {
        match(COMMA, file);
        parse_Var(file);
    }
}

void parse_Var(FILE *file) {
    if (currentToken.type == NAME) {
        parse_Name(file);
        if (currentToken.type == ASSIGN) {
            match(ASSIGN, file);
            parse_Exp(file);
        }
    } else if (currentToken.type == LPAREN) {
        match(LPAREN, file);
        parse_Exp(file);
        match(RPAREN, file);
    } else if (strcmp(currentToken.lexeme, "var") == 0) {
        match(VAR, file);
        if (currentToken.type == NAME) {
            parse_Name(file);
            if (currentToken.type == ASSIGN) {
                match(ASSIGN, file);
                parse_Exp(file);
            }
        } else {
            fprintf(stderr, "Error: Expected Var name after 'var', but found '%s'\n", currentToken.lexeme);
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "Error: Expected Var, but found '%s'\n", currentToken.lexeme);
        exit(EXIT_FAILURE);
    }
}

void parse_Function(FILE *file) {
    match(FUNCTION, file);
    parse_FunctionBody(file);
}

void parse_FunctionBody(FILE *file) {
    parse_Name(file);
    if (currentToken.type == LPAREN) {
        match(LPAREN, file);
        parse_Params(file);
        match(RPAREN, file);
    }
    parse_Block(file);
    match(END, file);
}

void parse_Params(FILE *file) {
    parse_Names(file);
}

void parse_Names(FILE *file) {
    parse_Name(file);
    while (currentToken.type == COMMA) {
        match(COMMA, file);
        parse_Name(file);
    }
}

void parse_Name(FILE *file) {
    if (currentToken.type == NAME) {
        match(NAME, file);
    } else {
        fprintf(stderr, "Error: Expected Name, but found '%s'\n", currentToken.lexeme);
        exit(EXIT_FAILURE);
    }
}

// Função para relatar um erro
void reportError(const char *message, int line, int position) {
    fprintf(stderr, "Error at line %d, position %d: %s\n", line, position, message);
}

void panicMode(TokenType syncSet[], int size, FILE *file) {
    while (currentToken.type != END_OF_INPUT) {
        for (int i = 0; i < size; ++i) {
            if (currentToken.type == syncSet[i] || currentToken.type == SEMICOLON) {
                return;
            }
        }
        if (currentToken.type == SEMICOLON) {
            return;
        }
        reportError("Panic mode: Skipping unexpected token.", currentToken.line, currentToken.position);

        advance(file);
    }
}

// Função principal para análise sintática
void parseProgram(FILE *file) {
    advance(file);  // Inicializa o token atual
    parse_Block(file);

    // Verifica se o último token é o esperado para o fim do programa
    if (currentToken.type != END_OF_INPUT) {
        reportError("Unexpected tokens after the end of the program.", currentToken.line, currentToken.position);
    }
}

int main() {
    const char *fileNames[] = {
      "function.txt", 
      "loop.txt", 
      "control.txt",
      "table.txt"
    };
    const int numFiles = sizeof(fileNames) / sizeof(fileNames[0]);

    for (int i = 0; i < numFiles; ++i) {
        FILE *file = fopen(fileNames[i], "r");

        if (file == NULL) {
            perror("Error opening file");
            return EXIT_FAILURE;
        }

        // Leitura do código-fonte do arquivo
        size_t bytesRead = fread(input, 1, sizeof(input) - 1, file);
        input[bytesRead] = '\0';  // Adiciona terminador de string

        // Chama o analisador sintático
        parseProgram(file);

        fclose(file);

        printf("Parsing successful for %s!\n", fileNames[i]);
    }

    return 0;
}