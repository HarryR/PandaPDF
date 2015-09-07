#pragma once
#ifndef PDFINFO_HH
#define PDFINFO_HH

#pragma interface

bool get_pdf_page_count( const char *pdf_file, const char *upw, const char *opw, int *pages );

#endif
