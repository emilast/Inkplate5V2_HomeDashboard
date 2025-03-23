#pragma once
#include <Inkplate.h>
#include "constants.h"
#include "utils.h"

void initializeDisplay(Inkplate &display, const FontCollection &fonts);
void clearAndPrepareDisplay(Inkplate &display);
void drawHeader(Inkplate &display, const FontCollection &fonts, String text, int xpos, int ypos);
int drawStrikethrough(Inkplate &display, const FontCollection &fonts, char *text, int xpos, int ypos);
int drawRightString(Inkplate &display, const FontCollection &fonts, const char *buf, int x, int y, bool strikethrough);
void drawInvertedString(Inkplate &display, const FontCollection &fonts, const char *buf, int x, int y, int width);
int drawWrappedText(Inkplate &display, const FontCollection &fonts, const String &text, int16_t x, int16_t y, int16_t maxWidth);
