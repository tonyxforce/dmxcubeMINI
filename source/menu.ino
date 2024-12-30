#include "menu.h"
#include "store.h"
#include "source.h"
#include "display.h"

void menuSetup() {
};

#define DISPLAYH 64
#define DISPLAYW 128

Menu currentMenu = MENU_MAIN;
Menu previousMenu = MENU_DEFAULT;

void switchMenuTo(Menu newMenu);

int scrollOffset = 0;
int optionsCount = 0;

void renderMenu()
{
	u8g2.setFont(u8g2_font_6x13_tf);
	u8g2.setDrawColor(1);
	u8g2.drawLine(0, 9, DISPLAYW, 9);

	switch (currentMenu)
	{
	case MENU_MAIN:

		printCenter("Main menu", 9);
		u8g2.setFontMode(1);
		u8g2.setFont(u8g2_font_6x13_mf);

		u8g2.setDrawColor(1);
		if (encoderPos >= 0)
			u8g2.drawBox(0, (9 * encoderPos) + 10 + (scrollOffset * 9), DISPLAYW, 9);

		optionsCount = mainMenuLen;
		for (int i = 0; i < optionsCount; i++)
		{
			int height = ((i + 1) * 9) + 10 + (scrollOffset * 9);

			u8g2.setDrawColor(i != encoderPos);

			if (height > 11)
				printCenter(gameMenuOptionsStrings[i].c_str(), height);
		}
		u8g2.setDrawColor(1);
		break;

	case MENU_SETTINGS:

		printCenter("Settings", 9);
		u8g2.setFontMode(1);
		u8g2.setFont(u8g2_font_6x13_mf);

		u8g2.setDrawColor(1);
		if (encoderPos >= 0)
			u8g2.drawBox(0, (9 * encoderPos) + 10 + (scrollOffset * 9), DISPLAYW, 9);

		optionsCount = settingsMenuLen;
		for (int i = 0; i < optionsCount; i++)
		{
			int height = ((i + 1) * 9) + 10 + (scrollOffset * 9);
			if (height < 11)
				continue;

			bool baseColor = i != encoderPos;

			u8g2.setDrawColor(baseColor);
			printCenter(settingsMenuOptionsStrings[i].c_str(), height);
			if (settingsMenuOptionsTypes[i] == menutype_CHECKBOX)
			{
				u8g2.setDrawColor(baseColor);
				u8g2.drawFrame(110, height - 8, 7, 7);
				// if (optionsConditions[i] != nullptr && optionsConditions[i](deviceSettings))
				//	u8g2.drawBox(110, height - 8, 7, 7);
			}
		}
		u8g2.setDrawColor(1); // Reset draw color
		break;
	};
};

bool menuLoop()
{

	nextLine();
	u8g2.setFont(u8g2_font_4x6_mf);
	u8g2.print(encoderPos);
	nextLine();
	u8g2.print(scrollOffset);
	nextLine();

	encoderPos = constrain(encoderPos, 0, optionsCount - 1);

	if (scrollOffset * -1 - 1 == encoderPos && optionsCount > 6)
	{
		scrollOffset++;
	}

	if (scrollOffset * -1 + 6 == encoderPos && encoderPos > 5 && optionsCount > 6)
	{
		scrollOffset--;
	}

	// if (currentMenu != MENU_SETTINGS)
	//	scrollOffset = 0;

	/*if (LEFTPressed() || RIGHTPressed())
	{
		switch (currentMenu)
		{
		case MENU_SETTINGS:
			switch (encoderPos)
			{
			case settingsmenuoption_BRIGHTNESS:
				if (LEFTPressed())
					deviceSettings.brightness -= 5;
				if (RIGHTPressed())
					deviceSettings.brightness += 5;

				deviceSettings.brightness = constrain(deviceSettings.brightness, 0, 255);
				FastLED.setBrightness(deviceSettings.brightness);
				break;
			}
			break;
		}
	}

	if (CENTERPressed())
	{

		while (CENTERPressed())
			processLoop();

		switch (currentMenu)
		{
		case MENU_MAIN:
			switch (encoderPos)
			{
			case gamemenuoption_PONG:
				runningGame = GAME_PONG;
				break;
			case gamemenuoption_SNAKE:
				runningGame = GAME_SNAKE;
				break;
#if !RELEASE
			case gamemenuoption_TEST:
				runningGame = GAME_TEST;
				break;
#endif
			case gamemenuoption_BREAKOUT:
				runningGame = GAME_BREAKOUT;
				break;
			case gamemenuoption_SETTINGS:
				switchMenuTo(MENU_SETTINGS);
				break;
			case gamemenuoption_RESTART:
				u8g2.clearDisplay();
				u8g2.sendBuffer();
				fillLeds(CRGB::Black);
				ledsLoop();
				ESP.restart();
				break;
			}
			encoderPos = 0;

			break;
		case MENU_SETTINGS:
			switch (encoderPos)
			{
			case settingsmenuoption_SOUNDS:
				deviceSettings.soundEnabled = !deviceSettings.soundEnabled;
				if (!deviceSettings.soundEnabled)
					noTone(25);
				break;
			case settingsmenuoption_MENUSOUNDS:
				deviceSettings.menuSounds = !deviceSettings.menuSounds;
				break;
			case settingsmenuoption_DEBUGMODE:
				deviceSettings.debugMode = !deviceSettings.debugMode;
				break;
			case settingsmenuoption_ONEHANDED:
				deviceSettings.oneHanded = !deviceSettings.oneHanded;
				break;
			case settingsmenuoption_SHOWFPS:
				deviceSettings.showFps = !deviceSettings.showFps;
				break;

			case settingsmenuoption_BACK:
				saveSettings();
				goBack();
				break;
			}
			if (settingsMenuOptionsTypes[encoderPos] == menutype_CHECKBOX)
			{
				beep(800, 50, BEEPTYPE_MENU);
			}
			if (optionsConditions[encoderPos] != nullptr && optionsConditions[encoderPos](deviceSettings))
				beep(1000, 50, BEEPTYPE_MENU);
			break;
		};
		beep(200, 50, BEEPTYPE_MENU);
	}
	*/

	renderMenu();

	return true;
}

void switchMenuTo(Menu newMenu)
{
	encoderPos = 0;
	scrollOffset = 0;
	previousMenu = currentMenu;
	currentMenu = newMenu;
};