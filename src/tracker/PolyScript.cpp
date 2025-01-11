#include "PolyScript.h"

FILE* PolyScript::scripts = NULL;

void PolyScript::loadScripts(const PPString& scriptsFile, PPString* scriptsFolder) {
    if (scripts) {
        fclose(scripts);
        scripts = NULL;
    }
    scripts = fopen(scriptsFile.getStrBuffer(), "r");
    if (!scripts) {
        printf("Error: could not open %s\n", scriptsFile.getStrBuffer());
        return;
    }
    *scriptsFolder = scriptsFile;
    PPString path = scriptsFolder->stripPath();
    scriptsFolder->deleteAt(scriptsFolder->length() - path.length() - 1, path.length() + 1);
}

void PolyScript::loadScriptsToMenu(PPContextMenu* menu) {
    if (!scripts) return;

    char name[100], cmd[255], ext[20];
    int i = 0;
    rewind(scripts);

    while (fscanf(scripts, SCRIPTS_FORMAT, name, cmd, ext) == SCRIPTS_TOKENS && i < SCRIPTS_MAX) {
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

    if (ID == MenuIDFile) {
        return runScript(cwd, cmd, screen, fin, fout, selectedFile);
    }

    while (fscanf(scripts, SCRIPTS_FORMAT, name, cmd, ext) == SCRIPTS_TOKENS && i < SCRIPTS_MAX) {
        if (MenuID + i == ID) {
            if (strlen(ext) == 0 || strcmp(ext, "exec") == 0) {
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
    return system(finalCmd);
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

    // If a specific application command is provided, use it
    if (!application_cmd.isEmpty()) {
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

