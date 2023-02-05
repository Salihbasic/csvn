# csvn

A jsmn inspired CSV parser. It is minimal in size and memory use, easily integrated in all projects.

csvn aims to provide its users liberty in parsing CSV files while at the same time supporting
RFC 4180 CSV standard.

As of now, it is work in progress and still needs fine tuning to better take care of all the subtle
issues CSV files may pose, even though it should work well for any "sane" file as is.

# Why use csvn?

- Around 400 lines of pure C code
- No dependencies\*
- Portable
- Memory management left to the user
- Simple two function interface

\* Except for `stddef.h` used to provide `size_t`

# Usage

Include `csvn.h` in your file and call the following code:
```c

#include <string.h>
#include "csvn.h"

. . .

char *csv_text = "Field1,Field2,\"Field3, now quoted\"";

struct csvn_p parser;
struct csv_t fields[3]; /* parse up to 3 fields */

csvn_init(&parser); /* initialise the parser */
int count = csvn_parse(csv_text, strlen(csv_text), &parser, fields, 3);
```

# Documentation

## Types

### csv\_t

`csv_t` represents a single parsed field.
 
```c
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
```

Since both `start` and `end` are inclusive, it should be noted that `size` will not include
the space for extra null-character.

### csv\_p

`csv_p` represents the parser state.

```c

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
```

### csv\_tok

`csv_tok` represents the type of a parsed field.

```c
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
```

### csv\_err

`csv_err` holds the error values potentially returned by `csvn_parse`.

```c
/*

	NOT_ENOUGH_MEM - not enough tokens were allocated 

	INVALID_CHARACTER - an invalid character was encountered
	                    (check parser position for further analysis)

*/
enum csv_err {
	
	NOT_ENOUGH_MEM = -1,
	INVALID_CHARACTER = -2

} csv_err;
```

## Functions

### csvn_init

```c
/*

	Initialises the provided (non-NULL) parser to default starting values.
	
	This function should also be used to reset the parser in case of repeated use.

*/
void csvn_init(struct csv_p *csv_p);
```

Of course, you could always initialise the parser manually, especially if you need some specific offsets.

### csvn_parse

```c
/*

	Parses the provided text (without exceeding textlen) and stores the parsed fields in tokens array pointed
	to by tokpool which contains at least num_tok tokens.

	If num_tok is 0, it will parse the text, but will not store any tokens in the tokpool. This is useful when
	the number of tokens is not known.

	Returns the number of parsed fields (always greater than or equal to 0) or a negative value indicating
	an error (refer to csv_err).

*/
int csvn_parse(const char *text, const size_t textlen, struct csv_p *csv_p, struct csv_t *tokpool, const size_t num_tok);
```

When `num_tok` is `0`, you can freely pass `NULL` in place of `tokpool`.

## Macros

Several macros provide additional parser options for more control over how it should do its job. All of these macros should, of course,
be declared either on compilation or before inclusion of the header file.

### CSVN\_STRICT

`CSVN_STRICT` forces the parser to conform to RFC 4180 standard.

### CSVN\_NEWLINE

`CSVN_NEWLINE` defines the newline character that the parser should use. By default, it is `\n`.

### CSVN\_DELIM

`CSVN_DELIM` specifies the delimeter that the parser should use. Its default is a comma (`,`), since that is what CSV was
originally based upon. However, it technically could work with similar formats which replaces the comma by another single character
delimeter, like tab-separated values (TSV) by setting `CSVN_DELIM` to `\t` or something similar. 

Of course, it is up to you to implement any required modifications to make custom delimeters work.

### CSVN\_CONSIDER\_NL

`CSVN_CONSIDER_NL` deals with the specific case of a newline with a quoted field.

Consider this:
```
"these","fields","are in the same","line",
"but","this field
suddenly changes it","and then","moves on"
```

According to RFC 4180, this is perfectly legal, but how should one deal with the sudden change in line? Should that quoted field be
counted as if it started on one line or on the other? What about the fields that follow it? Should they be counted as if they changed their line
or as if they are still on the previous until a "true" unoquoted newline is encountered?

These are questions best answered by the person dealing with their specific data, and as such `CSVN_CONSIDER_NL` does the following:
- If it is undefined, the quoted field with the newline and all following fields until the end of the line will be counted as if they
are on the line in which the quoted field started
- If it is defined, the quoted field and all subsequent fields will be considered as if they are on the next line

### CSVN\_NO\_EMPTY\_FIELD

`CSVN_NO_EMPTY_FIELD` tells the parser to return an `INVALID_CHARACTER` error as soon as it encounters an empty field (i.e. `,,`)

### CSVN\_IGNORE\_EMPTY\_FIELD

`CSVN_IGNORE_EMPTY_FIELD` tells the parser to skip empty fields as if they are not there.

### CSVN\_SKIP\_WHITESPACE

`CSVN_SKIP_WHITESPACE` tells the parser to skip trailing whitespace in all fields.
