#ifndef POLYSCRIPT_H
#define POLYSCRIPT_H

#include "Tracker.h"
#include "ModuleEditor.h"
#include "PPUI.h"
#include "DialogBase.h"
#include "PPPathFactory.h"
#include <cstdio>
#include <cstring>

#define SCRIPTS_MAX 75
#define SCRIPTS_TOKENS 3
#define SCRIPTS_FORMAT "%99[^;];%20[^;];%255[^\n]\n"
#define PARAMS_FORMAT "%15[^;];%15[^;];%s"

// <name>;<cmd format string>|<extension_for_filedialog>

class Addon {
public:
    static const int MenuID = 10000;
    static const int MenuIDFile = MenuID - 1;
    static const int MenuIDEdit = MenuID + SCRIPTS_MAX;

	static void load( PPString _scriptsFile, PPContextMenu *menu, Tracker *tracker );
    static void loadScripts();
	static void editScripts();
    static void loadScriptsToMenu(PPContextMenu* menu);
    static int runScriptMenuItem(const PPString& cwd, int ID, char* cmd, PPScreen* screen,
                                 const PPString& fin, const PPString& fout, PPString* selectedName);
    static int runScript(const PPString& cwd, const char* cmd, PPScreen* screen,
                         const PPString& fin, const PPString& fout, const PPString& file);
    static void filepicker(const char* name, const char* ext, PPString* result, PPScreen* screen);
    static void onScriptMenu(int commandId, Tracker *tracker);

private:
    static FILE* scripts;
	static PPString scriptsFolder;
	static PPString scriptsFile;
};

#endif // SCRIPT_H
