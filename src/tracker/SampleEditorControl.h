#ifndef SAMPLEEDITORCONTROL__H
#define SAMPLEEDITORCONTROL__H

#include "BasicTypes.h"
#include "Control.h"
#include "Event.h"
#include "SampleEditor.h"

// Forwards
class PPGraphicsAbstract;
class PPFont;
class PPScrollbar;
class PPControl;
class PPContextMenu;
class FilterParameters;
class RespondMessageBox;

class SampleEditorControl : public PPControl, public EventListenerInterface, public EditorBase::EditorNotificationListener
{
public:
	enum OffsetFormats
	{
		OffsetFormatHex,
		OffsetFormatDec,
		OffsetFormatMillis
	};

private:
	PPColor backgroundColor;

	bool border;
	PPColor ourOwnBorderColor;
	const PPColor* borderColor;

	PPScrollbar* hScrollbar;	

	PPControl* caughtControl;
	bool controlCaughtByLMouseButton, controlCaughtByRMouseButton;

	PPContextMenu* editMenuControl;
	PPContextMenu* subMenuAdvanced;
	PPContextMenu* subMenuGenerators;
	PPContextMenu* subMenuPT;

	// extent
	pp_int32 selectionStartNew, selectionEndNew;
	pp_int32 selectionDragPivot;
	pp_int32 selectionStartCopy, selectionEndCopy;

	pp_int32 currentRepeatStart, currentRepeatLength;
	
	struct ShowMark
	{
		pp_int32 pos;
		pp_int32 intensity;
		pp_int32 panning;
	} *showMarks;

	float xScale;
	float minScale;

	pp_int32 startPos;
	pp_int32 visibleWidth;
	pp_int32 visibleHeight;
	pp_int32 scrollDist;

	pp_int32 selecting;
	pp_int32 resizing;
	bool drawMode;

	bool hasDragged;

	// selection
	pp_int32 selectionTicker;
	
	// necessary for controlling
	SampleEditor* sampleEditor;
	
	pp_int32 relativeNote;
	OffsetFormats offsetFormat;

	mp_sint32 getVisibleLength();
	
	float calcScale(mp_sint32 len);
	float calcScale();
	
protected:
	virtual void translateCoordinates(PPPoint& cp) 
	{
		PPControl::translateCoordinates(cp);
		cp.x -= 1;
		cp.y -= 1;
	}

public:
	SampleEditorControl(pp_int32 id, 
				 PPScreen* parentScreen, 
				 EventListenerInterface* eventListener, 
				 PPPoint location, 
				 PPSize size, 
				 bool border = true);

	~SampleEditorControl();

	void setBackgroundColor(const PPColor& color) { backgroundColor = color; }
	void setBorderColor(const PPColor& color) { borderColor = &color; }

	static void formatMillis(char* buffer, pp_uint32 millis);

	// from PPControl
	virtual void paint(PPGraphicsAbstract* graphics);	
	virtual bool gainsFocus() { return false; }
	virtual bool isActive() { return true; }	
	virtual pp_int32 callEventListener(PPEvent* event);
	virtual pp_int32 handleEvent(PPObject* sender, PPEvent* event);

	virtual void setSize(PPSize size);
	virtual void setLocation(PPPoint location);

public:
	// controlling editor from outside
	void attachSampleEditor(SampleEditor* sampleEditor);	
	
	void setDrawMode(bool b) { drawMode = b; }
	bool getDrawMode() const { return drawMode; }

	void setOffsetFormat(OffsetFormats offsetFormat) { this->offsetFormat = offsetFormat; }
	OffsetFormats getOffsetFormat() const { return offsetFormat; }
	
	void setRelativeNote(pp_int32 relativeNote) { this->relativeNote = relativeNote; }
	pp_int32 getRelativeNote() const { return relativeNote; }
	
	// manipulating range (selection)
	void showRange();
	void rangeAll(bool updateNotify = false);
	void rangeClear(bool updateNotify = false);

