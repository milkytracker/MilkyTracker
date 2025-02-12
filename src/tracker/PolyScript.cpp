#include "PolyScript.h"
#include "PPOpenPanel.h"
#include "Tracker.h"
#include "SampleEditor.h"
#include "PPSystem.h"
#include "ControlIDs.h"
#include "SectionSamples.h"
#include "SampleEditorControl.h"

FILE* PolyScript::scripts = NULL;
PPString PolyScript::scriptsFolder = PPString("");

void PolyScript::load( PPString scriptsFile, PPContextMenu *menu, Tracker *tracker ){

	PPString path = PPString(System::getConfigFileName());
	path.deleteAt( path.length()-6, 6); // strip 'config'
	scriptsFolder = path;
	loadScripts(scriptsFile );
	loadScriptsToMenu(menu);
}

void PolyScript::loadScripts(PPString scriptsFile) {
    if (scripts) {
        fclose(scripts);
        scripts = NULL;
    }

	PPString scriptsFileAbs = PPString(scriptsFolder);
	scriptsFileAbs.append(scriptsFile);
    scripts = fopen(scriptsFileAbs.getStrBuffer(), "r");
    if (!scripts) {
        printf("scripts: did not detect %s\n", scriptsFileAbs.getStrBuffer());
        return;
    }else printf("scripts: detected %s\n", scriptsFileAbs.getStrBuffer());
}

void PolyScript::loadScriptsToMenu(PPContextMenu* menu) {
    if (!scripts) return;

    char name[100], cmd[255], ext[20];
    int i = 0;
    rewind(scripts);

    while (fscanf(scripts, SCRIPTS_FORMAT, name, ext, cmd) >= SCRIPTS_TOKENS_MIN && i < SCRIPTS_MAX) {
		printf("%s | %s | %s\n",name,ext,cmd);
        menu->addEntry(name, MenuID + i);
        i++;
    }
}

int PolyScript::runScriptMenuItem(const PPString& cwd, int ID, char* cmd, PPScreen* screen,
                                 const PPString& fin, const PPString& fout, PPString* selectedName) {
    if (!scripts) return -1;

    int i = 0;
    char name[100], ext[20];
    PPString selectedFile;
    rewind(scripts);


    while (fscanf(scripts, SCRIPTS_FORMAT, name, ext, cmd) >= SCRIPTS_TOKENS_MIN && i < SCRIPTS_MAX) {
        if (MenuID + i == ID) {
	printf("JA %s %s %s:]\n",name,cmd, ext);
            if (strlen(ext) == 0 || PPString(ext).startsWith(" ") || strcmp(ext, "exec") == 0) {
                ext[0] = '\0'; // Default to "exec" if empty
            } else {
                filepicker(name, ext, &selectedFile, screen);
            }
            *selectedName = PPString(name).subString(0, 24);
            return runScript(cwd, cmd, screen, fin, fout, selectedFile);
        }
        i++;
    }

    return -1;
}

int PolyScript::runScript(const PPString& cwd, const char* cmd, PPScreen* screen,
                         const PPString& fin, const PPString& fout, const PPString& file) {
    char finalCmd[255];
    PPString fclipboard;

    PPPath* currentPath = PPPathFactory::createPath();
    currentPath->change(cwd);

    snprintf(finalCmd, sizeof(finalCmd), cmd,
             fin.getStrBuffer(),
             fout.getStrBuffer(),
             fclipboard.getStrBuffer(),
             file.getStrBuffer());

    printf("> %s\n", finalCmd);
    return 1; //system(finalCmd);
}

void PolyScript::filepicker(const char* name, const char* ext, PPString* result, PPScreen* screen) {
    const char* extensions[] = {ext, name, NULL, NULL};

    PPOpenPanel* openPanel = new PPOpenPanel(screen, name);
    if (openPanel) {
        openPanel->addExtensions(extensions);

        if (openPanel->runModal() == PPModalDialog::ReturnCodeOK) {
            *result = openPanel->getFileName();
        }

        delete openPanel;
    }
}

