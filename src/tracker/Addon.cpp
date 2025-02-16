// ############## Milkytracker addons ################
// ##                                               ##
// ## get more at https://..../                     ##
// ##                                               ##
// ###################################################
// 
// # syntax: name      ; flags ; command
// ################################################
// hello linux         ; w     ; cp %s %s  && pwd
// hello windows       ; w     ; copy %s %s 
// hello linux sliders ; w    ; hellosliders.sh %s %s $foo:1:5:3 bar:1:5:3
// 
// # ffmpeg.com
// ffmpeg smooth       ; w    ; ffmpeg.com --smooth %s %s
// 
// ###################################################
// ##                                               ##
// ##      flags:  w = sample editor addon          ##
// ##                  (shows up in contextmenu)    ##
// ##                                               ##
// ##              s = invokes parameter dialog     ##
// ##                  the cmd runs twice now:      ##
// ##                                               ##
// ##                    1) PARAMS=1 [the cmd]      ##
// ##                                               ##
// ##                       output: foo;1:10:5      ##
// ##                               bar;1:20:3      ##
// ##                               ..and so on     ##
// ##                                               ##
// ##                    2) [the cmd]               ##
// ##                                               ##
// ##                                               ##
// ##                                               ##
// ###################################################
// 


#include "Addon.h"
#include "PPOpenPanel.h"
#include "Tracker.h"
#include "SampleEditor.h"
#include "PPSystem.h"
#include "ControlIDs.h"
#include "SectionSamples.h"
#include "SampleEditorControl.h"

FILE* Addon::scripts = NULL;
PPString Addon::scriptsFolder = PPString("");
PPString Addon::scriptsFile   = PPString("");
Tracker *tracker = NULL;

void Addon::load( PPString _scriptsFile, PPContextMenu *menu, Tracker *_tracker ){

	PPString path = PPString(System::getConfigFileName());
	path.deleteAt( path.length()-6, 6); // strip 'config'
	scriptsFolder = path;
	scriptsFile   = PPString(path);
	scriptsFile.append(_scriptsFile);
	tracker = _tracker;
	loadScripts();
	loadScriptsToMenu(menu);
}

void Addon::loadScripts() {
    if (scripts != NULL) {
        fclose(scripts);
        scripts = NULL;
    }
    scripts = fopen(scriptsFile.getStrBuffer(), "r");
    if (!scripts) {
        printf("addons: did not detect %s\n", scriptsFile.getStrBuffer());
        return;
    }else printf("addons: loading %s\n", scriptsFile.getStrBuffer());
}

void Addon::loadScriptsToMenu(PPContextMenu* menu) {
    if (!scripts) return;

    char name[100], cmd[255], ext[20], line[1024];
    int i      = 0;
    rewind(scripts);

	while (fgets(line,sizeof(line),scripts)){
		if( line[0] == '#' ) continue; // skip comments
		if( sscanf(line, SCRIPTS_FORMAT, name, ext, cmd) == SCRIPTS_TOKENS && i < SCRIPTS_MAX) {
			menu->addEntry(name, MenuID + i);
			i++;
		}
	}
	menu->addEntry("\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4", MenuIDEdit + 1);
	menu->addEntry("edit addons", MenuIDEdit );
}

int Addon::runScriptMenuItem(const PPString& cwd, int ID, char* cmd, PPScreen* screen,
                                 const PPString& fin, const PPString& fout, PPString* selectedName) {
    if (!scripts) return -1;

    int i = 0;
    char name[100], ext[20], line[1024];
    PPString selectedFile;
    rewind(scripts);

	while (fgets(line,sizeof(line),scripts)){
		if( line[0] == '#' ) continue; // skip comments
		if( sscanf(line, SCRIPTS_FORMAT, name, ext, cmd) == SCRIPTS_TOKENS && i < SCRIPTS_MAX) {
			if (MenuID + i == ID) {
				if (strlen(ext) == 0 
						|| PPString(ext).startsWith(" ") 
						|| strcmp(ext, "xp") == 0
						|| strcmp(ext, "xi") == 0
						|| strcmp(ext, "wav") == 0) {
					ext[0] = '\0'; // Default to "exec" if empty
				} else {
					filepicker(name, ext, &selectedFile, screen);
				}
				*selectedName = PPString(name).subString(0, 24);
				return runScript(cwd, cmd, screen, fin, fout, selectedFile, PPString(name) );
			}
			i++;
		}
	}

    return -1;
}


