#ifndef MIME_TYPE_H
#define MIME_TYPE_H

const char *find_mime_type(const char *fileext);
bool load_mime_types(const char *filename);

#endif
