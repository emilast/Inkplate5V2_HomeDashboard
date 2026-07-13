#pragma once
#include <Inkplate.h>
#include "constants.h"
#include "utils.h"

void initializeDisplay(Inkplate &display, const GFXfont &font);
void clearAndPrepareDisplay(Inkplate &display);
void drawHeader(Inkplate &display, const GFXfont &font, String text, int xpos, int ypos);
int drawStrikethrough(Inkplate &display, const GFXfont &font, char *text, int xpos, int ypos);
int drawRightString(Inkplate &display, const GFXfont &font, const char *buf, int x, int y, bool strikethrough);
void drawInvertedString(Inkplate &display, const GFXfont &font, const char *buf, int x, int y, int width);
int drawWrappedText(Inkplate &display, const GFXfont &font, const String &text, int16_t x, int16_t y, int16_t maxWidth);