int Addon::runScript(const PPString& cwd, const char* cmd, PPScreen* screen,
                         const PPString& fin, const PPString& fout, const PPString& selectedFile, PPString name) {
    char finalCmd[255];
	int res;

	// parse addon parameters if any
	char *str = "hellosliders.sh %s.wav %s.wav %foo:1:5:3 %bar:1:5:4";
    char addonType[8];
    Param params[MAX_PARAMS];
    int param_count = 0;
    parseAddon(str, addonType, params, &param_count);
    printf("AddonType: %s\n", addonType);
    for (int i = 0; i < param_count; i++) {
        printf("Param[%d]: label=%s, min=%d, max=%d, value=%d\n", i, params[i].label, params[i].min, params[i].max, params[i].value);
    }

	// *TODO* present dialog
	//if( param_count > 0 ){ 
	//	tracker->dialog = new DialogSliders(tracker->screen, toolHandlerResponder, PP_DEFAULT_ID, name, param_count, tracker->getSampleEditor(), &SampleEditor::tool_scaleSample );
	//	DialogSliders *sliders = static_cast<DialogSliders*>(dialog);
	//	for (int i = 0; i < param_count; i++) {
	//	  sliders->initSlider(i, float(params[i].min), float(params[i].max), float(params[i].value, params[i].label );
	//	}

	//	tracker->dialog->show();
	//}

	setenv("SELECTED_FILE",selectedFile.getStrBuffer(),1);

    PPPath* currentPath = PPPathFactory::createPath();
    currentPath->change(cwd);

	// first invoke without arguments [to present slider values]
    snprintf(finalCmd, sizeof(finalCmd), cmd, "", "");
    res = system(finalCmd) >> 8;
	printf("exit=%i\n",res);
	if( res == 255 ){ // present dialog values
		printf("addons: show sliders!\n");
		// setenv("P1",1); 
	}

    snprintf(finalCmd, sizeof(finalCmd), cmd,
             fin.getStrBuffer(),
             fout.getStrBuffer());

    printf("addons: %s\n", finalCmd);
    return system(finalCmd);
}

void Addon::filepicker(const char* name, const char* ext, PPString* result, PPScreen* screen) {
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

void Addon::editScripts() {
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

void Addon::onScriptMenu(int commandId, Tracker *tracker){
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

	if( commandId == MenuIDEdit ) return editScripts();

	PPPath *currentPath = PPPathFactory::createPath();
	PPString projectPath = currentPath->getCurrent();
	if( scriptsFolder.length() != 0 ) currentPath->change(scriptsFolder);

	// write temporary files
	PPString tmpFilePath = PPString(ModuleEditor::getTempFilename());
	PPString tmpFile    = tmpFilePath.stripPath();
	tmpFilePath.deleteAt( tmpFilePath.length()-tmpFile.length(), tmpFile.length());
	PPString fin   = PPString(tmpFilePath);
	PPString fout  = PPString(tmpFilePath);
	fin.append("in.wav");
	fout.append("out.wav");

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
		return tracker->showMessageBox(MESSAGEBOX_UNIVERSAL, "oops! check console :(", Tracker::MessageBox_OK);
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

void Addon::parseAddon(const char *str, char *type, Param *params, int *param_count) {
    *param_count = 0;
    char temp_str[256];
    strncpy(temp_str, str, sizeof(temp_str) - 1);
    temp_str[sizeof(temp_str) - 1] = '\0';
    
    char *token = strtok(temp_str, " ");
    while (token) {
        if (sscanf(token, "%%%[^.].%s", type, type) == 2) {
            // Extract file type
        } else if (sscanf(token, "%%%[^:]:%d:%d:%d", params[*param_count].label,
                           &params[*param_count].min, &params[*param_count].max, &params[*param_count].value) == 4) {
            (*param_count)++;
        }
        token = strtok(NULL, " ");
    }

}
