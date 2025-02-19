//############## Milkytracker addons ##############################
//##                                                             ##
//## get more at https://..../                                   ##
//##                                                             ##
//## vars  : %s                   input [exported] wav           ##
//##         %s                   output [imported[ wav          ##
//##         %p(name:min:max:val) sliderdialog parameter value   ##
//##                                                             ##                      
//## syntax: name            ; command                           ##
//#################################################################
//
//
//# FFMPEG
//# https://ffmpeg.org
//#
//# FX
//ffmpeg smooth               ; ffmpeg -y -hide_banner -i %s -af "afftdn=nr=%p(reduction:1:97:10):nf=-%p(floor_noise:20:80:50):bm=%p(band_multiply:1:5:1):gs=%p(gain_smooth:0:50:0)" %s
//ffmpeg stretch speedup      ; ffmpeg -y -hide_banner -i %s -af "atempo=%p(speed:1:5:1)" %s
//ffmpeg stretch slowdown     ; ffmpeg -y -hide_banner -i %s -af "atempo=0.%p(speed:5:9:9)" %s
//
//# SOX 
//# https://sourceforge.net/projects/sox/
//#
//# SAMPLERS
//#
//# ps1. uncomment the ones you want (and which work for your OS)
//# ps2. if you dont want them to freeze the UI, add '&' and select 'Addon > import from addon' afterwards
//# ps3. if you're on windows, uncomment the sox/waveaudio one
//# TIP: add your own oneliner which immediately targets the right backed+inputdevice and call it 'hardwaresynth' e.g.
//#
//sampler: sox/alsa          ; sox -V6 -t alsa hw:%p(alsa_device:0:20:0) -c1 -b16 -r44100 %s trim 0 %p(duration_sec:1:10:1)
//sampler: sox/pulse input   ; sox -V6 -t pulseaudio default -c1 -b16 -r44100 %s trim 0 %p(duration_sec:1:10:1)
//sampler: sox/pulse output  ; sox -V6 -t pulseaudio $(pactl list sources short | awk '/\.monitor\t/ {print $2}') -c1 -b16 -r44100 %s trim 0 %p(duration_sec:1:10:1)
//sampler: sox/coreaudio     ; sox -V6 -t coreaudio default -c1 -b16 -r44100 %s trim 0 %p(duration_sec:1:10:1)
//sampler: sox/JACK          ; sox -V6 -t jack default -c1 -b16 -r44100 %s trim 0 %p(duration_sec:1:10:1)
//sampler: sox/waveaudio     ; sox -V6 -t waveaudio %p(pulse_device:0:20:0) -c1 -b16 -r44100 %s trim 0 %p(duration_sec:1:10:1)
//sampler: JACK/jack_capture ; jack_capture -d %p(duration_sec:1:10:1) --channels 1 --port system:playback* %s
//sampler: arecord/pulse     ; arecord -D pulse -f S16_LE -r 44100 -c 1 -d %p(duration_sec:1:10:1) %s
//sampler: arecord/alsa      ; arecord -D hw:%p(alsa_device:0:20:0),0 -f S16_LE -r 44100 -c 1 -d %p(duration_sec:1:10:1) %s
//sampler: pulseaudio/parec  ; timeout %p(duration_sec:1:10:1) parec -d $(pactl list sources short | awk '/\.monitor\t/ {print $2}') --channels=1 --rate=44100 --format=s16le --file-format=wav %s
//
//# linux/mac apps
//tenacity                   ; tenacity %s &
//audacity                   ; audacity %s &
//sunvox                     ; sunvox %s &
//polyphone                  ; polyphone &
//
//# windows apps
//audacity                   ; start /b audacity.exe %s
//sunvox                     ; start /b sunvox.exe %s
//
//# linux specific
//#hello linux               ; echo %p(hello:1:5:3) && cp %s %s
//
//# windows specific ##############################################
//#hello windows             ; echo %p(hello:1:5:3) && copy %s %s 
//


#include "Addon.h"
#include "Addons.h"
#include "PPOpenPanel.h"
#include "Tracker.h"
#include "SampleEditor.h"
#include "PPSystem.h"
#include "ControlIDs.h"
#include "SectionSamples.h"
#include "SampleEditorControl.h"

// singleton (since addons should never run in parallel)
Param Addon::params[MAX_PARAMS];
FILE* Addon::addons          = NULL;
PPString Addon::addonsFolder = PPString("");
PPString Addon::addonsFile   = PPString("");
Tracker* Addon::tracker       = NULL;
int Addon::param_count        = 0;
PPString Addon::selectedName  = PPString();
PPString Addon::selectedCmd   = PPString();
PPString Addon::fin           = PPString();
PPString Addon::fout          = PPString();

