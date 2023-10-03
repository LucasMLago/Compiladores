#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

// Definições dos tipos de token usando #define
#define IDENTIFIER 256
#define KEYWORD 257
#define NUMBER 258
#define STRING_LITERAL 259
#define DELIMITER 260
#define OPERATOR 261
#define COMMENT 262
#define EOF_TOKEN 263

// semicolon e IF
#define SEMICOLON ";"
#define IF "IF"

// Constante para o tamanho máximo do valor do token
#define MAX_VALUE_LENGTH 100

// Estrutura para representar um token
struct Token {
    int type;
    char value[MAX_VALUE_LENGTH];
    int attribute;  // Atributo para identificadores
};

// Função para converter uma string para letras minúsculas
void toLowerCase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

// Autômato para verificar se uma palavra é uma palavra-chave
bool isKeyword(const char* word) {
    const char* keywords[] = {
        "and", "break", "do", "else", "elseif",
        "end", "false", "for", "function", "if",
        "in", "local", "nil", "not", "or",
        "repeat", "return", "then", "true", "until", "while"
    };
    int numKeywords = sizeof(keywords) / sizeof(keywords[0]);
    char lowercaseWord[MAX_VALUE_LENGTH];
    strcpy(lowercaseWord, word);
    toLowerCase(lowercaseWord);
    for (int i = 0; i < numKeywords; i++) {
        if (strcmp(lowercaseWord, keywords[i]) == 0) {
            return true;
        }
    }
    return false;
}

// Autômato para verificar se o lexema é uma string ou literal de string
bool isStringLiteral(const char* frase) {
    // Verifica se o lexema começa e termina com aspas duplas
    if (frase[0] == '"' || frase[strlen(frase) - 1] == '"') {
        return true;
    }
    return false;
}

// Autômato para verificar se o lexema é um número
bool isNum(const char* lexema) {
    int i = 0;
    int dotCount = 0;  // Contador de pontos (para verificar floats)

    // Verifica se o primeiro caractere é um sinal de mais ou menos
    if (lexema[i] == '+' || lexema[i] == '-') {
        i++;
    }

    // Verifica os dígitos antes do ponto decimal
    while (isdigit(lexema[i])) {
        i++;
    }

    // Verifica o ponto decimal e dígitos após o ponto (para floats)
    if (lexema[i] == '.') {
        dotCount++;
        i++;
        while (isdigit(lexema[i])) {
            i++;
        }
    }

    // Verifica se o número é válido (deve ter pelo menos um dígito)
    if (i > 0 && dotCount <= 1) {
        return true;
    }

    return false;
}

// Autômato para verificar se um lexema é um operador
bool isOperator(char c) {
    const char* operators = "+-*/=<>~;";
    return strchr(operators, c) != NULL;
}

// Autômato para verificar se um lexema é um delimitador
bool isDelimiter(char c) {
    const char* delimiters = "()[]{}:.,";
    return strchr(delimiters, c) != NULL;
}

// Autômato para verificar se um lexema é um identificador
bool isIdentifier(const char* var) {
    // Verifica se o lexema não está vazio e começa com uma letra ou sublinhado
    if (var[0] == '\0' || (!isalpha(var[0]) && var[0] != '_')) {
        return false;
    }

    // Verifica os caracteres restantes do lexema
    for (int i = 1; var[i] != '\0'; i++) {
        if (!isalnum(var[i]) && var[i] != '_') {
            return false;
        }
    }

    return true;
}

// Autômato para verificar se um lexema é um comentário
bool isComment(const char* lexema) {
    // Verifica se o lexema começa com "--"
    if (strncmp(lexema, "--", 2) == 0) {
        return true; // Comentário curto
    }

    // Se o texto imediatamente após "--" for "[[", é um comentário longo
    if (strncmp(lexema, "--[[", 4) == 0) {
        // Verifica se o comentário longo está fechado com "]]"
        const char* end = strstr(lexema + 4, "]]");
        if (end != NULL) {
            return true; // Comentário longo fechado
        }
    }

    return false; // Não é um comentário
}

// Autômato para analisar um lexema e retornar o token correspondente

struct Token analisaToken(const char* lexema, int linha) {
    struct Token token;

    if (isKeyword(lexema)) {
        token.type = KEYWORD;
    } else if (isStringLiteral(lexema)) {
        token.type = STRING_LITERAL;
    } else if (isNum(lexema)) {
        token.type = NUMBER;
    } else if (isComment(lexema)) {
        token.type = COMMENT;
    } else if (isOperator(lexema[0])) {
        token.type = OPERATOR;
    } else if (isDelimiter(lexema[0])) {
        token.type = DELIMITER;
    } else if (isIdentifier(lexema)) {
        token.type = IDENTIFIER;
        token.attribute = linha;  // Atributo para identificadores (linha na tabela de símbolos)
    } else {
        // Verifica se é um dos tokens específicos
        if (strlen(lexema) == 1) {
            if (lexema[0] == ';') {
                token.type = SEMICOLON;
            } else {
                token.type = IF;
            }
        } else {
            token.type = EOF_TOKEN;
        }
    }

    // Se for um identificador, o valor do token será "<ID, linha>"
    if (token.type == IDENTIFIER) {
        snprintf(token.value, MAX_VALUE_LENGTH, "<ID, %d>", token.attribute);
    } else {
        strncpy(token.value, lexema, MAX_VALUE_LENGTH);
    }
    
    token.value[MAX_VALUE_LENGTH - 1] = '\0'; // Garante que o valor seja nul-terminated
    return token;
}

int main() {
    FILE* arquivo = fopen("codigo_fonte.txt", "r"); // Abre o arquivo de texto

    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return 1;
    }

    char* linha = NULL;
    size_t tamanho_buffer = 0;
    int linha_atual = 0;

    while (getline(&linha, &tamanho_buffer, arquivo) != -1) {
        linha_atual++;

        char* token = strtok(linha, " \t\n");
        while (token != NULL) {
            struct Token analisa_Token = analisaToken(token, linha_atual);

            // Imprime Token
            if (strcmp(analisa_Token.value, SEMICOLON) == 0)
                printf("<%s,>\n", analisa_Token.value);
            else if (strcmp(analisa_Token.value, IF) == 0)
                printf("<%s,>\n", analisa_Token.value);
            else
                printf("<%s,%d>\n", analisa_Token.value, analisa_Token.type);

            token = strtok(NULL, " \t\n");
        }
    }

    free(linha); //liberando a memória alocada
    fclose(arquivo); // Fecha o arquivo

    return 0;
}