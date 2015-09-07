#pragma implementation "json-overload.hh"
#include "json-overload.hh"
#include "asprintf.h"

// Copyright (c) 2010-2013 PixelMags Inc. All Rights Reserved

int
json_print(json_printer *printer, int value) {
    char *str;
    ::asprintf(&str, "%d", value);
    int ret = json_print_raw(printer, JSON_INT, str, strlen(str));
    ::free(str);
    return ret;
}

int
json_print(json_printer *printer, off_t value) {
    char *str;
    ::asprintf(&str, "%ld", value);
    int ret = json_print_raw(printer, JSON_INT, str, ::strlen(str));
    ::free(str);
    return ret;
}

int json_print_key(json_printer *printer, int k) {
    int ret;
    char *str;
    ::asprintf(&str, "%d", k);
    ret = json_print_raw(printer, JSON_KEY, str, ::strlen(str));
    ::free(str);
    return ret;
}

int
json_print(json_printer *printer, unsigned int value) {
    char *str;
    ::asprintf(&str, "%u", value);
    int ret = json_print_raw(printer, JSON_INT, str, ::strlen(str));
    ::free(str);
    return ret;
}

int
json_print(json_printer *printer, float value) {
    char *str;
    ::asprintf(&str, "%0.5f", value);
    int ret = json_print_raw(printer, JSON_FLOAT, str, ::strlen(str));
    ::free(str);
    return ret;
}

int
json_print(json_printer *printer, double value) {
    char *str;
    ::asprintf(&str, "%0.5f", value);
    int ret = json_print_raw(printer, JSON_FLOAT, str, ::strlen(str));
    ::free(str);
    return ret;
}

int
json_print(json_printer *printer, const char *value) {
    return json_print_raw(printer, JSON_STRING, value, ::strlen(value));
}

int json_print_key(json_printer *printer, const char *key) {
    return json_print_raw(printer, JSON_KEY, key, ::strlen(key));
}

int json_print_key(json_printer *printer, const char *key, int value) {
    json_print_raw(printer, JSON_KEY, key, ::strlen(key));
    return json_print(printer, value);
}

int json_print_key(json_printer *printer, const char *key, unsigned int value) {
    json_print_raw(printer, JSON_KEY, key, ::strlen(key));
    return json_print(printer, value);
}

int json_print_key(json_printer *printer, const char *key, float value) {
    json_print_raw(printer, JSON_KEY, key, ::strlen(key));
    return json_print(printer, value);
}

int json_print_key(json_printer *printer, const char *key, off_t value) {
    json_print_raw(printer, JSON_KEY, key, ::strlen(key));
    return json_print(printer, value);
}

int json_print_key(json_printer *printer, const char *key, double value) {
    json_print_raw(printer, JSON_KEY, key, ::strlen(key));
    return json_print(printer, value);
}

int json_print_key(json_printer *printer, const char *key, const char *value) {
    json_print_raw(printer, JSON_KEY, key, ::strlen(key));
    return json_print(printer, value);
}