	void increaseRangeStart();
	void decreaseRangeStart();

	void increaseRangeEnd();
	void decreaseRangeEnd();
	
	bool canZoomOut();
	void zoomOut(float factor = 2.0f, pp_int32 center = -1);
	
	void zoomIn(float factor = 0.5f, pp_int32 center = -1);
	
	void scrollWheelZoomOut(const PPPoint* p = NULL)
	{
		if (p)
			zoomOut(1.25f, positionToSample(*p));
		else
			zoomOut(1.25f);
			
		notifyUpdate();
	}

	void scrollWheelZoomIn(const PPPoint* p = NULL)
	{
		if (p)
			zoomIn(0.75f, positionToSample(*p));
		else
			zoomIn(0.75f);

		notifyUpdate();
	}
	
	void showAll();

	pp_int32 getCurrentPosition();
	pp_int32 getCurrentRangeLength() { return sampleEditor->getSelectionLength(); }
	pp_int32 getCurrentDisplayRange(); 

	pp_int32 getSelectionStart() const { return sampleEditor->getLogicalSelectionStart(); }
	pp_int32 getSelectionEnd() const { return sampleEditor->getLogicalSelectionEnd(); }

	pp_uint32 getRepeatStart() const;
	pp_uint32 getRepeatLength() const;

	bool hasValidSelection();

	void setShowMark(pp_int32 c, pp_int32 m, pp_int32 i = 256, pp_int32 p = 128) { showMarks[c].pos = m; showMarks[c].intensity = i; showMarks[c].panning = p; }
	pp_int32 getShowMarkPos(pp_int32 c) { return showMarks[c].pos; }
	bool showMarksVisible();

	void reset();
	
	SampleEditor* getSampleEditor() { return sampleEditor; }

private:
	void validate(bool repositionBars = true, bool rescaleBars = false);

	pp_int32 getMaxWidth();

	void adjustScrollbars();

	pp_int32 positionToSample(PPPoint cp);
	void drawSample(const PPPoint& p);

	void drawLoopMarker(PPGraphicsAbstract* g, pp_int32 x, pp_int32 y, bool down, const pp_int32 size);

	bool hitsLoopstart(const PPPoint* p);
	bool hitsLoopend(const PPPoint* p);

	void startMarkerDragging(const PPPoint* p);
	void endMarkerDragging();

	void signalWaitState(bool b);

	void notifyUpdate()
	{
		PPEvent e(eUpdated);						
		eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
	}

	// -- Multilevel UNDO / REDO ----------------------------------
	// undo/redo information
	struct UndoInfo
	{
		float xScale;
		float minScale;
		pp_int32 startPos;
		pp_int32 barPos; 
		pp_int32 barScale;
		
		UndoInfo()
		{
		}

		UndoInfo(float xScale, 
				 float minScale,
				 pp_int32 startPos,
				 pp_int32 barPos = -1, 
				 pp_int32 barScale = -1) :
			xScale(xScale),
			minScale(minScale),
			startPos(startPos),
			barPos(barPos),
			barScale(barScale)
		{
		}
	} undoInfo;

	enum MenuCommandIDs
	{
		MenuCommandIDCrop = 99,
		MenuCommandIDMixPaste,
		MenuCommandIDNormalize,
		MenuCommandIDVolumeBoost,
		MenuCommandIDVolumeFade,
		MenuCommandIDReverse,
		MenuCommandIDPTBoost,
		MenuCommandIDXFade,
		MenuCommandIDChangeSign,
		MenuCommandIDSwapByteOrder,
		MenuCommandIDResample,
		MenuCommandIDDCNormalize,
		MenuCommandIDDCOffset,
		MenuCommandIDRectangularSmooth,
		MenuCommandIDTriangularSmooth,
		MenuCommandIDEQ3Band,
		MenuCommandIDEQ10Band,
		MenuCommandIDGenerateSilence,
		MenuCommandIDGenerateNoise,
		MenuCommandIDGenerateSine,
		MenuCommandIDGenerateSquare,
		MenuCommandIDGenerateTriangle,
		MenuCommandIDGenerateSawtooth
	};
	
