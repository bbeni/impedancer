#ifndef UTI_H_
#define UTI_H_

#include "stdbool.h"
#include "stddef.h"

// allocate space for file content + \0 terminator and read into it. out size is without \0 terminator.
bool read_entire_file(const char *path, char** content, size_t* out_size);
bool read_entire_dir(const char *parent_dir, char*** children, size_t *children_count);

// Adopted from nob.h:
// TEMP buffer because we need copy strings to null terminated. Raylib MeaserTexteEx, etc.. uses only null terminated
#define TEMP_BUFFER_CAP_INTERNAL 4096*4096
void uti_temp_reset(void);
void *uti_temp_alloc(size_t requested_size);
char *uti_temp_strndup(const char *s, size_t n);
// end temp allocator stuff


#endif //UTI_H_