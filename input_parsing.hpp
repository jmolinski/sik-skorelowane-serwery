#ifndef SIK_SKORELOWANE_SERWERY_INPUT_PARSING_H
#define SIK_SKORELOWANE_SERWERY_INPUT_PARSING_H

#include <exception>
#include <cstdio>
#include <string>

class malformed_request_exception : public std::exception {};
class client_closed_connection : public std::exception {};
class io_function_error : public std::exception {};

void safe_fread_bytes(FILE *f, char *buffer, size_t bytes);
char safe_fgetc(FILE *f);
std::string read_until_crlf(FILE *f);

#endif // SIK_SKORELOWANE_SERWERY_INPUT_PARSING_H
