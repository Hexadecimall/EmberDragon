#include <stdint.h>

/* Whitespace/delimiter tokenizer producing token spans into a fixed buffer. */

#define MAX_TOKENS 64

typedef struct Token {
    int start;
    int length;
    int kind;
} Token;

typedef struct TokenStream {
    Token tokens[MAX_TOKENS];
    int count;
} TokenStream;

enum TokenKind {
    TOKEN_WORD = 1,
    TOKEN_NUMBER = 2,
    TOKEN_SYMBOL = 3
};

static int is_space(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
}

static int is_digit(char ch) {
    return ch >= '0' && ch <= '9';
}

static int is_letter(char ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
}

static int classify(char ch) {
    if (is_digit(ch)) {
        return TOKEN_NUMBER;
    }
    if (is_letter(ch)) {
        return TOKEN_WORD;
    }
    return TOKEN_SYMBOL;
}

void tokenize(const char *source, TokenStream *stream) {
    stream->count = 0;
    int index = 0;
    while (source[index] != '\0' && stream->count < MAX_TOKENS) {
        char current = source[index];
        if (is_space(current)) {
            index++;
            continue;
        }
        int kind = classify(current);
        int token_start = index;
        if (kind == TOKEN_SYMBOL) {
            index++;
        } else {
            while (source[index] != '\0' && classify(source[index]) == kind) {
                index++;
            }
        }
        Token *slot = &stream->tokens[stream->count];
        slot->start = token_start;
        slot->length = index - token_start;
        slot->kind = kind;
        stream->count++;
    }
}

int count_tokens_of_kind(const TokenStream *stream, int kind) {
    int total = 0;
    for (int i = 0; i < stream->count; i++) {
        if (stream->tokens[i].kind == kind) {
            total++;
        }
    }
    return total;
}

int longest_token_length(const TokenStream *stream) {
    int best = 0;
    for (int i = 0; i < stream->count; i++) {
        if (stream->tokens[i].length > best) {
            best = stream->tokens[i].length;
        }
    }
    return best;
}
