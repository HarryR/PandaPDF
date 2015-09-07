#pragma implementation "pdfinfo.hh"
#include "pdfinfo.hh"

#include "poppler/PDFDoc.h"

bool
get_pdf_page_count( const char *pdf_file, const char *upw, const char *opw, int *pages ) {
	GooString *file_name = new GooString(pdf_file);
	GooString *ownerPW = NULL;
    GooString *userPW = NULL;
    PDFDoc *doc;
    bool result = false;

    if (upw) userPW = new GooString(upw);
    if (opw) ownerPW = new GooString(opw);
	doc = new PDFDoc(file_name, ownerPW, userPW);    
	if( doc->isOk() ) {
		result = true;
		*pages = doc->getNumPages();
	}

	delete doc;
	return result;	
}