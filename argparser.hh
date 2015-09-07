#pragma once
#ifndef ARGPARSER_HH
#define ARGPARSER_HH

#pragma interface

#include "logger.hh"

enum ArgType {
	ARG_INT,
	ARG_BOOL,
	ARG_STR,
	ARG_FLOAT,
};

struct Arg {
	char *name;
	ArgType type;
	char *value;
	const char *help;
	struct Arg *next;	
};

class ArgParser {
public:
	ArgParser(Logger *log);
	~ArgParser();

	bool ok();
	void help();
	int parse(int argc, char **argv);
	void add(const char *name, ArgType type, const char *help);
	bool has(const char *name);
	struct Arg *get(const char *name);	
	bool set(const char *name, const char *value);

	int getInt(const char *name, int default_value = -1);
	bool getBool(const char *name);
	const char *getString(const char *name, const char *default_value = NULL);
	float getFloat(const char *name, float default_value = 0.0f);

private:
	bool m_ok;
	Arg *m_args;
	Logger *m_log;
};

#endif
