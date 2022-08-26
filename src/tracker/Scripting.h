/*
 *  tracker/ScopesControl.h
 *
 *  Copyright 2022 Coderofsalvation / Leon van Kammen 
 *
 *  This file is part of Milkytracker.
 *
 *  Milkytracker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Milkytracker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Milkytracker.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 *  Scripting.h
 *  A simple scripting-bridge for MilkyTracker SampleEditor
 *  Instead of bloating milkytracker with more effects/plugins, the burden of maintenance is reversed: external 
 *  applications & scripts can 'hook' into the sample-editor using the 'Script'-contextmenu.
 * 
 *  Created by coderofsalvation / Leon van Kammen on 26-10-2022 
 */

#ifndef SCRIPT_H
#define SCRIPT_H

#include "Tracker.h"
#include "ModuleEditor.h"
#include "PPUI.h"
#include "DialogBase.h"
#include "PPPathFactory.h"

/*
 * a list of scripting backends can be defined by 
 * the enduser (using the configfile below)
 */

#define SCRIPTS_MAX 75
#define SCRIPTS_TOKENS 3
#define SCRIPTS_FORMAT "%[^';'];%[^'|']|%[^'\n']\n"
//                      <name>;<cmd fmstring>|<exec|extension_for_filedialog>\n

// default example scripts (console will warn if dependencies are not installed)
// more scripts can be found here: https://gitlab.com/coderofsalvation/milkytracker-scripts

class Scripting{
	public:

		static const int MenuID             = 10000;
		static const int MenuIDFile         = MenuID-1;
		static const int MenuIDScriptBrowse = MenuID+SCRIPTS_MAX;
		static FILE *scripts;

		static void loadScripts( PPString scriptsFile, PPString *scriptsFolder ){
			if( scripts != NULL ) fclose(scripts);
			scripts = fopen( scriptsFile.getStrBuffer(), "r");
			if( scripts == NULL ) printf("error: could not open %s",scriptsFile.getStrBuffer());
			scriptsFolder->replace(scriptsFile);
			PPString path = scriptsFolder->stripPath();
			scriptsFolder->deleteAt( scriptsFolder->length()-path.length()-1, path.length()+1 );
		}

		static void loadScriptsToMenu( PPContextMenu *m ){
			if( scripts == NULL ) return;
			// # <name>;<binary>;<cmd>;<exec|extension_for_filedialog>\n
			char name[100], cmd[255], ext[20];
			int  i = 0;
			rewind(scripts);
			while( fscanf(scripts, SCRIPTS_FORMAT, name, cmd, ext) == SCRIPTS_TOKENS && i < SCRIPTS_MAX ){
				m->addEntry( name, MenuID+i);
				i++;
			}
		}

		static int runScriptMenuItem(PPString cwd, int ID, char *cmd, PPScreen *screen, PPString fin, PPString fout, PPString *selectedName ){
			int  i = 0;
			char name[100], ext[20];
			PPString selectedFile;
			if( ID == Scripting::MenuIDFile ){
				// run selected script from filepicker
				return runScript(cwd,cmd,screen,fin,fout,selectedFile);
			}else{ 
				// run menu shortcut item from scripts.txt
				rewind(scripts);
				while( fscanf(scripts, SCRIPTS_FORMAT, name, cmd, ext) == SCRIPTS_TOKENS && i < SCRIPTS_MAX ){
					if( MenuID+i == ID ){ 
						if( strncmp("exec",ext,4) != 0 ) filepicker(name,ext,&selectedFile,screen);
						*selectedName = PPString(name).subString(0,24);
						return runScript(cwd,cmd,screen,fin,fout,selectedFile);
					}
					i++;
				}
			}
			return -1;
		}

		static int runScript(PPString cwd, char *cmd, PPScreen *screen, PPString fin, PPString fout, PPString file ){
			char finalcmd[255];
			PPString fclipboard;
			PPPath *currentPath = PPPathFactory::createPath();
			currentPath->change(cwd);
			sprintf(finalcmd, cmd,  fin.getStrBuffer(),
									fout.getStrBuffer(),
									fclipboard.getStrBuffer(),
									file.getStrBuffer() );
			printf("> %s\n",finalcmd);
			return system(finalcmd);
		}

		static void filepicker(char *name, char *ext, PPString *result, PPScreen *screen ){
			PPOpenPanel* openPanel = NULL;
			const char *extensions[] = {
				ext,name,
				NULL,NULL
			};
			openPanel = new PPOpenPanel(screen, name );
			openPanel->addExtensions(extensions);
			if (!openPanel) return;
			bool res = true;
			if (openPanel->runModal() == PPModalDialog::ReturnCodeOK)
			{
				*result = openPanel->getFileName();
				delete openPanel;
			}
		}
};

FILE* Scripting::scripts              = NULL;

#endif

