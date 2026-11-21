
#pragma once
#include "raylib.h"
#include <stdlib.h>
#include "util.h"
#include "settext.h"

typedef struct {
    Rectangle rectangle;
    int size;
    Color color;
    char *text;
    float vertical_spacing;
    float horizontal_spacing;
    Font* font;
} UIText;

UIText *UICreateText(Rectangle rect, int fontSize, Color color, const char *text, float vertical_spacing, float horizontal_spacing, Font* font) {
    UIText *uitext = malloc(sizeof(UIText));
    uitext->rectangle = rect;
    uitext->size = fontSize;
    uitext->color = color;
    uitext->text = strdup(text);
    uitext->vertical_spacing = vertical_spacing;
    uitext->horizontal_spacing = horizontal_spacing;
    uitext->font = font;
    return uitext;
}

void UIDrawText(UIText *text) {
    DrawText(text->text, text->rectangle.x, text->rectangle.y, text->size, text->color);
}

int UIGetCodepointIncIndex(const char* text, int* index) {
    int codepointByteCount = 0;
    int codepoint = GetCodepoint(&text[*index], &codepointByteCount);
    if (codepoint == 0x3f) codepointByteCount = 1;
    *index += codepointByteCount;
    
    return codepoint;
}

void UIConvertTextToSetText(const UIText* text, SetText* set_text) {
    // https://www.raylib.com/examples/text/loader.html?name=text_rectangle_bounds
    ClearSetTextLines(set_text);
    set_text->horizontal_spacing = text->horizontal_spacing;
    set_text->vertical_spacing = text->vertical_spacing;
    set_text->size = text->size;
    set_text->rectangle = text->rectangle;

    int length = TextLength(text->text);  // Total length in bytes of the text, scanned by codepoints in loop
    float max_x = text->rectangle.width;
    float current_x = 0;
    Font* current_font = text->font;
    Color current_color = text->color;
    int size = text->size;

    SetTextLine* current_line = AddSetTextLine(set_text, 0);

    

    for (int i = 0; i < length; )
    {
        // Get next codepoint from byte string and glyph index in font
        int codepoint_index = i;
        int codepoint = UIGetCodepointIncIndex(text->text, &i);
        float glyphWidth = GetCharacterWidth(*current_font, codepoint, size);

        // if gone too far -> add new line
        if (current_x + glyphWidth > max_x || codepoint == '\n') {
            // add a new line
            current_line = AddSetTextLine(set_text, i);
            current_x = 0;
        }

        if (codepoint == '\n') continue;

        // add codepoint
        AddSetCharacter(current_line, current_font, codepoint, current_color, codepoint_index);

        // increment width
        current_x += glyphWidth + text->horizontal_spacing;
    }
}

