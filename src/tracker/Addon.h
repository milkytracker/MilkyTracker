// addon singleton (since scripts should never run in parallel)

#ifndef ADDON_H
#define ADDON_H

#include "Tracker.h"
#include "ModuleEditor.h"
#include "PPUI.h"
#include "DialogBase.h"
#include "PPPathFactory.h"
#include "FilterParameters.h"
#include <cstdio>
#include <cstring>

#define ADDON_MAX 75
#define ADDON_TOKENS 2
// <name>;<cmd format string>|<extension_for_filedialog>
#define MAX_PARAMS 30
#define MAX_CMD_LENGTH 4096
#define ADDON_FORMAT "%99[^;];%2048[^\n]\n"

typedef struct {
    int min, max, value;
    char label[25];
} Param;


class Addon {
public:
    static const int MenuID = 10000;
    static const int MenuIDFile   = MenuID - 1;
    static const int MenuIDEdit   = MenuID + ADDON_MAX;
    static const int MenuIDImport = MenuID + ADDON_MAX + 1;
	static Param params[MAX_PARAMS];
	static int param_count;
	static PPString selectedName;
	static PPString selectedCmd;

	static void load( PPString _addonsFile, PPContextMenu *menu, Tracker *tracker );
    static void loadAddons();
	static void importResult(int selected_instrument, int selected_sample );
	static void editAddons();
    static void loadAddonsToMenu(PPContextMenu* menu);
    static void onMenuSelect(int commandId, Tracker *tracker);
    static int runMenuItem(const FilterParameters *par);
    static int runAddon( const FilterParameters *par);
    static void filepicker(const char* name, const char* ext, PPString* result, PPScreen* screen);
	static void parseParams(const char *str);

private:
    static FILE* addons;
	static PPString addonsFolder;
	static PPString addonsFile;
	static Tracker *tracker;
	static PPString fin;  
	static PPString fout;
};

#endif // SCRIPT_H
