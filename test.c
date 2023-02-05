#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CSVN_STRICT
#include "csvn.h"

#define fail() return __LINE__
#define done() return 0

#define check(cond) 			\
	do {				\
		if (!(cond)) {		\
			fail();		\
		}			\
	} while (0)

static int passed_tests = 0;
static int failed_tests = 0;

static char *get_parsed(struct csv_t *token, char *text);

static int test_normal();
static int test_quoted();
static int test_newlines();

static void test(int (*testf)(void), char *msg);

static void 
test(int (*testf)(void), char *msg)
{
	
	int res;
	res = testf();

	if (res == 0) {
		passed_tests++;
	} else {
		failed_tests++;
		printf("Failed test %s at line %d\n", msg, res);
	}

}

static char * 
get_parsed(struct csv_t *token, char *text)
{

	char *parsed;
        parsed = malloc(sizeof(char *) * (token->size + 1));
	parsed[token->size] = '\0';

	strncpy(parsed, text + token->start, token->size + 1);

	return parsed;

}

static int
test_normal()
{

	struct csv_t tokens[6];	
	struct csv_p parser;
	csvn_init(&parser);

	/* parse, this, text
	   then, this 
	*/
	char *test_text;
	test_text = "parse, this, text\nthen,this";

	int parsed;

	parsed = csvn_parse(test_text, strlen(test_text), &parser, tokens, 0);
	printf("Parsed %d normal tokens\n", parsed);
	check(parsed == 5);

	csvn_init(&parser);
	parsed = csvn_parse(test_text, strlen(test_text), &parser, tokens, 6);

	printf("Parsed %d normal tokens\n", parsed);
	check(parsed == 5);

	for (int i = 0; i < parsed; i++) {
	
		char *parsed = get_parsed(&tokens[i], test_text);
		printf("Parsed token: %s at line %d\n", parsed, tokens[i].line);
		free(parsed);		

	}

	done();
}

static int 
test_quoted()
{

	struct csv_t tokens[6];
	struct csv_p parser;
	csvn_init(&parser);

	/*
		"parse, this"" text"
		"then parse this text"
	*/
	char *test_text = "\"parse, this\"\" text\"\n\"then parse this text\"";
	
	int parsed = csvn_parse(test_text, strlen(test_text), &parser, tokens, 6);

	printf("Parsed %d quoted tokens\n", parsed);
	check(parsed == 2);

	for (int i = 0; i < parsed; i++) {
		
		char *parsed = get_parsed(&tokens[i], test_text);
		printf("Parsed token: %s (start: %d, end: %d)\n", parsed, tokens[i].start, tokens[i].end);
		free(parsed);

	}

	done();

}

static int 
test_newlines()
{

	struct csv_t tokens[11];
	struct csv_p parser;
	csvn_init(&parser);

	/*
		parse,this,text
		then,this
		then,finally,this
		and,then
		this
	*/
	char *test_text = "parse,this,text\nthen,this\nthen,finally,this\nand,then\nthis";

	int parsed = csvn_parse(test_text, strlen(test_text), &parser, tokens, 11);
	printf("Parsed %d newline tokens\n", parsed);
	check(parsed == 11);
	
	for (int i = 0; i < parsed; i++) {
		
		char *parsed = get_parsed(&tokens[i], test_text);
		printf("Parsed token: %s (start: %d, end: %d, line: %d)\n", parsed, tokens[i].start, tokens[i].end, tokens[i].line);
		free(parsed);

	}

	done();

}

int 
main(int argc, char **argv) 
{
	/*
	test(test_normal, "parsing of normal line");
	test(test_quoted, "parsing of quoted lines");
	test(test_newlines, "parsing of newlines");
	*/
	printf("Parsing whole file:\n\n");
	struct csv_t tokens[1000];
	struct csv_p parser;
	csvn_init(&parser);
	
	char *text = strdup(argv[1]);

	printf("Got input %s\n", text);

	int parsed = csvn_parse(text, strlen(text), &parser, tokens, 10);
	printf("Parsed %d tokens from the file.", parsed);

	for (int i = 0; i < parsed; i++) {
		
		char *parsed = get_parsed(&tokens[i], text);
		printf("Parsed token: %s (start: %d, end: %d, line: %d)\n", parsed, tokens[i].start, tokens[i].end, tokens[i].line);
		free(parsed);

	}
	
	free(text);

	return 0;
}