void PolyScript::editScripts(const PPString& scriptsFile) {
    char command[512];
	PPString application_cmd = PPString(""); // get from db or env

    // If a specific application command is provided, use it
    if (!application_cmd.length() == 0) {
        snprintf(command, sizeof(command), "%s \"%s\" &", 
                 application_cmd.getStrBuffer(), scriptsFile.getStrBuffer());
    } else {
#ifdef _WIN32
        // On Windows, use `start` to open the file
        snprintf(command, sizeof(command), "start \"\" \"%s\"", scriptsFile.getStrBuffer());
#elif __APPLE__
        // On macOS, use `open` to open the file (no polling needed)
        snprintf(command, sizeof(command), "open \"%s\"", scriptsFile.getStrBuffer());
#else
        // On Linux, poll for available commands
        const char* commands[] = { "xdg-open", "gnome-open", "kde-open", "open", NULL };
        const char* selectedCommand = NULL;

        for (int i = 0; commands[i] != NULL; i++) {
            char testCmd[128];
            snprintf(testCmd, sizeof(testCmd), "command -v %s > /dev/null 2>&1", commands[i]);
            if (system(testCmd) == 0) { // Found a valid command
                selectedCommand = commands[i];
                break;
            }
        }

        if (selectedCommand != NULL) {
            snprintf(command, sizeof(command), "%s \"%s\" &", selectedCommand, scriptsFile.getStrBuffer());
        } else {
            fprintf(stderr, "Error: No supported file opener command found (xdg-open, gnome-open, kde-open, open).\n");
            return;
        }
#endif
    }

    // Print the command for debugging purposes
    printf("> %s\n", command);

    // Execute the command asynchronously
    system(command);
}

void PolyScript::onScriptMenu(int commandId, Tracker *tracker){
	char cmd[255];
	pp_int32 selected_instrument;
	pp_int32 selected_sample;
	PPString selected;
	SampleEditor *sampleEditor = tracker->getSampleEditor();

	#if defined(WINDOWS) || defined(WIN32) // C++ >= v17
	AllocConsole();				   // popup console for errors
	HWND hwnd = ::GetConsoleWindow();
	if (hwnd != NULL){ // prevent user from closing console (thus milkytracker)
		HMENU hMenu = ::GetSystemMenu(hwnd, FALSE);
		if (hMenu != NULL) DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
	}
	freopen("conin$", "r", stdin);
	freopen("conout$", "w", stdout);
	freopen("conout$", "w", stderr);
	#endif

	PPPath *currentPath = PPPathFactory::createPath();
	PPString projectPath = currentPath->getCurrent();
	if( scriptsFolder.length() != 0 ) currentPath->change(scriptsFolder);
	PPString fin   = PPString("in.wav");
	PPString fout  = PPString("out.wav");
	selected_instrument = (pp_int32)tracker->getListBoxInstruments()->getSelectedIndex();
	selected_sample     = (pp_int32)tracker->getListBoxSamples()->getSelectedIndex();
	// save samples to local disk
	tracker->getModuleEditor()->saveSample(
		fin,
		selected_instrument,
		selected_sample,
		ModuleEditor::SampleFormatTypeWAV);
	tracker->getModuleEditor()->saveSample(
		fout,
		selected_instrument,
		selected_sample,
		ModuleEditor::SampleFormatTypeWAV);
	sampleEditor->prepareUndo();
	int ret = runScriptMenuItem(scriptsFolder, commandId, cmd, tracker->screen, fin, fout, &selected);
	if (ret != 0 && ret != -1)
		return tracker->showMessageBox(MESSAGEBOX_UNIVERSAL, "script error :/", Tracker::MessageBox_OK);
	tracker->getModuleEditor()->loadSample(
		fout,
		selected_instrument,
		selected_sample,
		ModuleEditor::SampleFormatTypeWAV);
	tracker->getModuleEditor()->setSampleName(selected_instrument, selected_sample, selected.getStrBuffer(), selected.length() );
	tracker->sectionSamples->updateAfterLoad();
	tracker->sectionSamples->getSampleEditorControl()->rangeAll(true);
	tracker->sectionSamples->getSampleEditorControl()->showAll();
	sampleEditor->finishUndo();
	sampleEditor->postFilter();
	currentPath->change(projectPath); // restore
}
