
#pragma once
#include "raylib.h"
#include "util.h"

typedef struct {
    Font* font;
    int codepoint;
    Color color;
    int index;
} SetCharacter;

typedef struct {
    SetCharacter* characters;
    size_t num_characters;
    int start_index;
} SetTextLine;

typedef struct {
    SetTextLine* lines;
    size_t num_lines;
    Rectangle rectangle;
    float size;
    float vertical_spacing;
    float horizontal_spacing;
} SetText;

SetText* CreateSetText(Rectangle rectangle, float size, float vertical_spacing, float horizontal_spacing) {
    SetText* value = alloc(SetText);
    value->rectangle = rectangle;
    value->size = size;
    value->vertical_spacing = vertical_spacing;
    value->horizontal_spacing = horizontal_spacing;
    value->num_lines = 0;
    value->lines = NULL;
    return value;
}

SetTextLine* AddSetTextLine(SetText* text, int start_index) {
    text->num_lines++;
    text->lines = realloc_array(SetTextLine, text->lines, text->num_lines);
    text->lines[text->num_lines - 1].num_characters = 0;
    text->lines[text->num_lines - 1].characters = NULL;
    text->lines[text->num_lines - 1].start_index = start_index;
    return &text->lines[text->num_lines - 1];
}

SetCharacter* AddSetCharacter(SetTextLine* line, Font* font, int codepoint, Color color, int index) {
    line->num_characters++;
    line->characters = realloc_array(SetCharacter, line->characters, line->num_characters);
    line->characters[line->num_characters - 1].font = font;
    line->characters[line->num_characters - 1].codepoint = codepoint;
    line->characters[line->num_characters - 1].color = color;
    line->characters[line->num_characters - 1].index = index;
    return &line->characters[line->num_characters - 1];
}

SetTextLine* ClearSetTextLines(SetText* text) {
    for (int i = 0; i < text->num_lines; i++) {
        free(text->lines[i].characters);
    }
    text->num_lines = 0;
}

void FreeSetText(SetText* text) {
    ClearSetTextLines(text);
    free(text->lines);
    free(text);
}

float GetHorizontalSpacing(const SetText* text) {
    return text->horizontal_spacing;
}

float GetVerticalSpacing(const SetText* text) {
    return text->size * text->vertical_spacing;
}

float GetCharacterWidth(Font font, int codepoint, float size) {
    int index = GetGlyphIndex(font, codepoint);

    if (codepoint == '\n') {
        return 0;
    }
    else {
        float scaleFactor = size / (float) font.baseSize;
        return (font.glyphs[index].advanceX == 0) ?
                font.recs[index].width*scaleFactor :
                font.glyphs[index].advanceX*scaleFactor;
    }
}

float DrawSetCharacter(const SetCharacter* character, Vector2 position, float size) {
    Font font = *(character->font);
    int codepoint = character->codepoint;

    // Draw current character glyph
    if ((codepoint != ' ') && (codepoint != '\t'))
    {
        DrawTextCodepoint(font, codepoint, (Vector2){position.x, position.y}, size, character->color);
    }
    return GetCharacterWidth(font, character->codepoint, size);
}

void DrawSetTextLine(const SetTextLine* line, Vector2 position, float size, float spacing) {
    for(int i = 0; i < line->num_characters; i++) {
        SetCharacter* character = &line->characters[i];
        float width = DrawSetCharacter(character, position, size);
        position.x += width;
        position.x += spacing;
    }
}

void DrawSetText(const SetText* text) {
    // https://www.raylib.com/examples/text/loader.html?name=text_rectangle_bounds
    float size = text->size;
    Vector2 position = (Vector2){.x=text->rectangle.x,.y=text->rectangle.y};

    for (int i = 0; i < text->num_lines; i++) {
        SetTextLine* line = &text->lines[i];
        DrawSetTextLine(line, position, size, GetHorizontalSpacing(text));
        position.y += GetVerticalSpacing(text);
    }
}

Vector2 SetTextGetPositionOfChar(const SetText* text, int stringIndex) {
    float x = text->rectangle.x;
    float y = text->rectangle.y;
    if (text->num_lines == 0) return (Vector2){.x=x,.y=y};

    // first find which line it belongs to
    SetTextLine* line = NULL;
    for (int i = 0; i < text->num_lines; i++) {
        line = &text->lines[i];
        if (i == text->num_lines - 1
            || (stringIndex < text->lines[i+1].start_index)) break;
        // increment y for each line
        y += GetVerticalSpacing(text);
    }

    // line found -> search the characters for the correct string index
    if (line->num_characters == 0) return (Vector2){.x=x,.y=y};
    for (int i = 0; i < line->num_characters; i++) {
        SetCharacter* character = &line->characters[i];
        if (i < line->num_characters - 1
            && (stringIndex < line->characters[i+1].index)) break;
        if (i == line->num_characters - 1) {
            // if it is the last character -> only add width if beyond the codepoint
            int codepoint_size;
            (void)CodepointToUTF8(character->codepoint, &codepoint_size);
            if (stringIndex < line->characters[i].index + codepoint_size) break;
        }
        x += GetHorizontalSpacing(text) + GetCharacterWidth(*(character->font), character->codepoint, text->size);
    }

    // if we reach here, the index is beyond the line or found, just return the x and y
    return (Vector2){.x=x,.y=y};
}

int SetTextGetCharClosestToPosition(const SetText* text, Vector2 position) {
    if (text->num_lines == 0) return -1;

    // get the closest line number
    int line_number;
    if (position.y <= text->rectangle.y) {
        line_number = 0;
    } else {
        line_number = (int)((position.y - text->rectangle.y) / GetVerticalSpacing(text));
        if (line_number >= text->num_lines) line_number = text->num_lines - 1;
    }
    SetTextLine* line = &text->lines[line_number];
    
    // now search through the line for the position closest
    if (position.x <= text->rectangle.x) {
        // it is to the left of the line -> return the first character of line
        return line->start_index;
    }

    float current_x = text->rectangle.x;
    float horizontal_spacing = GetHorizontalSpacing(text);
    int i;
    for (i = 0; i < line->num_characters; i++) {
        SetCharacter* character = &line->characters[i];
        float width = GetCharacterWidth(*(character->font), character->codepoint, text->size);
        
        // check if the cursor falls on this character
        if (position.x < current_x + width + horizontal_spacing) {
            // if it falls in first half of character, return index
            if (position.x < current_x + width / 2) {
                return character->index;
            } else {
                // otherwise, return index of next character
                int codepoint_size;
                (void)CodepointToUTF8(character->codepoint, &codepoint_size);
                return character->index + codepoint_size;
            }
        } else {
            // increment to next character and continue
            current_x += width + horizontal_spacing;
        }
    }
    // not found -> to right of string -> return i
    return line->start_index + i;
}
