#ifndef UTILS_H
#define UTILS_H

#pragma interface

char *escapeshellarg(char* str);
bool is_dir( const char* path );
bool file_exists( const char *path );

void scaleWidthHeight( int width, int height,
                       int max_width, int max_height,
                       int *out_width, int *out_height );

#endif /* UTILS_H */
