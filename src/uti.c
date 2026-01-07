#include "uti.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"
#include "errno.h"
#include "assert.h"


#ifdef _WIN32
#include "dirent_win32ports.h"
#else
#include "dirent.h"
#endif

bool read_entire_file(const char *path, char** content, size_t* out_size) {
    char* mem_block = NULL;
    FILE *file = fopen(path, "rb");
    if (!file) goto error;
    if (fseek(file, 0, SEEK_END) < 0) goto error;

    long long length = 0;
#ifndef _WIN32
    length = ftell(file);
#else
    length = _ftelli64(file);
#endif
    
    if (length < 0) goto error;
    if (fseek(file, 0, SEEK_SET) < 0) goto error;

    mem_block = (char*)malloc(length+1);
    if (mem_block == NULL) {
        printf("ERROR: malloc failed in %s:%d\n", __FILE__, __LINE__);
        goto error;
    }

    mem_block[length] = '\0'; // for good measure

    fread(mem_block, length, 1, file);
    if (ferror(file)) goto error;

    fclose(file);
    // if all is fine, set it for output
    *content = mem_block;
    *out_size = length;
    return true;

error:
    printf("ERROR: Could not read file %s: %s\n", path, strerror(errno));
    if (file) fclose(file);
    if (mem_block) free(mem_block);
    return false;
}


bool read_entire_dir(const char *parent_dir, char*** children, size_t *children_count) {
    DIR *dir = NULL;
    struct dirent *ent = NULL;

#define _ENTRY_MEMORY_BLOCK_BASE 4092
#define _ENTRY_COUNT_BASE_MULT 2

    // stores pointers to mem_block string starts
    char** c_array = NULL;
    size_t c_size = 0;
    size_t dir_count = 0;

    // stores literal characters
    char* mem_block = NULL;
    size_t size = 0;
    size_t filled_so_far = 0;

    dir = opendir(parent_dir);
    if (dir == NULL) {
        #ifdef _WIN32
        printf("ERROR: Could not open directory %s: %s\n", parent_dir, "TODO: win error msg"/*nob_win32_error_message(GetLastError())*/);
        #else
        printf("ERROR: Could not open directory %s: %s\n", parent_dir, strerror(errno));
        #endif // _WIN32
        goto error;
    }


    errno = 0;
    ent = readdir(dir);

    while (ent != NULL) {
        size_t dir_length = strlen(ent->d_name);
        if (dir_length == 0) {
            ent = readdir(dir);
            continue;
        }

        // realloc madness
        while (filled_so_far + dir_length + 1 > size) {
            size = size + _ENTRY_MEMORY_BLOCK_BASE;
            char* new_mem_block = realloc(mem_block, size);
            if (new_mem_block == NULL) {
                printf("ERROR: realloc failed in %s:%d\n", __FILE__, __LINE__);
                goto error;
            }
            mem_block = new_mem_block;
        }
        strcpy(&mem_block[filled_so_far], ent->d_name);

        // realloc madness
        if (dir_count + 1 > c_size) {
            if (c_size == 0) c_size = _ENTRY_COUNT_BASE_MULT;
            else c_size *= _ENTRY_COUNT_BASE_MULT;
            char** new_c_array = realloc(c_array, c_size*sizeof(char*));
            if (new_c_array == NULL) {
                printf("ERROR: realloc failed in %s:%d\n", __FILE__, __LINE__);
                goto error;
            }
            c_array = new_c_array;
        }

        c_array[dir_count] = &mem_block[filled_so_far];

        dir_count++;
        filled_so_far += dir_length + 1;

        ent = readdir(dir);
    }

    if (errno != 0) {
        #ifdef _WIN32
        printf("ERROR: Could not read directory %s: %s\n", parent_dir, "TODO: win error msg"/*nob_win32_error_message(GetLastError())*/);
        #else
        printf("ERROR: Could not read directory %s: %s\n", parent_dir, strerror(errno));
        #endif // _WIN32
        goto error;
    }

    // set the output if all is well
    *children_count = dir_count;
    *children = c_array;

    closedir(dir);
    return true;

error:
    if (dir) closedir(dir);
    if (c_array) free(c_array);
    if (mem_block) free(mem_block);
    return false;
}

// Adopted from nob.h
// TEMP buffer because we need copy strings to null terminated. Raylib MeaserTexteEx, etc.. uses only null terminated
static char temp_buffer_internal[TEMP_BUFFER_CAP_INTERNAL] = {0};
static size_t temp_size_internal = 0;
void uti_temp_reset(void){ temp_size_internal = 0; }
void *uti_temp_alloc(size_t requested_size) {
    size_t word_size = sizeof(uintptr_t);
    size_t size = (requested_size + word_size - 1)/word_size*word_size;
    if (temp_size_internal + size > TEMP_BUFFER_CAP_INTERNAL) return NULL;
    void *result = &temp_buffer_internal[temp_size_internal];
    temp_size_internal += size;
    return result;
}
char *uti_temp_strndup(const char *s, size_t n) {
    char *r = uti_temp_alloc(n + 1);
    assert(r != NULL && "Extend the size of the temporary allocator in uti.c");
    memcpy(r, s, n);
    r[n] = '\0';
    return r;
}

// end temp allocator
