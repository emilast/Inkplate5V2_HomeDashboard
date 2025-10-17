#include "DisplayManager.h"

void initializeDisplay(Inkplate &display, const FontCollection &fonts) {
    display.begin();
    display.setFont(&fonts.normalTextFont);
    display.clearDisplay();
    display.display();
    display.setTextSize(1);
    display.setTextColor(BLACK, WHITE);
    display.setTextWrap(false);
}

void clearAndPrepareDisplay(Inkplate &display) {
    display.clearDisplay();
}

void drawHeader(Inkplate &display, const FontCollection &fonts, String text, int xpos, int ypos) {
    display.setCursor(xpos, ypos);
    display.setFont(&fonts.boldTextFont);
    display.print(text);
    display.setFont(&fonts.normalTextFont);
}

int drawStrikethrough(Inkplate &display, const FontCollection &fonts, char *text, int xpos, int ypos) {
    display.setFont(&fonts.normalTextFont);
    display.setCursor(xpos, ypos);
    display.print(text);
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(text, xpos, ypos, &x1, &y1, &w, &h);
    display.fillRect(xpos, ypos - (h / 2), w, 3, BLACK);
    return w;
}

int drawRightString(Inkplate &display, const FontCollection &fonts, const char *buf, int x, int y, bool strikethrough) {
    display.setFont(&fonts.normalTextFont);
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(buf, x, y, &x1, &y1, &w, &h);
    display.setCursor(x - w, y);
    if (strikethrough) {
        display.fillRect(x - w, y - (h / 3), w, 3, BLACK);
    }
    display.print(buf);
    return w;
}

void drawInvertedString(Inkplate &display, const FontCollection &fonts, const char *buf, int x, int y, int width) {
    display.setFont(&fonts.normalTextFont);
    int16_t x1, y1;
    uint16_t w, h;
    const int margin = 5;
    display.getTextBounds(buf, x, y, &x1, &y1, &w, &h);
    display.fillRoundRect(x, y - h - margin, width, h + 2 * margin, margin, BLACK);
    display.setTextColor(WHITE, BLACK);
    display.setCursor(x + (width / 2) - (w / 2) - 2, y);
    display.print(buf);
    display.setCursor(x + width, y);
    display.setTextColor(BLACK, WHITE);
}

int drawWrappedText(Inkplate &display, const GFXfont &font, const String &text, int16_t x, int16_t y, int16_t maxWidth) {
    display.setFont(&font);
    display.setTextWrap(false);
    int linesDrawn = 0;
    String line = "";
    int16_t cursor_y = y;
    int i = 0;
    while (i < text.length()) {
        int spaceIndex = text.indexOf(' ', i);
        if (spaceIndex == -1)
            spaceIndex = text.length();
        String word = text.substring(i, spaceIndex);
        String testLine = line + (line.length() > 0 ? " " : "") + word;
        int16_t x1, y1;
        uint16_t w, h;
        display.getTextBounds(testLine, 0, 0, &x1, &y1, &w, &h);
        if (w > maxWidth && line.length() > 0) {
            display.setCursor(x, cursor_y);
            display.print(line);
            cursor_y += h;
            line = word;
            linesDrawn++;
        } else {
            line = testLine;
        }
        i = spaceIndex + 1;
    }
    if (line.length() > 0) {
        display.setCursor(x, cursor_y);
        display.print(line);
        linesDrawn++;
    }
    return linesDrawn;
}
