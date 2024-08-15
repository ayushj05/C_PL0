#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

/*
 * program		= block "." .
 
 * block		= [ "const" ident "=" number { "," ident "=" number } ";" ]
 		  		  [ "var" ident { "," ident } ";" ]
 		  		  { "procedure" ident ";" block ";" } statement .
 
 * statement	= [ ident ":=" expression
 		  		  | "call" ident
 		  		  | "begin" statement { ";" statement } "end"
 		  		  | "if" condition "then" statement
 		  		  | "while" condition "do" statement ] .
 
 * condition	= "odd" expression
 				| expression ( "=" | "#" | "<" | ">" ) expression .
 
 * expression	= [ "+" | "-" ] term { ( "+" | "-" ) term } .
 
 * term			= factor { ( "*" | "/" ) factor } .
 
 * factor		= ident
 				| number
 				| "(" expression ")" .
 */

char expect (FILE *file_ptr, char *value_ptr, char expected) {
	char *token_ptr;
	if (next_token(token_ptr, value_ptr) != expected) {
		fprintf(stderror, "Syntax error\n");
		exit(1);
	}
	return expected;
}


/*
factor	= ident
 		| number
 		| "(" expression ")" .
*/
void handle_factor (FILE *file_ptr) {
	char *value_ptr;
	char token_type = next_token(file_ptr, value_ptr);
	
	switch (token_type) {
	case TOK_IDENT:
	case TOK_NUM:
		break;
	case TOK_LPAREN:
		handle_expression(file_ptr);
		expect(file_ptr, value_ptr, TOK_RPAREN);
	}
}


/*
term = factor { ( "*" | "/" ) factor } .
*/
void handle_term (FILE *file_ptr) {
	char *value_ptr;
	char token_type;
	
	handle_factor(file_ptr);
	
	FILE *prev_file_ptr = file_ptr;
	token_type = next_token(file_ptr, value_ptr);
	
	while (token_type == TOK_MULT || token_type == TOK_DIV) {
		handle_factor(file_ptr);
		
		prev_file_ptr = file_ptr;
		token_type = next_token(file_ptr, value_ptr);
	}
	
	file_ptr = prev_file_ptr;
	
	free(value_ptr);
}


/*
expression = [ "+" | "-" ] term { ( "+" | "-" ) term } .
*/
void handle_expression (FILE *file_ptr) {
	char *value_ptr;
	FILE *prev_file_ptr = file_ptr;
	char token_type = next_token(file_ptr, value_ptr);
	
	if (token_type == TOK_ADD || token_type == TOK_SUBT) {
		file_ptr = prev_file_ptr;
	}
	
	handle_term(file_ptr);
	
	prev_file_ptr = file_ptr;
	token_type = next_token(file_ptr, value_ptr);
	
	while (token_type == TOK_ADD || token_type == TOK_SUBT) {
		handle_term(file_ptr);
		
		prev_file_ptr = file_ptr;
		token_type = next_token(file_ptr, value_ptr);
	}
	
	file_ptr = prev_file_ptr;
	
	free(value_ptr);
}


/*
condition = "odd" expression
 		  | expression ( "=" | "#" | "<" | ">" ) expression .
*/
void handle_condition (FILE *file_ptr) {
	char *value_ptr;
	FILE *prev_file_ptr = file_ptr;
	char token_type = next_token(file_ptr, value_ptr);
	
	if (token_type == TOK_ODD) {
		handle_expression(file_ptr);
	}
	else {
		file_ptr = prev_file_ptr;
		
		handle_expression(file_ptr);
		token_type = next_token(file_ptr, value_ptr);
		
		switch (token_type) {
		case TOK_EQUAL:
		case TOK_HASH:
		case TOK_LESS:
		case TOK_GREATER:
			handle_expression(file_ptr);
			break;
		default:
			fprintf(stderror, "Invalid condition\n");
			exit(1);
		}
	}
	
	free(value_ptr);
}


/*
statement = [ ident ":=" expression
 		    | "call" ident
 		    | "begin" statement { ";" statement } "end"
 		    | "if" condition "then" statement
 		    | "while" condition "do" statement ] .
*/
void handle_statement (FILE *file_ptr) {
	char *value_ptr;
	char token_type = next_token(file_ptr, value_ptr);
	
	switch (token_type) {
	case TOK_IDENT:
		expect(file_ptr, value_ptr, TOK_ASSIGN);
		handle_expression(file_ptr);
		break;
		
	case TOK_CALL:
		expect(file_ptr, value_ptr, TOK_IDENT);
		break;
		
	case TOK_BEGIN:
		do {
			handle_statement(file_ptr);
		}
		while ((token_type = next_token(file_ptr, value_ptr)) == TOK_SEMICOLON);
		
		if (token_type != TOK_END) {
			fprintf(stderror, "Syntax error\n");
			exit(1);
		}
		break;
		
	case TOK_IF:
		handle_condition(file_ptr);
		expect(file_ptr, value_ptr, TOK_THEN);
		handle_statement(file_ptr);
		break;
		
	case TOK_WHILE:
		handle_condition(file_ptr);
		expect(file_ptr, value_ptr, TOK_DO);
		handle_statement(file_ptr);
		break;
	}
	
	free(value_ptr);
}


/*
block = [ "const" ident "=" number { "," ident "=" number } ";" ]
 	    [ "var" ident { "," ident } ";" ]
 	    { "procedure" ident ";" block ";" } statement .
*/
void handle_block (FILE *file_ptr) {
	char *value_ptr;
	FILE *prev_file_ptr = file_ptr;
	char token_type = next_token(file_ptr, value_ptr);
	
	// [ "const" ident "=" number { "," ident "=" number } ";" ]
	if (token_type == TOK_CONST) {
		do {
			expect(file_ptr, value_ptr, TOK_IDENT);
			expect(file_ptr, value_ptr, TOK_EQUAL);
			expect(file_ptr, value_ptr, TOK_NUM);
		}
		while ((token_type = next_token(file_ptr, value_ptr)) == TOK_COMMA);
		
		if (token_type != TOK_SEMICOLON) {
			fprintf(stderror, "Syntax error\n");
			exit(1);
		}
	}
		
	// [ "var" ident { "," ident } ";" ]
	if (token_type == TOK_VAR) {
		do {
			expect(file_ptr, value_ptr, TOK_IDENT);
		}
		while ((token_type = next_token(file_ptr, value_ptr)) == TOK_COMMA);
		
		if (token_type != TOK_SEMICOLON) {
			fprintf(stderror, "Syntax error\n");
			exit(1);
		}
	}
	
	// { "procedure" ident ";" block ";" } statement .
	while (token_type == TOK_PROCEDURE) {
		expect(file_ptr, value_ptr, TOK_IDENT);
		expect(file_ptr, value_ptr, TOK_SEMICOLON);
		handle_block(file_ptr);
		expect(file_ptr, value_ptr, TOK_SEMICOLON);
		
		prev_file_ptr = file_ptr;
		token_type = next_token(file_ptr, value_ptr);
	}
	
	file_ptr = prev_file_ptr;
	
	handle_statement(file_ptr);
	
	free(value_ptr);
}

void parser (FILE *file_ptr) {
	handle_block(file_ptr);
	expect(TOK_DOT);
}

#endif /* PARSER_H */