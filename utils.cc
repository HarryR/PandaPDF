// Copyright (c) 2009-2013 PixelMags Inc. All Rights Reserved

#pragma implementation "utils.hh"
#include "utils.hh"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>

char*
escapeshellarg(char* str) {
    char *esc = ::strdup(str);
    int i = 0;
    int len = ::strlen(str) + 1;

    while( *str ) {
        if( *str == '\'' || *str == '\\' ) {
            esc[i++] = '\\';
            len += 1;
            esc = (char*)::realloc(esc, len);
        }
        esc[i++] = *str++;
    }
    esc[i] = 0;
    return esc;
}

bool
file_exists( const char *path ) {
	struct stat buf;
	return ::stat(path, &buf) == 0;
}

bool
is_dir( const char* path ) {
	struct stat buf;
	if( ::stat(path,&buf) != 0 ) {
		return false;
	}
	return (buf.st_mode & S_IFDIR) != 0;
}

void scaleWidthHeight( int width, int height,
                         int max_width, int max_height,
                         int *out_width, int *out_height )
{
	if( width > height ) {
		*out_width = max_width;
		*out_height = height * ((double)max_width/(double)width);
	}
	else {
		*out_height = max_height;
		*out_width = width * ((double)max_height/(double)height);
	}
}
