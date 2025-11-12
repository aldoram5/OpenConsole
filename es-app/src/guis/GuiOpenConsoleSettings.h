#pragma once
#ifndef ES_APP_GUIS_GUI_OPENCONSOLE_SETTINGS_H
#define ES_APP_GUIS_GUI_OPENCONSOLE_SETTINGS_H

#include "GuiSettings.h"
#include "components/MenuComponent.h"

class GuiOpenConsoleSettings : public GuiSettings
{
public:
	GuiOpenConsoleSettings(Window* window);
	virtual ~GuiOpenConsoleSettings();

private:
	void addDatabaseSettings();
	void addPluginSettings();
	void addScanningSettings();
	void addMaintenanceSettings();

	// Actions
	void scanGames();
	void refreshDatabase();
	void clearDatabase();
	void viewDatabaseStats();
	void addScanPath();
	void removeScanPath();
	void configureScanDepth();
};

#endif // ES_APP_GUIS_GUI_OPENCONSOLE_SETTINGS_H
