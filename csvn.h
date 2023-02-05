/*

Copyright 2023 Mahir Salihbasic

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of 
the Software, and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR 
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
OTHER DEALINGS IN THE SOFTWARE.

*/

#ifndef CSVN_H
#define CSVN_H

#include <stddef.h>

#define CSVN_NEWLINE '\n'

/*
	While this is a CSV parser, it technically could support other 
	delimeters like tabulators. 

	Changing the char to another character should be enough,
	however there are no guarantees that the parser will work fine afterwards.
*/
#define CSVN_DELIM ','

#ifdef __cplusplus
extern "C" {
#endif

/*

	NOT_ENOUGH_MEM - not enough tokens were allocated 

	INVALID_CHARACTER - an invalid character was encountered
	                    (check parser position for further analysis)

*/
enum csv_err {
	
	NOT_ENOUGH_MEM = -1,
	INVALID_CHARACTER = -2

} csv_err;

/*

	UNASSIGN - no field has been assigned to this token
	
	DQUOTE - this token holds a quoted field

	TEXT - this token holds an unquoted field

	EMPTY - this token is an empty field

*/
enum csv_tok {

	UNASSIGN,

	DQUOTE,

	TEXT,

	EMPTY

} csv_tok;

/*

	pos - current parser position in the text

	toknext - index of the next token to be allocated

	line - current line of text in which the parser operates

*/
struct csv_p {

	int pos;

	int toknext;

	int line;

} csv_p;

/*

	start - starting (inclusive) position of the field

	end - ending (inclusive) position of the field

	line - line in which the field was located

	size - exact length of the field (end - start)

	token - field type

*/
struct csv_t {
	
	int start;
	
	int end;

	int line;

	int size;

	enum csv_tok token;

} csv_t;

/*

	Allocates the next token from the provided token pool and initialises it 
	to default starting values.

	Returns a pointer to the allocated token if the allocation was 
	successful and NULL otherwise.

*/
static struct csv_t *csvn_allocate_token(struct csv_p *csv_p, 
		                         struct csv_t *tokens, 
					 const size_t num_tok);

/*

	Fills the provided (non-NULL) token with the provided data.

	Refer to csv_t documentation for a brief explanation of parameters.

*/
static void csvn_fill_token(struct csv_t *csv_t, 
		            int start, 
			    int end, 
			    int line, 
			    enum csv_tok token);

static int csvn_parse_quotes(const char *text, 
		             const size_t textlen, 
			     struct csv_p *csv_p, 
			     struct csv_t *tokpool, 
			     const size_t num_tok);

static int csvn_parse_normal(const char *text, 
		             const size_t textlen, 
			     struct csv_p *csv_p, 
			     struct csv_t *tokpool, 
			     const size_t num_tok);

/*

	Initialises the provided (non-NULL) parser to default starting values.
	
	This function should also be used to reset the parser in case of 
	repeated use.

*/
void csvn_init(struct csv_p *csv_p);

/*

	Parses the provided text (without exceeding textlen) and stores 
	the parsed fields in tokens array pointed to by tokpool which contains 
	at least num_tok tokens.

	If tokpool is NULL, it will parse the text, but will not store any 
	tokens in the tokpool. This is useful when the number of tokens is not 
	initially known.

	Returns the number of parsed fields (always greater than or equal to 0) 
	or a negative value indicating an error (refer to csv_err).

*/
int csvn_parse(const char *text, 
	       const size_t textlen, 
	       struct csv_p *csv_p, 
	       struct csv_t *tokpool, 
	       const size_t num_tok);

void
csvn_init(struct csv_p *csv_p)
{

	csv_p->pos = 0;
	csv_p->toknext = 0;
	csv_p->line = 1;
}

static struct csv_t *
csvn_allocate_token(struct csv_p *csv_p, 
		    struct csv_t *tokens, 
		    const size_t num_tok) 
{

	if (tokens == NULL) {
		return NULL;
	}

	if (csv_p->toknext > num_tok) {
		return NULL;
	}

	struct csv_t *token = &tokens[csv_p->toknext++];
	token->start = token->end = token->line = token->size = 0;
	token->token = UNASSIGN;

	return token;

}

static void
csvn_fill_token(struct csv_t *csv_t, 
		int start, 
		int end, 
		int line, 
		enum csv_tok token)
{

	csv_t->start = start;
	csv_t->end = end;
	csv_t->line = line;
	csv_t->size = (end - start);
	csv_t->token = token;	

}

static int
csvn_parse_quotes(const char *text, 
		  const size_t textlen, 
		  struct csv_p *csv_p, 
		  struct csv_t *tokpool, 
		  const size_t num_tok)
{
		
	struct csv_t *token;
	int start, end;
	start = csv_p->pos;
	
	int line = csv_p->line;

	while (text[csv_p->pos] != '\0' && csv_p->pos < textlen) {
		
		if (text[csv_p->pos] == '\"' && text[csv_p->pos + 1] == '\"') {
		
			csv_p->pos += 2;
			continue;

		}

		if (text[csv_p->pos] == '\"' && text[csv_p->pos + 1] != '\"') {

			break;

		}
		
		if (text[csv_p->pos] == CSVN_NEWLINE) {
			#ifdef CSVN_CONSIDER_NL
			line++
			#endif
			csv_p->line++;
		}

		csv_p->pos++;
		
	}

	end = csv_p->pos - 1; /* curent pos is at the closing quote */

	if (tokpool != NULL) {
		
		token = csvn_allocate_token(csv_p, tokpool, num_tok);
		if (token == NULL) {
			return NOT_ENOUGH_MEM; 
		}

		csvn_fill_token(token, start, end, line, DQUOTE);
	}

	return 0;

}


static int 
csvn_parse_normal(const char *text, 
		  const size_t textlen, 
		  struct csv_p *csv_p, 
		  struct csv_t *tokpool, 
		  const size_t num_tok)
{

	struct csv_t *token;
	int start, end;
	start = csv_p->pos;

	int line = csv_p->line;

	for (; text[csv_p->pos] != '\0' && 
	       csv_p->pos < textlen && 
	       text[csv_p->pos] != CSVN_NEWLINE && 
	       text[csv_p->pos] != CSVN_DELIM; csv_p->pos++) {
	
		#ifdef CSVN_STRICT
		if (text[csv_p->pos] == '\"') {
			return INVALID_CHARACTER;
		}
		#endif
		continue;
	
	}		

	/* let the main loop handle newlines and delimeters */
	if (text[csv_p->pos] == CSVN_NEWLINE || text[csv_p->pos] == CSVN_DELIM) {
		csv_p->pos--;
	}

	end = csv_p->pos;
	
	if (tokpool != NULL) {
		
		token = csvn_allocate_token(csv_p, tokpool, num_tok);
		if (token == NULL) {
			return NOT_ENOUGH_MEM; 
		}

		csvn_fill_token(token, start, end, line, DQUOTE);
	}	
	
	return 0;

}

int
csvn_parse(const char *text, 
	   const size_t textlen, 
	   struct csv_p *csv_p, 
	   struct csv_t *tokpool, 
	   const size_t num_tok)
{

	int parsed = 0;
	int res = 0;

	while(text[csv_p->pos] != '\0' && csv_p->pos < textlen) {
	
		switch (text[csv_p->pos]) {
	
		case CSVN_NEWLINE:
			csv_p->line++;
			csv_p->pos++;
			break;

		case CSVN_DELIM:
			csv_p->pos++;
			
			#ifdef CSVN_SKIP_WHITESPACE
			if (text[csv_p->pos] == ' ') {
				
				do {
					csv_p->pos++;
				} while (text[csv_p->pos] == ' ');

			}
			#endif

			if (text[csv_p->pos] == CSVN_DELIM) {

			#ifdef CSVN_NO_EMPTY_FIELD
				return INVALID_CHARACTER;
			#elif CSVN_IGNORE_EMPTY_FIELD
				break;
			#else
				if (tokpool != NULL) {
					struct csv_t *tok;
					tok = csvn_allocate_token(csv_p, tokpool, num_tok);
					if (tok == NULL) {
						return NOT_ENOUGH_MEM;
					}
					csvn_fill_token(tok, csv_p->pos - 1, csv_p->pos, csv_p->line, EMPTY);
				}
				parsed++;
				break;
			#endif	

			}
			break;

		case '\"':
			csv_p->pos++; /* skip opening quote */
			res = csvn_parse_quotes(text, textlen, csv_p, tokpool, num_tok);
			if (res != 0) {
				return res;
			}
			csv_p->pos++;
			#ifdef CSVN_STRICT
			/* closing quote has to be followed by a delimiter, newline or end of file */
			if (text[csv_p->pos] == CSVN_DELIM || text[csv_p->pos] == CSVN_NEWLINE) {
				parsed++;
				break;
			} else {
				if (text[csv_p->pos] != '\0' && csv_p->pos < textlen) {
					return INVALID_CHARACTER;
				}
			}
			#endif
			parsed++;
			break;
		default:
			res = csvn_parse_normal(text, textlen, csv_p, tokpool, num_tok);
			if (res != 0) {
				return res;
			}
			parsed++;
			csv_p->pos++;
			break;

		}

	}

	return parsed;

}

#ifdef __cplusplus
}
#endif

#endif /* ifndef CSVN_H */