void UIDrawBoundedText(UIText *text, Font font, bool wordWrap, float spacing) {
    // https://www.raylib.com/examples/text/loader.html?name=text_rectangle_bounds

    int length = TextLength(text->text);  // Total length in bytes of the text, scanned by codepoints in loop

    float textOffsetY = 0;          // Offset between lines (on line break '\n')
    float textOffsetX = 0.0f;       // Offset X to next character to draw

    float scaleFactor = text->size/(float)font.baseSize;     // Character rectangle scaling factor

    // Word/character wrapping mechanism variables
    enum { MEASURE_STATE = 0, DRAW_STATE = 1 };
    int state = wordWrap? MEASURE_STATE : DRAW_STATE;

    int startLine = -1;         // Index where to begin drawing (where a line begins)
    int endLine = -1;           // Index where to stop drawing (where a line ends)
    int lastk = -1;             // Holds last value of the character position

    for (int i = 0, k = 0; i < length; i++, k++)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepoint(&text->text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol moving one byte
        if (codepoint == 0x3f) codepointByteCount = 1;
        i += (codepointByteCount - 1);

        float glyphWidth = 0;
        if (codepoint != '\n')
        {
            glyphWidth = (font.glyphs[index].advanceX == 0) ? font.recs[index].width*scaleFactor : font.glyphs[index].advanceX*scaleFactor;

            if (i + 1 < length) glyphWidth = glyphWidth + spacing;
        }

        // NOTE: When wordWrap is ON we first measure how much of the text we can draw before going outside of the rec container
        // We store this info in startLine and endLine, then we change states, draw the text between those two variables
        // and change states again and again recursively until the end of the text (or until we get outside of the container)
        // When wordWrap is OFF we don't need the measure state so we go to the drawing state immediately
        // and begin drawing on the next line before we can get outside the container
        if (state == MEASURE_STATE)
        {
            // TODO: There are multiple types of spaces in UNICODE, maybe it's a good idea to add support for more
            // Ref: http://jkorpela.fi/chars/spaces.html
            if ((codepoint == ' ') || (codepoint == '\t') || (codepoint == '\n')) endLine = i;

            if ((textOffsetX + glyphWidth) > text->rectangle.width)
            {
                endLine = (endLine < 1)? i : endLine;
                if (i == endLine) endLine -= codepointByteCount;
                if ((startLine + codepointByteCount) == endLine) endLine = (i - codepointByteCount);

                state = !state;
            }
            else if ((i + 1) == length)
            {
                endLine = i;
                state = !state;
            }
            else if (codepoint == '\n') state = !state;

            if (state == DRAW_STATE)
            {
                textOffsetX = 0;
                i = startLine;
                glyphWidth = 0;

                // Save character position when we switch states
                int tmp = lastk;
                lastk = k - 1;
                k = tmp;
            }
        }
        else
        {
            if (codepoint == '\n')
            {
                if (!wordWrap)
                {
                    textOffsetY += (font.baseSize + (float)font.baseSize/2)*scaleFactor;
                    textOffsetX = 0;
                }
            }
            else
            {
                if (!wordWrap && ((textOffsetX + glyphWidth) > text->rectangle.width))
                {
                    textOffsetY += (font.baseSize + (float)font.baseSize/2)*scaleFactor;
                    textOffsetX = 0;
                }

                // When text overflows rectangle height limit, just stop drawing
                if ((textOffsetY + font.baseSize*scaleFactor) > text->rectangle.height) break;

                // Draw current character glyph
                if ((codepoint != ' ') && (codepoint != '\t'))
                {
                    DrawTextCodepoint(font, codepoint, (Vector2){ text->rectangle.x + textOffsetX, text->rectangle.y + textOffsetY }, text->size, text->color);
                }
            }

            if (wordWrap && (i == endLine))
            {
                textOffsetY += (font.baseSize + (float)font.baseSize/2)*scaleFactor;
                textOffsetX = 0;
                startLine = endLine;
                endLine = -1;
                glyphWidth = 0;
                k = lastk;

                state = !state;
            }
        }

        if ((textOffsetX != 0) || (codepoint != ' ')) textOffsetX += glyphWidth;  // avoid leading spaces
    }
}

int UIGetTextWidth(UIText *text) {
    return MeasureText(text->text, text->size);
}

int UIGetTextWidthWithLength(UIText *text, size_t length) {
    if (length == 0) return 0;
    char displaced_char = text->text[length];
    text->text[length] = '\0';
    int width = MeasureText(text->text, text->size);
    text->text[length] = displaced_char;
    return width;
}

size_t UIGetCharClosestToPosition(UIText *text, Vector2 position) {
    if (strlen(text->text) == 0) return 0;
    if (position.x <= text->rectangle.x) return 0; // first char
    if (position.x >= text->rectangle.x + UIGetTextWidth(text)) return strlen(text->text); // last char
    
    // otherwise binary search
    size_t left = 0;
    size_t right = strlen(text->text);
    while (true) {
        // if in between a single character -> see which side it is closer to
        if (right - left == 1) {
            int left_length = UIGetTextWidthWithLength(text, left);
            int right_length = UIGetTextWidthWithLength(text, right);
            int average_length = left_length + (right_length - left_length) / 2;
            if (position.x < text->rectangle.x + average_length) return left;
            return right;
        }
        // find middle, and make search smaller
        size_t middle = left + (right - left) / 2;
        int middle_length = UIGetTextWidthWithLength(text, middle);
        if (position.x == text->rectangle.x + middle_length) return middle;
        if (position.x > text->rectangle.x + middle_length) left = middle;
        if (position.x < text->rectangle.x + middle_length) right = middle;
    }
}

