#pragma once

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#define PANIC_NUMBER -67
#define PANIC(message) { panic(message); return -67; }
#define alloc(type) ((type*)(malloc(sizeof(type))))
#define alloc_array(type, length) ((type*)(malloc(sizeof(type) * length)))
#define realloc_array(type, ptr, length) ((type*)realloc\
        (ptr, sizeof(type) * length))
#define recalloc_array(type, ptr, original_length, new_length) ((type*)(recalloc)\
        (ptr, sizeof(type) * original_length, sizeof(type) * new_length))


void* (recalloc)(void* ptr, size_t original_length, size_t new_length) {
    ptr = realloc(ptr, new_length);
    if (original_length < new_length) {
        memset(ptr + original_length, 0, new_length - original_length);
    }
    return ptr;
}

char *strdup(const char *c)
{
    if(c == NULL) {
        char* empty_str = malloc(1);
        empty_str[0] = '\0';
        return empty_str;
    }
    
    char *dup = malloc(strlen(c) + 1);

    if (dup != NULL)
       strcpy(dup, c);

    return dup;
}

void panic(const char *message) {
    FILE *file = fopen("panic.txt", "w");
    if (file == NULL) {
        return;
    }

    fprintf(file, "%s\n", message);

    fclose(file);

    return;
}

uint64_t ntohll(uint64_t value) {
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        return __builtin_bswap64(value);
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        return value;
    #else
        #error "Unsupported endianness"
    #endif
}

uint64_t htonll(uint64_t value) {
    return ntohll(value);
}

long get_file_size(FILE* file) {
    // get current position
    long cur_pos = ftell(file);
    if (cur_pos < 0) return -1;

    // move to end and get position
    if (fseek(file, 0, SEEK_END) != 0) return -1;
    long end_pos = ftell(file);
    if (end_pos < 0) return -1;

    // return to start
    if (fseek(file, cur_pos, SEEK_SET) != 0) return -1;

    // return size
    return end_pos;
}

char* string_append(char* string, const char* addition) {
    size_t new_length = strlen(string) + strlen(addition) + 1;
    string = realloc_array(char, string, new_length);
    strcat(string, addition);
    return string;
}

char* string_insert(char* string, size_t index, char c) {
    size_t new_length = strlen(string) + 1 + 1;
    string = realloc_array(char, string, new_length);
    for (size_t i = new_length - 1; i > index; i --) {
        string[i] = string[i - 1];
    }
    string[index] = c;
    return string;
}

char* string_remove(char* string, size_t index) {
    size_t new_length = strlen(string) + 1 - 1;
    
    for (size_t i = index; i < new_length; i ++) {
        string[i] = string[i + 1];
    }
    string = realloc_array(char, string, new_length);
    return string;
}

int vasprintf(char **strp, const char *fmt, va_list ap) {
    if (strp == NULL) {
        return -1;
    }

    // Copy va_list to measure the required string length first
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int len = vsnprintf(NULL, 0, fmt, ap_copy);
    va_end(ap_copy);

    if (len < 0) {
        *strp = NULL;
        return -1;
    }

    // Allocate memory for the string plus the null terminator ('\0')
    char *buf = malloc((size_t)len + 1);
    if (buf == NULL) {
        *strp = NULL;
        return -1;
    }

    // Write the formatted string into the allocated buffer
    int written = vsnprintf(buf, (size_t)len + 1, fmt, ap);
    if (written < 0) {
        free(buf);
        *strp = NULL;
        return -1;
    }

    *strp = buf;
    return written;
}

int asprintf(char **strp, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = vasprintf(strp, fmt, ap);
    va_end(ap);
    return ret;
}