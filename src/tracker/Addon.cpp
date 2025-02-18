//############## Milkytracker addons ################
//##                                               ##
//## get more at https://..../                     ##
//##                                               ##
//###################################################
//
//# syntax: name      ; command
//################################################
//hello linux         ; cp %s %s  && pwd
//hello windows       ; copy %s %s 
//hello linux sliders ; hellosliders.sh %s %s %foo:1:5:3 %bar:1:5:3
//
//# ffmpeg.com
//smooth              ; ffmpeg -y -hide_banner -i %s -af "afftdn=nr=%reduction:1:97:10:nf=-%floor_noise:20:80:50:bm=%band_multiply:1:5:1:gs=%gain_smooth:0:50:0" %s
//stretch speedup     ; ffmpeg -y -hide_banner -i %s -af "atempo=%speed:1:5:1" %s
//stretch slowdown    ; ffmpeg -y -hide_banner -i %s -af "atempo=0.%speed:5:9:9" %s

// if first cmd not available, grayed out 
// if 2 %s: it means export-import (sampleditor) script?
// %s.pd? triggers filepicker



#include "Addon.h"
#include "PPOpenPanel.h"
#include "Tracker.h"
#include "SampleEditor.h"
#include "PPSystem.h"
#include "ControlIDs.h"
#include "SectionSamples.h"
#include "SampleEditorControl.h"

// singleton (since addons should never run in parallel)
FILE* Addon::addons          = NULL;
PPString Addon::addonsFolder = PPString("");
PPString Addon::addonsFile   = PPString("");
Tracker* Addon::tracker       = NULL;
Param Addon::params[MAX_PARAMS];
int Addon::param_count       = 0;
PPString Addon::selectedName  = PPString();
PPString Addon::selectedCmd   = PPString();

void Addon::load( PPString _addonsFile, PPContextMenu *menu, Tracker *_tracker ){

	PPString path = PPString(System::getConfigFileName());
	path.deleteAt( path.length()-6, 6); // strip 'config'
	addonsFolder = path;
	addonsFile   = PPString(path);
	addonsFile.append(_addonsFile);
	tracker = _tracker;
	loadAddons();
	loadAddonsToMenu(menu);
}

void Addon::loadAddons() {
    if (addons != NULL) {
        fclose(addons);
        addons = NULL;
    }
    addons = fopen(addonsFile.getStrBuffer(), "r");
    if (!addons) {
        printf("addons: did not detect %s\n", addonsFile.getStrBuffer());
        return;
    }else printf("addons: loading %s\n", addonsFile.getStrBuffer());
}

void Addon::loadAddonsToMenu(PPContextMenu* menu) {
    if (!addons) return;
    char name[100], cmd[255], line[1024];
    int i      = 0;
    rewind(addons);
	while (fgets(line,sizeof(line),addons)){
		if( line[0] == '#' ) continue; // skip comments
		if( sscanf(line, ADDON_FORMAT, name, cmd) == ADDON_TOKENS && i < ADDON_MAX) {
			menu->addEntry(name, MenuID + i);
			i++;
		}
	}
	menu->addEntry("\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4", MenuIDEdit + 1);
	menu->addEntry("edit addons", MenuIDEdit );
}

int Addon::runMenuItem( const FilterParameters *par){
	pp_int32 selected_instrument;
	pp_int32 selected_sample;
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
	if( addonsFolder.length() != 0 ) currentPath->change(addonsFolder);

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

	int ret = runAddon(fin, fout, par);

	if (ret != 0 && ret != -1){
		tracker->showMessageBox(MESSAGEBOX_UNIVERSAL, "addon failed :( see console", Tracker::MessageBox_OK);
		return ret;
	}
	printf("addon: importing /tmp/out.wav\n");
	tracker->getModuleEditor()->loadSample(
		fout,
		selected_instrument,
		selected_sample,
		ModuleEditor::SampleFormatTypeWAV);
		tracker->sectionSamples->updateAfterLoad();
		tracker->sectionSamples->getSampleEditorControl()->rangeAll(true);
		tracker->sectionSamples->getSampleEditorControl()->showAll();
	//if( selected ){
	//	tracker->getModuleEditor()->setSampleName(selected_instrument, selected_sample, selected.getStrBuffer(), selected.length() );
	//}
	tracker->getSampleEditor()->finishUndo();
	tracker->getSampleEditor()->postFilter();
	currentPath->change(projectPath); // restore
	return ret;

}