	void executeMenuCommand(pp_int32 commandId);

	virtual void editorNotification(EditorBase* sender, EditorBase::EditorNotifications notification);
	
public:
	void invokeSetSampleVolume() { executeMenuCommand(MenuCommandIDVolumeBoost); }
	
	bool contextMenuVisible();
	void invokeContextMenu(const PPPoint& p, bool translatePoint = true);
	void hideContextMenu();
	
	// --- Sample tool responder
private:
	class ToolHandlerResponder : public RespondListenerInterface
	{
	public:
		enum SampleToolTypes
		{
			SampleToolTypeNone,
			SampleToolTypeNew,
			SampleToolTypeVolume,
			SampleToolTypeFade,
			SampleToolTypeNormalize,
			SampleToolTypeReverse,
			SampleToolTypePTBoost,
			SampleToolTypeXFade,
			SampleToolTypeChangeSign,
			SampleToolTypeSwapByteOrder,
			SampleToolTypeResample,
			SampleToolTypeDCNormalize,
			SampleToolTypeDCOffset,
			SampleToolTypeRectangularSmooth,
			SampleToolTypeTriangularSmooth,
			SampleToolTypeEQ3Band,
			SampleToolTypeEQ10Band,
			SampleToolTypeGenerateSilence,
			SampleToolTypeGenerateNoise,
			SampleToolTypeGenerateSine,
			SampleToolTypeGenerateSquare,
			SampleToolTypeGenerateTriangle,
			SampleToolTypeGenerateSawtooth
		};
	
	private:
		SampleEditorControl& sampleEditorControl;
		SampleToolTypes sampleToolType;
		
	public:
		ToolHandlerResponder(SampleEditorControl& theSampleEditorControl);

		void setSampleToolType(SampleToolTypes type) { sampleToolType = type; }
		SampleToolTypes getSampleToolType() { return sampleToolType; }
				
		virtual pp_int32 ActionOkay(PPObject* sender);
		virtual pp_int32 ActionCancel(PPObject* sender);
	};

	friend class ToolHandlerResponder;

	RespondMessageBox* respondMessageBox;
	ToolHandlerResponder* toolHandlerResponder;
	
	bool invokeToolParameterDialog(ToolHandlerResponder::SampleToolTypes type);
	bool invokeTool(ToolHandlerResponder::SampleToolTypes type);

	// Last values
	struct TLastValues
	{
		pp_int32 newSampleSize;
		float boostSampleVolume;
		float fadeSampleVolumeStart;
		float fadeSampleVolumeEnd;
		float DCOffset;
		pp_int32 silenceSize;
		float waveFormVolume;
		float waveFormNumPeriods;
		
		bool hasEQ3BandValues;
		float EQ3BandValues[3];
		
		bool hasEQ10BandValues;
		float EQ10BandValues[10];
		
		static float invalidFloatValue() 
		{
			return -12345678.0f;
		}

		static int invalidIntValue() 
		{
			return -12345678;
		}
	
		void reset()
		{
			newSampleSize = invalidIntValue();
			boostSampleVolume = invalidFloatValue();
			fadeSampleVolumeStart = invalidFloatValue();
			fadeSampleVolumeEnd = invalidFloatValue();
			DCOffset = invalidFloatValue();
			silenceSize = invalidIntValue();
			waveFormVolume = invalidFloatValue();
			waveFormNumPeriods = invalidFloatValue();
			hasEQ3BandValues = hasEQ10BandValues = false;
		}
	};
	
	TLastValues lastValues;
	
	void resetLastValues()
	{
		lastValues.reset();
	}	
};

#endif
