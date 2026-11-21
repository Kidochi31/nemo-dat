#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "util.h"

typedef struct NemoSection{
    uint64_t body_length;
    uint8_t* body;
    struct NemoSection* next_section;
    struct NemoSection* prev_section;
} NemoSection;

typedef struct {
    NemoSection* start;
} NemoSectionList;

void ReallocSection(NemoSection* section, uint64_t new_length) {
    section->body = recalloc_array(uint8_t, section->body, section->body_length, new_length);
    section->body_length = new_length;
}

NemoSection* AddSection(NemoSectionList* list) {
    NemoSection** section_ptr = &list->start;
    NemoSection* prev_section = NULL;
    while(*section_ptr != NULL) {
        prev_section = (*section_ptr)->prev_section;
        section_ptr = &((*section_ptr)->next_section);
    }
    NemoSection* new_section = alloc(NemoSection);
    *section_ptr = new_section;
    new_section->prev_section = prev_section;
    new_section->next_section = NULL;
    new_section->body = NULL;
    new_section->body_length = 0;

    return new_section;
}

// file structure of .nemo file:
// N E M O
// Sections (length (8 bytes) + body)

#define ValidateNemo(buffer) (buffer[0] == 'N' && buffer[1] == 'E' \
                           && buffer[2] == 'M' && buffer[3] == 'O')

int SaveNemoFile(const char* path, NemoSectionList* sections) {
    FILE *file = fopen(path, "wb");
    if (file == NULL) {
        goto error;
    }

    // write N E M O
    uint8_t nemo_buffer[4] = {'N', 'E', 'M', 'O'};
    size_t bytes_written = fwrite(nemo_buffer, 1, sizeof(nemo_buffer), file);
    if (bytes_written < sizeof(nemo_buffer)) goto close_file;

    // write in all the sections
    NemoSection* curr_section = sections->start;
    while(curr_section != NULL) {
        // write the length
        uint64_t section_length = htonll(curr_section->body_length);
        size_t length_bytes_written = fwrite(&section_length, 1, sizeof(section_length), file);
        if (length_bytes_written < sizeof(section_length)) goto close_file;

        // write the data
        section_length = curr_section->body_length;
        size_t data_bytes_written = fwrite(curr_section->body, 1, section_length, file);
        if (data_bytes_written < section_length) goto close_file;

        // go to next section
        curr_section = curr_section->next_section;
    }

    fclose(file);
    return 0;

close_file:
    fclose(file);

error:
    return -1;
}

NemoSectionList* LoadNemoFile(const char* path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        goto error;
    }

    // check first four bytes match N E M O
    uint8_t nemo_buffer[4];
    size_t bytes_read = fread(nemo_buffer, 1, sizeof(nemo_buffer), file);
    if (bytes_read < sizeof(nemo_buffer)) {
        goto close_file;
    }
    if (!ValidateNemo(nemo_buffer)) {
        goto close_file;
    }
    // get file size
    long file_size = get_file_size(file);
    if (file_size < 0) goto close_file;

    // validated, now go through all sections
    NemoSectionList* list = alloc(NemoSectionList);
    list->start = NULL;
    NemoSection* prev_section = NULL;
    while (true) {
        // get length
        uint64_t length;
        size_t length_bytes_read = fread(&length, 1, sizeof(length), file);
        if (length_bytes_read < sizeof(length)) {
            goto free_list;
        }
        length = ntohll(length); // convert to correct endianness

        // ensure there is enough space left in file
        long cur_pos = ftell(file);
        if (cur_pos < 0) goto free_list;
        long bytes_left = file_size - cur_pos;
        if (bytes_left < length) goto free_list;

        // initialise section
        NemoSection* section = alloc(NemoSection);
        section->body = alloc_array(uint8_t, length);
        section->body_length = length;
        section->next_section = NULL;
        section->prev_section = prev_section;
        if (prev_section != NULL) prev_section->next_section = section;
        prev_section = section;
        if (list->start == NULL) list->start = section;

        // load the data from file
        size_t data_bytes_read = fread(section->body, 1, length, file);
        if (data_bytes_read < length) goto free_list;

        // check if we have reached the end of the file
        cur_pos = ftell(file);
        if (cur_pos < 0) goto free_list;
        if (cur_pos == file_size) break;
    }
    fclose(file);
    return list;

free_list:
    NemoSection* section = list->start;
    while (section != NULL) {
        NemoSection* old_section = section;
        section = section->next_section;
        free(old_section->body);
        free(old_section);
    }
    free(list);

close_file:
    fclose(file);

error:
    return NULL;
}