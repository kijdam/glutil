/*
 * str.h
 *
 *  Created on: Dec 4, 2013
 *      Author: reboot
 */

#ifndef STR_H_
#define STR_H_

#define REG_MATCHES_MAX                 4

#include <fp_types.h>

#include <stdio.h>

_d_achar_p g_basename, g_dirname;

_d_is_am is_ascii_text, is_ascii_lowercase_text, is_ascii_alphanumeric,
    is_ascii_hexadecimal, is_ascii_uppercase_text, is_ascii_numhex,
    is_ascii_numeric, is_ascii_numeric_float;

int
is_ascii_arith_bin_oper(char c);

int
remove_repeating_chars(char *string, char c);
char *
strcp_s(char *dest, size_t max_size, char *source);

char *
s_char(char *input, char c, size_t max);
off_t
s_string(char *input, char *m, off_t offset, size_t i_l);
off_t
s_string_r(char *input, char *m);
size_t
g_floatstrlen(char *in);

int
split_string(char *line, char dl, pmda output_t);
int
split_string_sp_tab(char *line, pmda output_t);
int
is_char_uppercase(char);
int
get_opr(char *in);
int
is_opr(char in);
int
is_comp(char in);
char *
generate_chars(size_t, char, char*);
char *
replace_char(char w, char r, char *string);
char *
string_replace(char *input, char *match, char *with, char *output,
    size_t max_out);
size_t
str_match(char *input, char *match);

int
reg_match(char *expression, char *match, int flags);

#endif /* STR_H_ */