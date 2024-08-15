#ifndef LEXER_H
#define LEXER_H

#include <string.h>
#include "tokens.h"

size_t check_number (FILE *file_ptr, char *value_ptr) {
	// First digit has already been read
	char digit = *value_ptr;
	size_t length = 0;
	
	while (('0' <= digit && digit <= '9') || digit == '_') {
		length += (digit != '_');
		fread(&digit, sizeof(char), 1, file_ptr);
	}
	
	char first_char = *value_ptr;
	value_ptr = (char*) malloc(sizeof(char) * (length + 1));
	
	value_ptr[0] = first_char;
	if (length > 0) {
		fread(value_ptr + 1, sizeof(char) * (length - 1), 1, file_ptr);
	}
	value_ptr[length] = '\0';
	
	return length;
}

size_t check_identifier (FILE *file_ptr, char *value_ptr) {
	// First letter has already been read
	char letter = *value_ptr;
	size_t length = 0;
	
	while (	('a' <= letter && letter <= 'z') || 
			('A' <= letter && letter <= 'Z') || 
			('0' <= letter && letter <= '9') || 
			letter == '_' )
	{
		length++;
		fread(&letter, sizeof(char), 1, file_ptr);
	}
	
	char first_char = *value_ptr;
	value_ptr = (char*) malloc(sizeof(char) * (length + 1));
	
	value_ptr[0] = first_char;
	if (length > 0) {
		fread(value_ptr + 1, sizeof(char) * (length - 1), 1, file_ptr);
	}
	value_ptr[length] = '\0';
	
	return length;
}

char next_token (FILE *file_ptr, char *value_ptr) {
	char token;
	
	fread(&token, sizeof(char), 1, file_ptr);
	while (token == ' ' || token == '\t' || token == '\n') {
		fread(&token, sizeof(char), 1, file_ptr);
	}
	
	switch (token) {
	case '+':
	case '-':
	case '*':
	case '/':
	case '<':
	case '>':
	case '=':
	case ';':
	case ',':
	case '.':
	case '(':
	case ')':
		return token;
	case '{':
		while (token != '}') {
			fread(token, sizeof(char), 1, file_ptr);
		}
		return next_token(file_ptr);
	case ':':
		char next_tok = next_token(file_ptr);
		if (next_tok != '=') {
			fprintf(stderror, "Invalid token \':%c\'\n", next_tok);
			exit(1);
		}
		return TOK_ASSIGN;
	default:
		value_ptr = &token;
		
		if (check_identifier(file_ptr, value_ptr)) {
			if (strcmp(value_ptr, "const") == 0) {
				return TOK_CONST;
			}
			if (strcmp(value_ptr, "var") == 0) {
				return TOK_VAR;
			}
			if (strcmp(value_ptr, "procedure") == 0) {
				return TOK_PROCEDURE;
			}
			return TOK_IDENT;
		}
		
		if (check_number(file_ptr, value_ptr)) {
			return TOK_NUM;
		}
		
		fprintf(stderror, "Invalid token \'%c\'\n", token);
		exit(1);
	}
}

#endif /* LEXER_H */