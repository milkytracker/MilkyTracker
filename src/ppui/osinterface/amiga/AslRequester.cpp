/*
 *  ppui/osinterface/amiga/AslRequester.cpp
 *
 *  Copyright 2017 Juha Niemimaki
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

// This is a common file requester for both load / save functions

#include "AslRequester.h"

#include <stdio.h>

#include <proto/exec.h>
#include <proto/asl.h>
#include <proto/dos.h>

static bool GetCurrentPath(char *buffer, size_t len)
{
    BPTR lock = IDOS->GetCurrentDir();
    int32 success = IDOS->NameFromLock(lock, buffer, len);

    if (success) {
        return true;
    } else {
        puts("Failed to get current dir name");
        return false;
    }
}

PPSystemString GetFileName(CONST_STRPTR title, bool saveMode, CONST_STRPTR name)
{
    PPSystemString fileName = "";

    struct Library *AslBase = IExec->OpenLibrary(AslName, 53);

    if (AslBase) {
        struct AslIFace *IAsl = (struct AslIFace *)IExec->GetInterface(AslBase, "main", 1, NULL);

        if (IAsl) {
            char buffer[MAX_DOS_PATH];

            if (GetCurrentPath(buffer, sizeof(buffer))) {
                struct FileRequester *r = (struct FileRequester *)IAsl->AllocAslRequestTags(
                    ASL_FileRequest,
                    ASLFR_TitleText, title,
                    //ASLFR_PositiveText, "Open file",
                    ASLFR_DoSaveMode, saveMode ? TRUE : FALSE,
                    ASLFR_SleepWindow, TRUE,
                    ASLFR_StayOnTop, TRUE,
                    ASLFR_RejectIcons, TRUE,
                    ASLFR_InitialDrawer, buffer,
                    ASLFR_InitialFile, name,
                    TAG_DONE);

                if (r) {

                    BOOL b = IAsl->AslRequestTags(r, TAG_DONE);

                    //printf("%d '%s' '%s'\n", b, r->fr_File, r->fr_Drawer);

                    if (b != FALSE) {
                        if (strlen(r->fr_Drawer) < sizeof(buffer)) {

                            strncpy(buffer, r->fr_Drawer, sizeof(buffer));
                            IDOS->AddPart(buffer, r->fr_File, sizeof(buffer));

                            fileName = buffer;

                            //printf("%s\n", fileName.getStrBuffer());
                        } else {
                            printf("Path is too long (limit %ld)\n", sizeof(buffer));
                        }
                    }

                    IAsl->FreeAslRequest(r);
                } else {
                    puts("Failed to allocate file requester");
                }
            }

            IExec->DropInterface((struct Interface *)IAsl);
        } else {
            puts("Failed to get ASL interface");
        }

        IExec->CloseLibrary(AslBase);
    } else {
        printf("Failed to open %s\n", AslName);
    }

    return fileName;
}

