#ifndef MENU_H
#define MENU_H

#include <Arduino.h>

bool menuLoop();
void menuSetup();

enum MenuType
{
	menutype_GENERAL,
	menutype_CHECKBOX,
	menutype_NUMBER,
};

const int mainMenuLen = 3;

enum GameMenuOption
{
	gamemenuoption_INFO,
	gamemenuoption_SETTINGS,
	gamemenuoption_RESTART,
};

const String gameMenuOptionsStrings[mainMenuLen] = {
		"Hardware info",
		"Settings",
		"Restart"};

const int settingsMenuLen = 1;
enum SettingsMenuOption
{
	settingsmenuoption_BACK,
};

const String settingsMenuOptionsStrings[settingsMenuLen] = {
		"Vissza",
};

const MenuType settingsMenuOptionsTypes[settingsMenuLen] = {
		menutype_GENERAL,
};

enum Menu
{
	MENU_MAIN,
	MENU_SETTINGS,
	MENU_DEFAULT,
};

extern Menu currentMenu;
void goBack();
void switchMenuTo(Menu newMenu);

#endif