/*
 *  tracker/ControlIDs.h
 *
 *  Copyright 2009 Peter Barth
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

#ifndef CONTROLLIDS__H
#define CONTROLLIDS__H

enum
{	
	BUTTON_0 =							100,
	BUTTON_1 =							101,
	BUTTON_2 =							102,
	BUTTON_3 =							103,
	BUTTON_4 =							104,
	BUTTON_5 =							105,
	BUTTON_6 =							106,
	BUTTON_APP_EXIT =					110,
	CONTAINER_SCOPECONTROL =			65536,
	BUTTON_SCOPECONTROL_MUTE =			(65536+1),
	BUTTON_SCOPECONTROL_SOLO =			(65536+2),
	BUTTON_SCOPECONTROL_REC =			(65536+3),
	
	// orderlist control
	BUTTON_ORDERLIST_INSERT =			180,
	BUTTON_ORDERLIST_NEXT =				181,
	BUTTON_ORDERLIST_PREVIOUS =			182,
	BUTTON_ORDERLIST_DELETE =			184,
	BUTTON_ORDERLIST_SONGLENGTH_PLUS =	185,
	BUTTON_ORDERLIST_SONGLENGTH_MINUS =	186,
	BUTTON_ORDERLIST_REPEAT_PLUS =		187,
	BUTTON_ORDERLIST_REPEAT_MINUS =		188,
	BUTTON_ORDERLIST_EXTENT =			189,
	BUTTON_ORDERLIST_SEQENTRY =			190,
	BUTTON_ORDERLIST_CLNENTRY =			191,
	
	BUTTON_JAMMENU_NEXTORDERLIST =		200,
	BUTTON_JAMMENU_PREVORDERLIST =		201,
	STATICTEXT_JAMMENU_CURORDER =		202,
	STATICTEXT_JAMMENU_CURPATTERN =		203,
	BUTTON_JAMMENU_NEXTINSTRUMENT =		204,
	BUTTON_JAMMENU_PREVINSTRUMENT =		205,
	STATICTEXT_JAMMENU_CURINSTRUMENT =	206,
	BUTTON_JAMMENU_TOGGLEPIANOSIZE =	207,
	
	STATICTEXT_ORDERLIST_SONGLENGTH =	10200,
	STATICTEXT_ORDERLIST_REPEAT =    	10201,
	
	// speed control
	BUTTON_BPM_PLUS =					210,
	BUTTON_BPM_MINUS =					211,
	BUTTON_SPEED_PLUS =					212,
	BUTTON_SPEED_MINUS =				213,
	BUTTON_ADD_PLUS =					214,
	BUTTON_ADD_MINUS =					215,
	BUTTON_OCTAVE_PLUS =				216,
	BUTTON_OCTAVE_MINUS =				217,
	BUTTON_SPEEDCONTAINERFLIP =			218,
	
	STATICTEXT_SPEED_BPM_DESC =			10300,
	STATICTEXT_SPEED_SPEED_DESC =  		10301,
	STATICTEXT_SPEED_PATTERNADD_DESC =	10302,
	STATICTEXT_SPEED_OCTAVE_DESC =		10303,
	STATICTEXT_SPEED_MAINVOL_DESC =		10304,
	STATICTEXT_SPEED_BPM =				10305,
	STATICTEXT_SPEED_SPEED =    		10306,
	STATICTEXT_SPEED_PATTERNADD =		10307,
	STATICTEXT_SPEED_OCTAVE =			10308,
	STATICTEXT_SPEED_MAINVOL =			10309,
	
	// pattern control
	BUTTON_PATTERN_PLUS =				220,
	BUTTON_PATTERN_MINUS =				221,
	BUTTON_PATTERN_SIZE_PLUS =			222,
	BUTTON_PATTERN_SIZE_MINUS =			223,
	BUTTON_PATTERN_EXPAND =				224,
	BUTTON_PATTERN_SHRINK =				225,
	BUTTON_PATTERN_CAPTURE =			226,
	BUTTON_PATTERN_CAPTURE_OVERDUB =	227,
	BUTTON_PATTERN_TOGGLE_VIEW     =	228,
	
	STATICTEXT_PATTERN_INDEX =			10400,
	STATICTEXT_PATTERN_LENGTH =    		10401,
	
	// instrument/samples listboxes
	STATICTEXT_INSTRUMENTS_ALTERNATIVEHEADER =	226,
	STATICTEXT_INSTRUMENTS_ALTERNATIVEHEADER2 =	227,
	BUTTON_INSTRUMENTS_FLIP =					228,
	BUTTON_INSTRUMENT =							229,
	BUTTON_INSTRUMENTS_PLUS =					230,
	BUTTON_INSTRUMENTS_MINUS =					231,
	BUTTON_SAMPLES_PLUS =						232,
	BUTTON_SAMPLES_MINUS =						233,
	BUTTON_PIANO_EDIT =							234,
	BUTTON_PIANO_PLAY =							235,
	STATICTEXT_SAMPLEHEADER =					492,
	BUTTON_SAMPLES_INVOKEHDRECORDER =			493,
	
	// envelope/instrument editor
	BUTTON_ENVELOPE_UNDO =				236,
	BUTTON_ENVELOPE_REDO =				237,
	BUTTON_ENVELOPE_COPY =				238,
	BUTTON_ENVELOPE_PASTE =				239,
	BUTTON_ENVELOPE_ADD =				240,
	BUTTON_ENVELOPE_DELETE =			241,
	BUTTON_ENVELOPE_SUSTAIN_PLUS =		242,
	BUTTON_ENVELOPE_SUSTAIN_MINUS =		243,
	BUTTON_ENVELOPE_LOOPSTART_PLUS =	244,
	BUTTON_ENVELOPE_LOOPSTART_MINUS =	245,
	BUTTON_ENVELOPE_LOOPEND_PLUS =		246,
	BUTTON_ENVELOPE_LOOPEND_MINUS =		247,
	BUTTON_ENVELOPE_VOLUME =			248,
	BUTTON_ENVELOPE_PANNING =			249,
	BUTTON_ENVELOPE_PREDEF_STORE =		699,
	BUTTON_ENVELOPE_PREDEF_0 =			700,
	BUTTON_ENVELOPE_PREDEF_1 =			701,
	BUTTON_ENVELOPE_PREDEF_2 =			702,
	BUTTON_ENVELOPE_PREDEF_3 =			703,
	BUTTON_ENVELOPE_PREDEF_4 =			704,
	BUTTON_ENVELOPE_PREDEF_5 =			705,
	BUTTON_ENVELOPE_PREDEF_6 =			706,
	BUTTON_ENVELOPE_PREDEF_7 =			707,
	BUTTON_ENVELOPE_PREDEF_8 =			708,
	BUTTON_ENVELOPE_PREDEF_9 =			709,
	BUTTON_ENVELOPE_SCALEX =			750,
	BUTTON_ENVELOPE_SCALEY =			751,
	BUTTON_ENVELOPE_ZOOMIN =			760,
	BUTTON_ENVELOPE_ZOOMOUT =			761,
	BUTTON_ENVELOPE_ZOOMDEFAULT =		762,
	
	CHECKBOX_ENVELOPE_ON =				250,
	CHECKBOX_ENVELOPE_SUSTAIN =			251,
	CHECKBOX_ENVELOPE_LOOP =			252,
	
	SLIDER_SAMPLE_VOLUME =				253,
	SLIDER_SAMPLE_PANNING =				254,
	SLIDER_SAMPLE_FINETUNE =			255,
	
	BUTTON_SAMPLE_RELNOTENUM_OCTUP =	256,
	BUTTON_SAMPLE_RELNOTENUM_OCTDN =	257,
	BUTTON_SAMPLE_RELNOTENUM_NOTEUP =	258,
	BUTTON_SAMPLE_RELNOTENUM_NOTEDN =	259,
	
	SLIDER_SAMPLE_VOLFADE = 			260,
	SLIDER_SAMPLE_VIBSPEED = 			261,
	SLIDER_SAMPLE_VIBDEPTH = 			262,
	SLIDER_SAMPLE_VIBSWEEP = 			263,
	RADIOGROUP_SAMPLE_VIBTYPE =			264,
	
	BUTTON_INSTRUMENTEDITOR_EXIT =		265,
	BUTTON_INSTRUMENTEDITOR_LOAD =		266,
	BUTTON_INSTRUMENTEDITOR_SAVE =		267,
	
	BUTTON_INSTRUMENTEDITOR_COPY =		268,
	BUTTON_INSTRUMENTEDITOR_SWAP =		269,
	BUTTON_INSTRUMENTEDITOR_CLEAR =		270,
	
	// sample editor
	BUTTON_SAMPLE_PLAY_STOP =			280,
	BUTTON_SAMPLE_PLAY_UP =				281,
	BUTTON_SAMPLE_PLAY_DOWN =			282,
	BUTTON_SAMPLE_PLAY_WAVE =			283,
	BUTTON_SAMPLE_PLAY_RANGE =			284,
	BUTTON_SAMPLE_PLAY_DISPLAY =		285,
	
	BUTTON_SAMPLE_RANGE_SHOW =			286,
	BUTTON_SAMPLE_RANGE_ALL =			287,
	BUTTON_SAMPLE_RANGE_CLEAR =			288,
	BUTTON_SAMPLE_RANGE_ZOOMOUT =		289,
	BUTTON_SAMPLE_RANGE_SHOWALL =		290,
	BUTTON_SAMPLE_APPLY_LASTFILTER =	291,
	
	BUTTON_SAMPLE_EDIT_CUT =			292,
	BUTTON_SAMPLE_EDIT_COPY =			293,
	BUTTON_SAMPLE_EDIT_PASTE =			294,
	
	BUTTON_SAMPLE_EDIT_CROP =			295,
	BUTTON_SAMPLE_EDIT_VOL =			296,
	BUTTON_SAMPLE_EDIT_DRAW =			297,
	
	RADIOGROUP_SAMPLE_LOOPTYPE =		298,
	RADIOGROUP_SAMPLE_RESTYPE =			299,
	
	BUTTON_SAMPLEEDITOR_EXIT =			300,
	
	BUTTON_SAMPLE_LOAD =				301,
	BUTTON_SAMPLE_SAVE =				302,
	
	BUTTON_SAMPLE_EDIT_CLEAR =			303,
	BUTTON_SAMPLE_EDIT_MINIMIZE =		304,
	BUTTON_SAMPLE_EDIT_REPSTARTPLUS =	305,
	BUTTON_SAMPLE_EDIT_REPSTARTMINUS =	306,
	BUTTON_SAMPLE_EDIT_REPLENPLUS =		307,
	BUTTON_SAMPLE_EDIT_REPLENMINUS =	308,
	
	BUTTON_SAMPLE_UNDO =				310,
	BUTTON_SAMPLE_REDO =				311,
	
	BUTTON_SAMPLE_ZOOM_PLUS =			312,
	BUTTON_SAMPLE_ZOOM_MINUS =			313,
	
	CHECKBOX_SAMPLE_ONESHOT =			314,
	
  BUTTON_SAMPLE_SYNTH =		315,
  BUTTON_SAMPLE_SYNTH_RAND =		316,
	
	// INPUT CONTROL
	INPUT_BUTTON_0 =					400,
	INPUT_BUTTON_1 =					401,
	INPUT_BUTTON_2 =					402,
	INPUT_BUTTON_3 =					403,
	INPUT_BUTTON_4 =					404,
	INPUT_BUTTON_5 =					405,
	INPUT_BUTTON_6 =					406,
	INPUT_BUTTON_7 =					407,
	INPUT_BUTTON_8 =					408,
	INPUT_BUTTON_9 =					409,
	INPUT_BUTTON_A =					410,
	INPUT_BUTTON_B =					411,
	INPUT_BUTTON_C =					412,
	INPUT_BUTTON_D =					413,
	INPUT_BUTTON_E =					414,
	INPUT_BUTTON_F =					415,
	INPUT_BUTTON_G =					416,
	INPUT_BUTTON_H =					417,
	INPUT_BUTTON_I =					418,
	INPUT_BUTTON_J =					419,
	INPUT_BUTTON_K =					420,
	INPUT_BUTTON_L =					421,
	INPUT_BUTTON_M =					422,
	INPUT_BUTTON_N =					423,
	INPUT_BUTTON_O =					424,
	INPUT_BUTTON_P =					425,
	INPUT_BUTTON_Q =					426,
	INPUT_BUTTON_R =					427,
	INPUT_BUTTON_S =					428,
	INPUT_BUTTON_T =					429,
	INPUT_BUTTON_U =					430,
	INPUT_BUTTON_V =					431,
	INPUT_BUTTON_W =					432,
	INPUT_BUTTON_X =					433,
	INPUT_BUTTON_Y =					434,
	INPUT_BUTTON_Z =					435,
	INPUT_BUTTON_DEL =					450,
	INPUT_BUTTON_INS =					451,
	INPUT_BUTTON_BACK =					452,
	INPUT_BUTTON_BACKLINE =				453,
	INPUT_BUTTON_INSLINE =				454,
	INPUT_BUTTON_KEYOFF =				455,
	INPUT_BUTTON_MINUS =				457,
	INPUT_BUTTON_PLUS =					458,
	INPUT_BUTTON_BRACKETOPEN =			460,
	INPUT_BUTTON_BRACKETCLOSE =			461,
	INPUT_BUTTON_SEMICOLON =			462,
	INPUT_BUTTON_TICK =					463,
	INPUT_BUTTON_BACKSLASH =			464,
	INPUT_BUTTON_TILDE =				465,
	INPUT_BUTTON_COMMA =				466,
	INPUT_BUTTON_PERIOD =				467,
	INPUT_BUTTON_SLASH =				468,
	INPUT_BUTTON_TAB =					470,
	INPUT_BUTTON_CAPSLOCK =				471,
	INPUT_BUTTON_LSHIFT =				472,
	INPUT_BUTTON_RSHIFT =				473,
	INPUT_BUTTON_ENTER =				474,
	INPUT_BUTTON_SPACE =				475,
	INPUT_BUTTON_SHRINK =				480,
	INPUT_BUTTON_EXPAND =				481,
	INPUT_BUTTON_EDIT =					490,
	INPUT_BUTTON_WTF =					491,
	
	STATICTEXT_ENVELOPE_SUSTAINPT =		10500,
	STATICTEXT_ENVELOPE_LOOPSTARTPT =	10501,
	STATICTEXT_ENVELOPE_LOOPENDPT =		10502,
	
	STATICTEXT_SAMPLE_VOLUME =			10503,
	STATICTEXT_SAMPLE_PANNING =			10504,
	STATICTEXT_SAMPLE_FINETUNE =		10505,
	STATICTEXT_SAMPLE_RELNOTE =			10506,
	STATICTEXT_SAMPLE_VOLFADE =			10507,
	STATICTEXT_SAMPLE_VIBSPEED =		10508,
	STATICTEXT_SAMPLE_VIBDEPTH =		10509,
	STATICTEXT_SAMPLE_VIBSWEEP =		10510,
	
	STATICTEXT_SAMPLE_LENGTH =			10511,
	STATICTEXT_SAMPLE_DISPLAY =			10512,
	STATICTEXT_SAMPLE_REPSTART =		10513,
	STATICTEXT_SAMPLE_REPLENGTH =		10514,
	STATICTEXT_SAMPLE_PLAYNOTE =		10515,
	
	BUTTON_MENU_ITEM_0 =				530,
	
	MAINMENU_PLAY_SONG =				(BUTTON_MENU_ITEM_0+12),
	MAINMENU_PLAY_PATTERN =				(BUTTON_MENU_ITEM_0+13),
	MAINMENU_STOP =						(BUTTON_MENU_ITEM_0+14),
	MAINMENU_ZAP =						(BUTTON_MENU_ITEM_0+0),
	MAINMENU_LOAD =						(BUTTON_MENU_ITEM_0+1),
	MAINMENU_SAVE =						(BUTTON_MENU_ITEM_0+2),
	MAINMENU_DISKMENU =					(BUTTON_MENU_ITEM_0+3),
	MAINMENU_INSEDIT =					(BUTTON_MENU_ITEM_0+5),
	MAINMENU_SMPEDIT =					(BUTTON_MENU_ITEM_0+4),
	MAINMENU_ADVEDIT =					(BUTTON_MENU_ITEM_0+6),
	MAINMENU_TRANSPOSE =				(BUTTON_MENU_ITEM_0+7),
	MAINMENU_ABOUT =					(BUTTON_MENU_ITEM_0+8),
	MAINMENU_OPTIMIZE =					(BUTTON_MENU_ITEM_0+9),
	MAINMENU_QUICKOPTIONS =				(BUTTON_MENU_ITEM_0+10),
	MAINMENU_CONFIG =					(BUTTON_MENU_ITEM_0+11),
	MAINMENU_PLAY_POSITION =			(BUTTON_MENU_ITEM_0+20),
	MAINMENU_SAVEAS =					(BUTTON_MENU_ITEM_0+21),
	MAINMENU_EDIT =						(BUTTON_MENU_ITEM_0+22),
	MAINMENU_HELP =						(BUTTON_MENU_ITEM_0+23),
	
	BUTTON_MENU_ITEM_ADDCHANNELS =		560,
	BUTTON_MENU_ITEM_SUBCHANNELS =		561,
	
	BUTTON_ABOUT_SHOWTITLE =			600,
	BUTTON_ABOUT_SHOWPEAK =				601,
	BUTTON_ABOUT_SHOWTIME =				602,
	BUTTON_ABOUT_FOLLOWSONG =			603,
	BUTTON_ABOUT_PROSPECTIVE =			604,
	BUTTON_ABOUT_WRAPCURSOR =			605,
	BUTTON_ABOUT_LIVESWITCH =			606,
	STATICTEXT_ABOUT_HEADING =			610,
	STATICTEXT_ABOUT_TIME =				611,
	BUTTON_ABOUT_ESTIMATESONGLENGTH =	612,
	
	BUTTON_TAB_CLOSE =					613,
	BUTTON_TAB_OPEN =					614,
	
	SCROLLBAR_0 =					1000,
	
	LISTBOX_SONGTITLE =				2000,
	LISTBOX_ORDERLIST =				2001,
	LISTBOX_INSTRUMENTS =			2002,
	LISTBOX_SAMPLES =				2003,
	
	CHECKBOX_0 =					2100,
	
	CONTAINER_0 =					3000,
	CONTAINER_ORDERLIST =			3001,
	CONTAINER_SPEED =    			3002,
	CONTAINER_PATTERN =				3003,
	CONTAINER_MENU =				3004,
	CONTAINER_ABOUT =				3005,
	CONTAINER_ENVELOPES =			3006,
	CONTAINER_INSTRUMENTLIST =		3007, // sample info
	CONTAINER_INSTRUMENTS_INFO1 =	3008, // vibrato info
	CONTAINER_INSTRUMENTS_INFO2 =	3009, // relative note number
	CONTAINER_INSTRUMENTS_INFO3 =	3010,
	CONTAINER_INSTRUMENTS_INFO4 =	3011,
	CONTAINER_INSTRUMENTS_INFO5 =	3012,
	CONTAINER_INSTRUMENTS_INFO6 =	3013,
	CONTAINER_PIANO =				3014,
	CONTAINER_SAMPLEEDITOR =		3015,
	CONTAINER_SAMPLE_PLAY =			3016,
	CONTAINER_SAMPLE_RANGE =		3017,
	CONTAINER_SAMPLE_EDIT1 =		3018,
	CONTAINER_SAMPLE_EDIT2 =		3019,
	CONTAINER_SAMPLE_EDIT3 =		3020,
	CONTAINER_SAMPLE_EDIT4 =		3021,
	CONTAINER_SAMPLE_EDIT5 =		3022,
	CONTAINER_SAMPLE_LOADSAVE =		3024,
	CONTAINER_SETTINGS =			3025,
	CONTAINER_TRANSPOSE =			3040,
	CONTAINER_ADVEDIT =				3041,
	CONTAINER_LOWRES_MENUSWITCH =	3042,
	CONTAINER_LOWRES_TINYMENU =		3043, // sample info
	CONTAINER_LOWRES_JAMMENU =		3044,
	CONTAINER_INPUTDEFAULT =		3045,
	CONTAINER_INPUTEXTENDED =		3046,
	CONTAINER_SAMPLE_ZOOMIN =		3047,
	CONTAINER_HDRECORDER =			3048,
	CONTAINER_OPENREMOVETABS =		3049,
	CONTAINER_ENTIREINSSECTION =	3050,
	CONTAINER_ENTIRESMPSECTION =	3051,
	
	PATTERN_EDITOR =				10000,
	PIANO_CONTROL =					10001,
	SAMPLE_EDITOR =					10002,
	PEAKLEVEL_CONTROL =				10003,
	SCOPES_CONTROL =				10004,
	TABHEADER_CONTROL =				10005,
	
	MESSAGEBOXZAP_CONTAINER =		20000,
	
	INSTRUMENT_CHOOSER_COPY =		21000,
	INSTRUMENT_CHOOSER_SWAP =		21001,
	INSTRUMENT_CHOOSER_LIST_SRC =	21101,
	INSTRUMENT_CHOOSER_LIST_DST =	21102,
	INSTRUMENT_CHOOSER_LIST_SRC2 =	21103,
	INSTRUMENT_CHOOSER_LIST_DST2 =	21104,
	INSTRUMENT_CHOOSER_LIST_SRC3 =	21105,
	INSTRUMENT_CHOOSER_LIST_DST3 =	21106,
	INSTRUMENT_CHOOSER_USERSTR1 =	21107,
	INSTRUMENT_CHOOSER_USERSTR2 =	21108,
	
	MESSAGEBOX_UNIVERSAL =			29999,
	MESSAGEBOX_CONVERTSAMPLE =		30000,
	MESSAGEBOX_CLEARSAMPLE =		30001,
	MESSAGEBOX_MINIMIZESAMPLE =		30002,
	MESSAGEBOX_CROPSAMPLE =			30003,
	MESSAGEBOX_INSREMAP =			30004,
	MESSAGEBOX_ZAPINSTRUMENT =		30005,
	MESSAGEBOX_REALLYQUIT =			30006,
	MESSAGEBOX_TRANSPOSEPROCEED =	30007,
	MESSAGEBOX_SAVEPROCEED =		30008,
	MESSAGEBOX_PANNINGSELECT =		30009
};

#endif