void Addon::load( PPString _addonsFile, PPContextMenu *menu, Tracker *_tracker ){

	PPString path = PPString(System::getConfigFileName());
	path.deleteAt( path.length()-6, 6); // strip 'config'
	addonsFolder = path;
	addonsFile   = PPString(path);
	addonsFile.append(_addonsFile);
	// generate temporary files
	PPString tmpFilePath = PPString(ModuleEditor::getTempFilename());
	PPString tmpFile    = tmpFilePath.stripPath();
	tmpFilePath.deleteAt( tmpFilePath.length()-tmpFile.length(), tmpFile.length());
	fin   = PPString(tmpFilePath);
	fout  = PPString(tmpFilePath);
	fin.append("in.wav");
	fout.append("out.wav");

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
		addons = fopen(addonsFile.getStrBuffer(), "wb");
		if( addons ){
			fwrite(src_tools_addons_txt,1,src_tools_addons_txt_len,addons);
			fclose(addons);
			addons = fopen(addonsFile.getStrBuffer(), "r");
		}else printf("addons: could not write default %s\n", addonsFile.getStrBuffer());
    }
	printf("addons: loading %s\n", addonsFile.getStrBuffer());
}

void Addon::loadAddonsToMenu(PPContextMenu* menu) {
    if (!addons) return;
    char name[100], cmd[MAX_CMD_LENGTH], line[1024], testCmd[128];
	bool brokenAddon = false;
    int i            = 0;
    rewind(addons);
	while (fgets(line,sizeof(line),addons)){
		if( line[0] == '#' ) continue; // skip comments
		if( sscanf(line, ADDON_FORMAT, name, cmd) == ADDON_TOKENS && i < ADDON_MAX) {
			char *firstCmd = strtok(cmd," ");
            snprintf(testCmd, sizeof(testCmd), "command -v %s > /dev/null 2>&1", firstCmd );
            if (system(testCmd) == 0) { // application is installed/accessible 
				menu->addEntry(name, MenuID + i);
            }else{
				brokenAddon = true;
				if( getenv("ADDON_DEBUG") ) printf("addons: '%s' not detected\n", firstCmd, name );
			}
			i++;
		}
	}
	if( brokenAddon && getenv("ADDON_DEBUG") ){
		printf("addons: to enjoy (optional) addons, install missing utils to %s [or add to PATH]\n", addonsFolder.getStrBuffer());
	}

	menu->addEntry("\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4", MenuIDEdit + 1);
	menu->addEntry("import from addon", MenuIDImport );
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

	int ret = runAddon(par);

	if (ret != 0 && ret != -1){
		tracker->showMessageBox(MESSAGEBOX_UNIVERSAL, "addon failed :( see console", Tracker::MessageBox_OK);
		return ret;
	}
	importResult(selected_instrument, selected_sample);
	//if( selected ){
	//	tracker->getModuleEditor()->setSampleName(selected_instrument, selected_sample, selected.getStrBuffer(), selected.length() );
	//}
	tracker->getSampleEditor()->finishUndo();
	tracker->getSampleEditor()->postFilter();
	currentPath->change(projectPath); // restore
	return ret;

}

void Addon::importResult(int selected_instrument, int selected_sample ){
	printf("addons: importing /tmp/out.wav\n");
	tracker->getModuleEditor()->loadSample(
		fout,
		selected_instrument,
		selected_sample,
		ModuleEditor::SampleFormatTypeWAV);
	tracker->sectionSamples->updateAfterLoad();
	tracker->sectionSamples->getSampleEditorControl()->rangeAll(true);
	tracker->sectionSamples->getSampleEditorControl()->showAll();
}


int Addon::runAddon(const FilterParameters *par) {
	int res;
    char finalCmd[MAX_CMD_LENGTH];

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
		PPString token  = PPString("%p(");     
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
		token.append(")");
		finalCmdWithParams.search_replace( token.getStrBuffer(), value.getStrBuffer() );
	}
	// invoke without arguments [to present slider values]
    snprintf(finalCmd, sizeof(finalCmd), finalCmdWithParams.getStrBuffer(),
             finalCmdWithParams.matches("%s") == 2 ? fin.getStrBuffer() : fout.getStrBuffer(),
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
        snprintf(command, sizeof(command), "start \"\" \"%s\"", addonsFile.getStrBuffer()); // start /b for bg?
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
    char name[100], cmd[MAX_CMD_LENGTH], line[MAX_CMD_LENGTH];
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
				printf("selected=%s\n",selectedCmd.getStrBuffer());
				parseParams(cmd);
			}
			i++;
		}
	}
}

void Addon::parseParams(const char *str){
    param_count = 0;
    const char *ptr = str;

    while ((ptr = strstr(ptr, "%p(")) != nullptr) {  // Look for "%p("
        ptr += 3;  // Move past "%p("

        // Extract content inside the parentheses
        char label[16];
        int min, max, value;
        if (sscanf(ptr, "%15[^:]:%d:%d:%d)", label, &min, &max, &value) == 4) {
            if (param_count < MAX_PARAMS) {
                strncpy(params[param_count].label, label, sizeof(params[param_count].label) - 1);
                params[param_count].label[sizeof(params[param_count].label) - 1] = '\0';
                params[param_count].min = min;
                params[param_count].max = max;
                params[param_count].value = value;
                param_count++;
            }
        }

        // Move past the closing parenthesis `)` to find the next parameter
        ptr = strchr(ptr, ')');
        if (!ptr) break; // Stop if no closing parenthesis is found
        ptr++; // Move to the next character after ')'
    }

    // Debug output
    for (int i = 0; i < param_count; i++) {
        printf("Parsed: %s %d %d %d\n", params[i].label, params[i].min, params[i].max, params[i].value);
    }	
}