int Addon::runAddon(const PPString& fin, const PPString& fout, const FilterParameters *par) {
	int res;
    char finalCmd[1024];

	setenv("MILKY_ADDON",selectedName,true);


    PPPath* currentPath = PPPathFactory::createPath();
    currentPath->change(addonsFolder);

	// replace params if any
	PPString finalCmdWithParams = PPString(selectedCmd);
	int nparams = par->getNumParameters();
	int ivalue;
	float fvalue;
	for( int i = 0; i < nparams; i++ ){
		char tmp[20];
		fvalue = par->getParameter(i).floatPart;
		ivalue = int(fvalue);                     // why not .intPart   ?
		snprintf(tmp,20,"%i",ivalue);             // this looks/is convoluted 
		PPString value  = PPString(tmp);          // but it bugfixes PPString( int(par->getParameter(i).floatPart ) )
		PPString token  = PPString("%");     
		token.append( params[i].label );
		token.append(":");
		snprintf(tmp,15,"%i", params[i].min);
		token.append( tmp );
		token.append(":");
		snprintf(tmp,15,"%i", params[i].max);
		token.append( tmp );
		token.append(":");
		snprintf(tmp,15,"%i", params[i].value);
		token.append( tmp );
		finalCmdWithParams.search_replace( token.getStrBuffer(), value.getStrBuffer() );
	}
	// invoke without arguments [to present slider values]
    snprintf(finalCmd, sizeof(finalCmd), finalCmdWithParams.getStrBuffer(),
             fin.getStrBuffer(),
             fout.getStrBuffer());

    printf("addons: %s\n", finalCmd );
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

void Addon::editAddons() {
    char command[512];
	PPString application_cmd = PPString(""); // get from db or env

    // If a specific application command is provided, use it
    if (!application_cmd.length() == 0) {
        snprintf(command, sizeof(command), "%s \"%s\" &", 
                 application_cmd.getStrBuffer(), addonsFile.getStrBuffer());
    } else {
#ifdef _WIN32
        // On Windows, use `start` to open the file
        snprintf(command, sizeof(command), "start \"\" \"%s\"", addonsFile.getStrBuffer());
#elif __APPLE__
        // On macOS, use `open` to open the file (no polling needed)
        snprintf(command, sizeof(command), "open \"%s\"", addonsFile.getStrBuffer());
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
            snprintf(command, sizeof(command), "%s \"%s\" &", selectedCommand, addonsFile.getStrBuffer());
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

void Addon::onMenuSelect(int commandId, Tracker *tracker){

    int i = 0;
    char name[100], cmd[1024], line[2048];
    PPString selectedFile;
    rewind(addons);

	while (fgets(line,sizeof(line),addons)){
		if( line[0] == '#' ) continue; // skip comments
		if( sscanf(line, ADDON_FORMAT, name, cmd) == ADDON_TOKENS && i < ADDON_MAX) {
			if (MenuID + i == commandId) {
				//if( pickfile )
				//	filepicker(name, ext, &selectedFile, screen);
				//}
				selectedName = PPString(name).subString(0, 24);
				selectedCmd  = PPString(cmd);
				parseParams(cmd);
			}
			i++;
		}
	}
}

void Addon::parseParams(const char *str){
    param_count = 0;
    const char *ptr = str;
    while ((ptr = strchr(ptr, '%')) != nullptr) {
        // Skip %s placeholders
        if (strncmp(ptr, "%s", 2) == 0) {
            ptr += 2;
            continue;
        }
        // Try to extract parameters in %key:min:max:value format
        char label[16];
        int min, max, value;
        if (sscanf(ptr, "%%%15[^:]:%d:%d:%d", label, &min, &max, &value) == 4) {
            // Store in params array
            if (param_count < MAX_PARAMS) {
                strncpy(params[param_count].label, label, sizeof(params[param_count].label) - 1);
                params[param_count].label[sizeof(params[param_count].label) - 1] = '\0';
                params[param_count].min = min;
                params[param_count].max = max;
                params[param_count].value = value;
                param_count++;
            }
        }


        // Move to the next '%' occurrence
        ptr++;
    }	
   for (int i = 0; i < param_count; i++) {
		printf("%s %i %i %i\n", params[i].label,params[i].min,params[i].max,params[i].value);
    }
}
