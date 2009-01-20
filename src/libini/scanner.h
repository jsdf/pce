/*****************************************************************************
 * libini                                                                    *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/libini/scanner.h                                         *
 * Created:     2001-03-05 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2001-2009 Hampa Hug <hampa@hampa.ch>                     *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  General *
 * Public License for more details.                                          *
 *****************************************************************************/


#ifndef LIBINI_SCANNER_H
#define LIBINI_SCANNER_H 1


#include <stdio.h>
#include <stdlib.h>


#define SCN_BUF_N   16
#define SCN_TOK_MAX 256
#define SCN_EOF     256

#define SCN_TOK_NONE 0
#define SCN_TOK_CHAR 1


/*!***************************************************************************
 * @short The character set type
 *****************************************************************************/
typedef struct {
	unsigned char el[32];
} scn_set_t;


/*!***************************************************************************
 * @short The token class type
 *****************************************************************************/
typedef struct scn_class_t {
	struct scn_class_t *next;

	unsigned  cls_id;

	scn_set_t start;
	scn_set_t run;
} scn_class_t;


/*!***************************************************************************
 * @short The scanner type
 *****************************************************************************/
typedef struct scanner_t {
	void     (*del) (struct scanner_t *scn);
	unsigned (*get_char) (struct scanner_t *scn);
	void     (*scan) (struct scanner_t *scn);

	unsigned      tok_id;
	char          tok_str[SCN_TOK_MAX];

	unsigned      curcnt;
	unsigned      curchr[SCN_BUF_N];

	scn_set_t     white;
	scn_set_t     newline;

	scn_class_t   *cls;

	unsigned long line;
	unsigned long offset;

	void          *ext;
} scanner_t;


/*!***************************************************************************
 * @short The file scanner type
 *****************************************************************************/
typedef struct {
	scanner_t scn;
	FILE      *file;
} scn_file_t;


/*!***************************************************************************
 * @short The string scanner type
 *****************************************************************************/
typedef struct {
	scanner_t     scn;
	unsigned long i;
	unsigned long n;
	unsigned char *str;
} scn_str_t;


#define scn_tid(scn) ((scn)->tok_id)
#define scn_str(scn) ((scn)->tok_str)


/*!***************************************************************************
 * @short Clear a character set by removing all elements
 *****************************************************************************/
void scn_set_clear (scn_set_t *set);

/*!***************************************************************************
 * @short Add an element to a character set
 *****************************************************************************/
void scn_set_add_el (scn_set_t *set, unsigned el);

/*!***************************************************************************
 * @short Add a range of elements to a character set
 *****************************************************************************/
void scn_set_add_els (scn_set_t *set, unsigned el1, unsigned el2);

/*!***************************************************************************
 * @short Copy a character set
 *****************************************************************************/
void scn_set_copy (scn_set_t *dst, const scn_set_t *src);

/*!***************************************************************************
 * @short  Test if an element is in a character set
 * @return Nonzero if el is in the set
 *****************************************************************************/
int scn_set_is_el (const scn_set_t *set, unsigned el);


/*!***************************************************************************
 * @short Create a new token class with the specified ID
 *****************************************************************************/
scn_class_t *scn_cls_new (unsigned id);

/*!***************************************************************************
 * @short Create an exact copy of a token class
 *****************************************************************************/
scn_class_t *scn_cls_clone (const scn_class_t *cls);

/*!***************************************************************************
 * @short Delete a token class
 *****************************************************************************/
void scn_cls_del (scn_class_t *cls);


/*!***************************************************************************
 * @short Initialize a scanner structure
 *****************************************************************************/
void scn_init (scanner_t *scn);

/*!***************************************************************************
 * @short Create a new scanner
 *****************************************************************************/
scanner_t *scn_new (void);

/*!***************************************************************************
 * @short Free the resources used by a scanner structure
 *****************************************************************************/
void scn_free (scanner_t *scn);

/*!***************************************************************************
 * @short Delete a scanner
 *****************************************************************************/
void scn_del (scanner_t *scn);

/*!***************************************************************************
 * @short Get the character at index i in the look ahead buffer
 *****************************************************************************/
unsigned scn_get_chr (scanner_t *scn, unsigned i);

/*!***************************************************************************
 * @short Remove n characters from the look ahead buffer
 *****************************************************************************/
void scn_rmv_chr (scanner_t *scn, unsigned n);

/*!***************************************************************************
 * @short Check if the current token is of type SCN_TOK_CHAR and matches 'c'
 *****************************************************************************/
int scn_chr (scanner_t *scn, unsigned c);

/*!***************************************************************************
 * @short Check if the current token is of type id and matches str.
 *****************************************************************************/
int scn_tok (scanner_t *scn, unsigned id, const char *str);

/*!***************************************************************************
 * @short Set the white space character set
 *****************************************************************************/
void scn_set_white (scanner_t *scn, const scn_set_t *set);

/*!***************************************************************************
 * @short Add a token class
 *****************************************************************************/
void scn_class_add (scanner_t *scn, scn_class_t *cls);

/*!***************************************************************************
 * @short Set the current line number
 *****************************************************************************/
void scn_set_line (scanner_t *scn, unsigned long ln);

/*!***************************************************************************
 * @short Get the current line number
 *****************************************************************************/
unsigned long scn_get_line (const scanner_t *scn);

/*!***************************************************************************
 * @short Set the current offset
 *****************************************************************************/
void scn_set_offset (scanner_t *scn, unsigned long i);

/*!***************************************************************************
 * @short Get the current offset
 *****************************************************************************/
unsigned long scn_get_offset (const scanner_t *scn);

/*!***************************************************************************
 * @short Scan the next token
 *
 * This function is usually called through the function pointer in the
 * scanner struct.
 *****************************************************************************/
void scn_scan (scanner_t *scn);


/*!***************************************************************************
 * @short Create a new file scanner
 *****************************************************************************/
scanner_t *scnf_new (void);

/*!***************************************************************************
 * @short Delete a file scanner
 *****************************************************************************/
void scnf_del (scanner_t *scn);

/*!***************************************************************************
 * @short Get the next character from the file
 *****************************************************************************/
unsigned scnf_get_char (scanner_t *scn);

/*!***************************************************************************
 * @short Set the file to scan
 *****************************************************************************/
void scnf_set_file (scanner_t *scn, FILE *file);


/*!***************************************************************************
 * @short Create a new string scanner
 *****************************************************************************/
scanner_t *scns_new (const char *str);

/*!***************************************************************************
 * @short Delete a string scanner
 *****************************************************************************/
void scns_del (scanner_t *scn);

/*!***************************************************************************
 * @short  Get the next character from a string scanner
 * @return The next character or SCN_EOF
 *****************************************************************************/
unsigned scns_get_char (scanner_t *scn);


#endif
