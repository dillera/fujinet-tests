#ifndef JSON_H
#define JSON_H

#include <fujinet-fuji.h> // for uint8_t
#include <fujinet-err.h>

extern FN_ERR json_open(const char *path);
extern FN_ERR json_close();

// Returns length of data read or negative FN_ERR on error
extern int json_query(const char *query, void *buffer);

#endif /* JSON_H */
