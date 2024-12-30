#include "display.h"
#include "store.h"

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
int lineCount = 0;

bool displaySetup()
{
	Wire.begin(21, 22);
	if (!u8g2.begin())
		return false; // an error occurred, return false

	return true; // no error, return true
}

bool displayLoop()
{
	u8g2.sendBuffer();	// update display
	u8g2.clearBuffer(); 
	// u8g2.setCursor(0, 8); // set cursor to first line
	lineCount = 0;
	return true; // reserved for error handling
}

void nextLine()
{
	if (lineCount == 6)
	{
		u8g2.clearBuffer(); // clear screen
		lineCount = 0;
	}
	else
	{
		lineCount++;
	}
	u8g2.setCursor(0, lineCount == 0 ? 8 : lineCount * 9);
}

void printText(String text)
{
	u8g2.print(text);
	nextLine();
}
void printText(double text)
{
	u8g2.print(text);
	nextLine();
}
void printCenter(const char text[], int y)
{
	u8g2.drawStr(64 - (u8g2.getStrWidth(text) / 2), y, text);
};

void printDebug(const char text[])
{
	nextLine();
	u8g2.setFont(u8g2_font_4x6_mf);
	u8g2.print(text);
}