#pragma implementation "argparser.hh"
#include "argparser.hh"

// Copyright (c) 2010-2013 PixelMags Inc. All Rights Reserved

#include <cstring>
#include <cstdlib>
#include <cassert>

ArgParser::ArgParser(Logger *log)
: m_ok(false), m_args(NULL), m_log(log)
{ }

bool
ArgParser::ok() {
	return m_ok;
}

void
ArgParser::help() {
	struct Arg *arg = m_args;
	const char str_float[] = "float";
	const char str_str[] = "str";
	const char str_int[] = "int";

	while( arg ) {
		if( arg->type == ARG_BOOL ) {
			m_log->println(" -%s\t: %s", arg->name, arg->help);
		}
		else {
			const char *arg_type_str = NULL;
			if( arg->type == ARG_FLOAT ) arg_type_str = str_float;
			else if( arg->type == ARG_STR ) arg_type_str = str_str;
			else if( arg->type == ARG_INT ) arg_type_str = str_int;
			else {
				assert( false );
			}
			m_log->println(" -%s <%s>\t: %s", arg->name, arg_type_str, arg->help);
		}
		arg = arg->next;
	}

	m_log->println("");
}

void
ArgParser::add(const char *name, ArgType type, const char *help) {
	struct Arg *arg = (struct Arg*)malloc(sizeof(*arg));
	arg->name = ::strdup(name);
	arg->type = type;
	arg->value = NULL;
	arg->next = m_args;
	arg->help = help;
	m_args = arg;
}

bool
ArgParser::set(const char *name, const char *value) {
	struct Arg *arg = get(name);
	if( ! arg ) {
		m_log->warn("Unknown argument '%s'", name);
		return false;
	}

	if( arg->type == ARG_BOOL ) {
		arg->value = ::strdup(name);
	}
	else {
		arg->value = ::strdup(value);
	}
	return true;
}

int
ArgParser::parse(int argc, char **argv) {
	int i;
	m_ok = false;
	for( i = 1; i < argc; i++ ) {
		if( argv[i][0] != '-' ) return i;
		if( argv[i][1] == 0 ) continue;
		char *argname = &argv[i][1];
		struct Arg *arg = get(argname);
		if( ! arg ) {
			m_log->warn("Unknown argument '%s' as position %d", argname, i);
			return i;
		}

		if( arg->type != ARG_BOOL ) {
			if( (i + 1) >= argc ) {
				m_log->warn("Argument '%s' requires operand, none supplied!", argname);
				return i;
			}

			if( ! set( argname, argv[++i] ) ) {
				return i;
			}
		}
		else {
			if( ! set( argname, argv[i]) ) {
				return i;
			}
		}
	}
	m_ok = true;
	return i;
}

bool
ArgParser::has(const char *name) {
	struct Arg *a = get(name);
	return a != NULL && a->value != NULL;
}

struct Arg *
ArgParser::get(const char *name) {
	struct Arg *a = m_args;
	while( a ) {
		if( ::strcmp(name, a->name) == 0 ) {
			return a;
		}
		a = a->next;
	}
	return NULL;
}

int
ArgParser::getInt(const char *name, int default_value) {
	struct Arg *a = get(name);
	if( ! a || ! a->value ) return default_value;
	return ::atoi(a->value);
}

bool
ArgParser::getBool(const char *name) {
	struct Arg *a = get(name);
	return a && a->value;
}

const char *
ArgParser::getString(const char *name, const char *default_value) {
	struct Arg *a = get(name);
	if( ! a || ! a->value ) return default_value;
	return a->value;
}

float
ArgParser::getFloat(const char *name, float default_value) {
	struct Arg *a = get(name);	
	if( ! a || ! a->value ) return default_value;
	return ::atof(a->value);
}

ArgParser::~ArgParser() {
	struct Arg *a;
	while( m_args ) {
		a = m_args;
		m_args = a->next;
		::free(a->name);
		::free(a);
	}
}