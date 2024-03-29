#ifndef SIK_SKORELOWANE_SERWERY_INPUT_PARSING_H
#define SIK_SKORELOWANE_SERWERY_INPUT_PARSING_H

#include "exceptions.hpp"
#include <cstdio>
#include <string>

void safe_fread_bytes(FILE *f, char *buffer, size_t bytes);
char safe_fgetc(FILE *f);
std::string readline_until_delim(FILE *f, char delim);
bool parse_headerline(FILE *stream, std::string &fieldname, std::string &fieldvalue);

#endif // SIK_SKORELOWANE_SERWERY_INPUT_PARSING_H
