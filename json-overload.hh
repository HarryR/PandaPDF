#pragma once
#ifndef JSON_OVERLOAD_HH
#define JSON_OVERLOAD_HH

#pragma interface

#include "json.h"

int json_print(json_printer *printer, int value);

int json_print(json_printer *printer, off_t value);

int json_print_key(json_printer *printer, int k);

int json_print(json_printer *printer, unsigned int value);

int json_print(json_printer *printer, float value);

int json_print(json_printer *printer, double value);

int json_print(json_printer *printer, const char *value);

int json_print_key(json_printer *printer, const char *key);

int json_print_key(json_printer *printer, const char *key, int value);

int json_print_key(json_printer *printer, const char *key, unsigned int value);

int json_print_key(json_printer *printer, const char *key, float value);

int json_print_key(json_printer *printer, const char *key, off_t value);

int json_print_key(json_printer *printer, const char *key, double value);

int json_print_key(json_printer *printer, const char *key, const char *value);

#endif
