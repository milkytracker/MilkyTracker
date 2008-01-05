#ifndef TRACKERCONFIG__H
#define TRACKERCONFIG__H

#include "BasicTypes.h"

#define NUMEFFECTMACROS 20

class TrackerConfig
{
public:
	static const PPString stringButtonPlus;
	static const PPString stringButtonMinus;
	static const PPString stringButtonUp;
	static const PPString stringButtonDown;
	static const PPString stringButtonExtended;
	static const PPString stringButtonCollapsed;
	
	enum 
	{
		MAXCHANNELS = 256
	};

	static const PPPoint trackerExitBounds;

	static PPColor colorThemeMain;
	static PPColor colorRecordModeButtonText;
	
	// Pattern colors
	static PPColor colorPatternEditorBackground;
	static PPColor colorPatternEditorCursor;
	static PPColor colorPatternEditorCursorLine;
	static PPColor colorPatternEditorCursorLineHighLight;
	static PPColor colorPatternEditorSelection;
	
	static PPColor colorPatternEditorNote;
	static PPColor colorPatternEditorInstrument;
	static PPColor colorPatternEditorVolume;
	static PPColor colorPatternEditorEffect;
	static PPColor colorPatternEditorOperand;
	
	static PPColor colorHighLight_1;
	static PPColor colorHighLight_2;
	static PPColor colorScopes;
	static PPColor colorRowHighLight_1;
	static PPColor colorRowHighLight_2;
	
	static pp_int32 numTabs;
	
	static pp_int32 numPlayerChannels;
	static pp_int32 numVirtualChannels;
	static pp_int32 totalPlayerChannels;
	static const pp_int32 maximumPlayerChannels;

	static bool useVirtualChannels;
	
	static const pp_int32 numPredefinedEnvelopes;
	static const pp_int32 numPredefinedColorPalettes;
	
	static const PPString defaultPredefinedVolumeEnvelope;
	static const PPString defaultPredefinedPanningEnvelope;
	static const PPString defaultProTrackerPanning;
	
	static const PPString defaultColorPalette;	
	static const char* predefinedColorPalettes[];
	
	static const PPSystemString untitledSong;
	
	static const pp_int32 numMixFrequencies;
	static const pp_int32 mixFrequencies[];

	static const pp_uint32 version;
	
	enum ModuleExtensions
	{
		ModuleExtension669,
		ModuleExtensionAMF,
		ModuleExtensionAMS,
		ModuleExtensionCBA,
		ModuleExtensionDBM,
		ModuleExtensionDIGI,
		ModuleExtensionDSM,
		ModuleExtensionDTM,
		ModuleExtensionFAR,
		ModuleExtensionGDM,
		ModuleExtensionGMC,
		ModuleExtensionIMF,
		ModuleExtensionIT,
		ModuleExtensionMDL,
		ModuleExtensionMOD,
		ModuleExtensionMTM,
		ModuleExtensionMXM,
		ModuleExtensionOKT,
		ModuleExtensionPLM,
		ModuleExtensionPSM,
		ModuleExtensionPTM,
		ModuleExtensionS3M,
		ModuleExtensionSTM,
		ModuleExtensionULT,
		ModuleExtensionUNI,
		ModuleExtensionXM,
		// compressed/archived modules
		ModuleExtensionPP,
		ModuleExtensionUMX,
	};	
	static const char* moduleExtensions[];
	static const char* getModuleExtension(ModuleExtensions extension) { return moduleExtensions[extension*2]; }
	static const char* getModuleDescription(ModuleExtensions extension) { return moduleExtensions[extension*2+1]; }
	
	enum InstrumentExtensions
	{
		InstrumentExtensionXI,
		InstrumentExtensionPAT
	};
	static const char* instrumentExtensions[];
	static const char* getInstrumentExtension(InstrumentExtensions extension) { return instrumentExtensions[extension*2]; }
	static const char* getInstrumentDescription(InstrumentExtensions extension) { return instrumentExtensions[extension*2+1]; }
	
	enum SampleExtensions
	{
		SampleExtensionWAV,
		SampleExtensionIFF,
		SampleExtension8SVX,
		SampleExtensionAIF,
		SampleExtensionAIFF
	};
	static const char* sampleExtensions[];
	static const char* getSampleExtension(SampleExtensions extension) { return sampleExtensions[extension*2]; }
	static const char* getSampleDescription(SampleExtensions extension) { return sampleExtensions[extension*2+1]; }
	
	enum PatternExtensions
	{
		PatternExtensionXP
	};
	static const char* patternExtensions[];
	static const char* getPatternExtension(PatternExtensions extension) { return patternExtensions[extension*2]; }
	static const char* getPatternDescription(PatternExtensions extension) { return patternExtensions[extension*2+1]; }
	
	enum TrackExtensions
	{
		TrackExtensionXT
	};
	static const char* trackExtensions[];
	static const char* getTrackExtension(TrackExtensions extension) { return trackExtensions[extension*2]; }
	static const char* getTrackDescription(TrackExtensions extension) { return trackExtensions[extension*2+1]; }
};

#endif
