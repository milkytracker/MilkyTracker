/*
 *  tracker/PatternEditorControl.h
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

/////////////////////////////////////////////////////////////////
//
//	PatternEditorTools control class
//
/////////////////////////////////////////////////////////////////
#ifndef PATTERNEDITORCONTROL__H
#define PATTERNEDITORCONTROL__H

#include "BasicTypes.h"
#include "Control.h"
#include "Event.h"
#include "PatternEditor.h"
#include "ScrollBar.h"
#include "PatternTools.h"
#include "EditModes.h"
#include "TrackerConfig.h"

// Forwards ------------------------------------------------------
class PPScrollbar;
class PPFont;
class PPContextMenu;
class PPDialogBase;

class PatternEditorControl;
typedef void (PatternEditorControl::*TPatternEditorKeyBindingHandler)();
typedef pp_int32 (PatternEditor::*TTransposeFunc)(const PatternEditorTools::TransposeParameters& transposeParameters, bool evaluate);

template<class Type> class PPKeyBindings;

class PatternEditorControl : 
	public PPControl, 
	public EventListenerInterface, 
	public EditorBase::EditorNotificationListener, 
	public PatternEditor::PatternAdvanceInterface
{
private:
	struct SongPos
	{
		pp_int32 orderListIndex, row;
	};

	struct UndoInfo
	{
		pp_int32 startIndex;
		pp_int32 startPos;
		
		UndoInfo() :
			startIndex(),
			startPos()
		{
		}
		
		UndoInfo(pp_int32 startIndex, pp_int32 startPos) :
			startIndex(startIndex), 
			startPos(startPos)
		{
		}
	} undoInfo;

	PPColor bgColor;
	const PPColor* borderColor;
	const PPColor* cursorColor;
	const PPColor* selectionColor;

	struct Properties
	{
		bool showFocus;

		bool rowAdvance;
		pp_int32 rowInsertAdd;

		pp_uint32 spacing, highlightSpacingPrimary, highlightSpacingSecondary;
		bool highLightRowPrimary, highLightRowSecondary;
		bool hexCount;
		bool wrapAround;
		bool advancedDnd;
		bool prospective;
		bool tabToNote;
		bool clickToCursor;
		bool multiChannelEdit;
		ScrollModes scrollMode;
		bool invertMouseVscroll;
		pp_uint32 muteFade;
		char zeroEffectCharacter;
		bool ptNoteLimit;
		
		Properties() :
			showFocus(true), 
			rowAdvance(true),
			rowInsertAdd(1),
			spacing(0), 
			highlightSpacingPrimary(4), 
			highlightSpacingSecondary(8),
			highLightRowPrimary(false), 
			highLightRowSecondary(false),
			hexCount(true), 
			wrapAround(true),
			advancedDnd(false),
			prospective(false), 
			tabToNote(true), 
			clickToCursor(true), 
			multiChannelEdit(false),
			scrollMode(ScrollModeToEnd),
			invertMouseVscroll(false),
			muteFade(32768),
			zeroEffectCharacter('\xf4'),
			ptNoteLimit(false)
		{
		}
	} properties;
	
	PPFont* font;
		
	PPScrollbar* hTopScrollbar;
	PPScrollbar* hBottomScrollbar;
	PPScrollbar* vLeftScrollbar;
	PPScrollbar* vRightScrollbar;

	PPControl* caughtControl;
	bool controlCaughtByLMouseButton, controlCaughtByRMouseButton;

	PatternEditor* patternEditor;
	XModule* module;
	TXMPattern* pattern;
	pp_int32 currentOrderlistIndex;
	SongPos songPos;

	PatternTools patternTools;

	pp_int32 startIndex;
	pp_int32 startPos;

	pp_int32 visibleWidth;
	pp_int32 visibleHeight;
	pp_int32 slotSize;

	pp_uint8 muteChannels[TrackerConfig::MAXCHANNELS];
	pp_uint8 recChannels[TrackerConfig::MAXCHANNELS];

	// Cursor position within editor
	pp_int32 cursorPositions[9];
	pp_int32 cursorSizes[8];
	
	PatternEditorTools::Position cursorCopy, preCursor, *ppreCursor;
	
	bool startSelection;
	bool keyboardStartSelection;
	bool assureUpdate, assureCursor;
	
	pp_int32 selectionTicker;

	bool hasDragged;
	
	bool moveSelection;
	PatternEditorTools::Position moveSelectionInitialPos;
	PatternEditorTools::Position moveSelectionFinalPos;

	// edit menu
	pp_int32 menuPosX;
	pp_int32 menuPosY;
	pp_int32 menuInvokeChannel;
	pp_int32 lastMenuInvokeChannel;

	PPContextMenu* editMenuControl;
	PPContextMenu* moduleMenuControl;
	PPContextMenu* instrumentMenuControl;
	PPContextMenu* channelMenuControl;

	// Keyboard shortcuts
	PPKeyBindings<TPatternEditorKeyBindingHandler>* eventKeyDownBindings;
	PPKeyBindings<TPatternEditorKeyBindingHandler>* scanCodeBindings;

	PPKeyBindings<TPatternEditorKeyBindingHandler>* eventKeyDownBindingsMilkyTracker;
	PPKeyBindings<TPatternEditorKeyBindingHandler>* scanCodeBindingsMilkyTracker;
	PPKeyBindings<TPatternEditorKeyBindingHandler>* eventKeyDownBindingsFastTracker;
	PPKeyBindings<TPatternEditorKeyBindingHandler>* scanCodeBindingsFastTracker;

	// Edit mode
	EditModes editMode;
	pp_int32 selectionKeyModifier;
	
public:
	PatternEditorControl(pp_int32 id, PPScreen* parentScreen, EventListenerInterface* eventListener, 
						 const PPPoint& location, const PPSize& size, 
						 bool border = true);
	
	virtual ~PatternEditorControl();
	
	void setColor(PPColor color) { bgColor = color; }
	void setFont(PPFont* font);

	void setShowFocus(bool showFocus) { properties.showFocus = showFocus; }
	void setScrollMode(ScrollModes mode) { properties.scrollMode = mode; adjustScrollBarPositionsAndSizes(); assureCursorVisible(); }
	void setInvertMouseVscroll(bool invert) { properties.invertMouseVscroll = invert; }

	// from PPControl
	virtual void setSize(const PPSize& size);
	virtual void setLocation(const PPPoint& location);
	virtual void paint(PPGraphicsAbstract* graphics);
	virtual bool gainsFocus() const { return true; }
	virtual bool gainedFocusByMouse() const { return caughtControl == NULL; }
	virtual pp_int32 dispatchEvent(PPEvent* event);
	
	// from EventListenerInterface
	pp_int32 handleEvent(PPObject* sender, PPEvent* event);

	void attachPatternEditor(PatternEditor* patternEditor);

	void reset();
	
	bool isDraggingVertical() const 
	{ 
		return (caughtControl == vLeftScrollbar) || (caughtControl == vRightScrollbar); 
	}

	// set number of visible channels, if this is -1 it will dynamically adjust it
	void setNumVisibleChannels(pp_int32 numChannels);

	pp_int32 getRowInsertAdd() { return properties.rowInsertAdd; }
	void setRowInsertAdd(pp_int32 rowInsertAdd) { properties.rowInsertAdd = rowInsertAdd; }
	
	void increaseRowInsertAdd() { properties.rowInsertAdd = (properties.rowInsertAdd+1) % 17; }
	void decreaseRowInsertAdd() { properties.rowInsertAdd--; if (properties.rowInsertAdd == -1) properties.rowInsertAdd = 16; }
	
	void setOrderlistIndex(pp_int32 currentOrderlistIndex) { this->currentOrderlistIndex = currentOrderlistIndex; }
	void setRow(pp_int32 row, bool bAssureCursorVisible = true);
	pp_int32 getRow() { return patternEditor->getCursor().row; }
	void setSongPosition(pp_int32 currentOrderlistIndex, pp_int32 row) { songPos.row = row; songPos.orderListIndex = currentOrderlistIndex; }
	void getSongPosition(pp_int32& currentOrderlistIndex, pp_int32& row) { row = songPos.row; currentOrderlistIndex = songPos.orderListIndex; }
	void setChannel(pp_int32 chn, pp_int32 posInner);

	pp_int32 getCurrentChannel() const { return patternEditor->getCursor().channel; }
	pp_int32 getCurrentRow() const { return patternEditor->getCursor().row; }
	pp_int32 getCursorPosInner() const { return patternEditor->getCursor().inner; }

	pp_int32 ScanCodeToNote(pp_int16 keyCode);

	void setCurrentInstrument(pp_int32 ins) { patternEditor->setCurrentInstrument(ins); }
	void enableInstrument(bool b) { patternEditor->enableInstrument(b); }
	bool isInstrumentEnabled() const { return patternEditor->isInstrumentEnabled(); }
	void setInstrumentBackTrace(bool b) { patternEditor->setInstrumentBackTrace(b); }

	bool hasValidSelection() const;

	//////////////////////////////////////////////////////////////////////////
	// --- more flags coming
	//////////////////////////////////////////////////////////////////////////
	void setSpacing(pp_uint32 spacing) { properties.spacing = spacing; }
	void setHighlightSpacingPrimary(pp_uint32 spacing) { properties.highlightSpacingPrimary = spacing; }
	void setHighLightRowPrimary(bool b) { properties.highLightRowPrimary = b; }
	void setHighlightSpacingSecondary(pp_uint32 spacing) { properties.highlightSpacingSecondary = spacing; }
	void setHighLightRowSecondary(bool b) { properties.highLightRowSecondary = b; }
	// use zero effect zeros
	void showZeroEffect(bool b) { properties.zeroEffectCharacter = (b ? '0' : '\xf4'); }
	// hex count
	void setHexCount(bool b) { properties.hexCount = b; }
	// set prospective mode
	void setProspective(bool b) { properties.prospective = b; }
	bool getProspective() const { return properties.prospective; }
	// set wraparound mode
	void setWrapAround(bool b) { properties.wrapAround = b; }
	bool getWrapAround() const { return properties.wrapAround; }
	// set advanced drag and drop mode
	void setAdvancedDnd(bool b) { properties.advancedDnd = b; }
	bool getAdvancedDnd() const { return properties.advancedDnd; }
	// set tab to note
	void setTabToNote(bool b) { properties.tabToNote = b; }
	bool getTabToNote() const { return properties.tabToNote; }
	// mouse click allows cursor-repositioning
	void setClickToCursor(bool b) { properties.clickToCursor = b; }
	bool getClickToCursor() const { return properties.clickToCursor; }
	// autoresize
	void setAutoResize(bool b) { patternEditor->setAutoResize(b); }
	bool getAutoResize() { return patternEditor->getAutoResize(); }
	// multichannel edit
	void setMultiChannelEdit(bool b) { properties.multiChannelEdit = b; }
	// Highlight notes outside the PT 3 octave limit
	void setPtNoteLimit(bool l) { properties.ptNoteLimit = l; }

	void setRowAdvance(bool b) { properties.rowAdvance = b; }
	
	void switchEditMode(EditModes mode);

	void setMuteFade(pp_int32 fade) { if (fade > 65536) fade = 65536; if (fade < 0) fade = 0; properties.muteFade = fade; }
	
	void muteChannel(pp_int32 index, bool b) { muteChannels[index] = (b ? 1 : 0); }
	void recordChannel(pp_int32 index, bool b) { recChannels[index] = (b ? 1 : 0); }
	void unmuteAll();	
	
	void setRecordMode(bool b);
	
	void advanceRow(bool assureCursor = true, bool repaint = true);

	PatternEditor* getPatternEditor() const { return patternEditor; }

	// --- these are defined in PatternEditorControlTransposeHandler.cpp -----
	void showNoteTransposeWarningMessageBox(pp_int32 fuckups);
	pp_int32 noteTransposeTrack(const PatternEditorTools::TransposeParameters& transposeParameters);
	pp_int32 noteTransposePattern(const PatternEditorTools::TransposeParameters& transposeParameters);
	pp_int32 noteTransposeSelection(const PatternEditorTools::TransposeParameters& transposeParameters);	
	
private:
	// --- Transpose handler
	class TransposeHandlerResponder : public DialogResponder
	{
	private:
		PatternEditorControl& patternEditorControl;
		PatternEditorTools::TransposeParameters transposeParameters;
		TTransposeFunc transposeFunc;
		
	public:
		TransposeHandlerResponder(PatternEditorControl& thePatternEditorControl);
		
		void setTransposeParameters(const PatternEditorTools::TransposeParameters& transposeParameters) { this->transposeParameters = transposeParameters; }
		void setTransposeFunc(TTransposeFunc transposeFunc) { this->transposeFunc = transposeFunc; }
		
		virtual pp_int32 ActionOkay(PPObject* sender);
		virtual pp_int32 ActionCancel(PPObject* sender);
	};

	friend class TransposeHandlerResponder;

	PPDialogBase* dialog;
	TransposeHandlerResponder* transposeHandlerResponder;

private:
	pp_int32 getRowCountWidth();

	void adjustExtents();

	void adjustVerticalScrollBarPositions(mp_sint32 startIndex);
	void adjustHorizontalScrollBarPositions(mp_sint32 startPos);

	void adjustScrollBarPositionsAndSizes();
	void adjustScrollBarSizes();

	void setScrollbarPositions(mp_sint32 startIndex, mp_sint32 startPos);

	void scrollCursorDown();
	void scrollCursorUp();

	void assureCursorVisible(bool row = true, bool channel = true);

	mp_sint32 getNextRecordingChannel(mp_sint32 currentChannel);

	virtual void advance();

	void validate();

	// ------- menu stuff ------------------------------------------ 
	enum MenuCommandIDs
	{
		MenuCommandIDMuteChannel = 100,
		MenuCommandIDSoloChannel,
		MenuCommandIDUnmuteAll,
		MenuCommandIDSelectChannel,
		MenuCommandIDPorousPaste,
		MenuCommandIDSwapChannels,
		MenuCommandIDChannelAdd,
		MenuCommandIDChannelDelete,
		MenuCommandIDModuleLoad,
		MenuCommandIDModuleSave,
		MenuCommandIDModuleSaveAs

	};
	
	void invokeMenu(pp_int32 channel, const PPPoint& p);
	
	void executeMenuCommand(pp_int32 commandId);
	
	void handleDeleteKey(pp_uint16 keyCode, pp_int32& result);
	void handleKeyChar(pp_uint8 character);
	void handleKeyDown(pp_uint16 keyCode, pp_uint16 scanCode, pp_uint16 character);

	void selectionModifierKeyDown();
	void selectionModifierKeyUp();

	// mark channel
	void markChannel(pp_int32 channel, bool invert = true);
	
	// select ALL
	void selectAll();

	// deselect ALL
	void deselectAll();

	enum RMouseDownActions
	{
		RMouseDownActionInvalid,
		RMouseDownActionFirstRClick,
		RMouseDownActionFirstLClick,
		RMouseDownActionSecondLClick
	};
	
	RMouseDownActions lastAction;
	pp_int32 RMouseDownInChannelHeading;
	
	pp_int32 pointInChannelHeading(PPPoint& cp);
	bool isSoloChannel(pp_int32 c);
	
private:
	bool executeBinding(const PPKeyBindings<TPatternEditorKeyBindingHandler>* bindings, pp_uint16 keyCode);
	void initKeyBindings();

	// Original FT2 bindings
	void eventKeyDownBinding_LEFT();
	void eventKeyDownBinding_RIGHT();
	void eventKeyDownBinding_UP();
	void eventKeyDownBinding_DOWN();
	void eventKeyDownBinding_PRIOR();
	void eventKeyDownBinding_NEXT();
	void eventKeyDownBinding_HOME();
	void eventKeyDownBinding_END();
	
	void eventKeyDownBinding_FIRSTQUARTER();	
	void eventKeyDownBinding_SECONDQUARTER();	
	void eventKeyDownBinding_THIRDQUARTER();	

	void eventKeyDownBinding_ReadMacro1();
	void eventKeyDownBinding_ReadMacro2();
	void eventKeyDownBinding_ReadMacro3();
	void eventKeyDownBinding_ReadMacro4();
	void eventKeyDownBinding_ReadMacro5();
	void eventKeyDownBinding_ReadMacro6();
	void eventKeyDownBinding_ReadMacro7();
	void eventKeyDownBinding_ReadMacro8();
	void eventKeyDownBinding_ReadMacro9();
	void eventKeyDownBinding_ReadMacro0();

	void eventKeyDownBinding_WriteMacro1();
	void eventKeyDownBinding_WriteMacro2();
	void eventKeyDownBinding_WriteMacro3();
	void eventKeyDownBinding_WriteMacro4();
	void eventKeyDownBinding_WriteMacro5();
	void eventKeyDownBinding_WriteMacro6();
	void eventKeyDownBinding_WriteMacro7();
	void eventKeyDownBinding_WriteMacro8();
	void eventKeyDownBinding_WriteMacro9();
	void eventKeyDownBinding_WriteMacro0();

	void eventKeyDownBinding_SC_Q();
	void eventKeyDownBinding_SC_W();
	void eventKeyDownBinding_SC_E();
	void eventKeyDownBinding_SC_R();
	void eventKeyDownBinding_SC_T();
	void eventKeyDownBinding_SC_Z();
	void eventKeyDownBinding_SC_U();
	void eventKeyDownBinding_SC_I();

	void eventKeyDownBinding_SC_A();
	void eventKeyDownBinding_SC_S();
	void eventKeyDownBinding_SC_D();
	void eventKeyDownBinding_SC_F();
	void eventKeyDownBinding_SC_G();
	void eventKeyDownBinding_SC_H();
	void eventKeyDownBinding_SC_J();
	void eventKeyDownBinding_SC_K();

	void eventKeyDownBinding_SC_IncreaseRowInsertAdd();
	void eventKeyDownBinding_SC_DecreaseRowInsertAdd();
	
	void eventKeyDownBinding_PreviousChannel();
	void eventKeyDownBinding_NextChannel();

	void eventKeyDownBinding_DeleteNote();
	void eventKeyDownBinding_DeleteNoteVolumeAndEffect();
	void eventKeyDownBinding_DeleteVolumeAndEffect();
	void eventKeyDownBinding_DeleteEffect();

	void eventKeyDownBinding_InsertNote();
	void eventKeyDownBinding_InsertLine();
	void eventKeyDownBinding_DeleteNoteSlot();
	void eventKeyDownBinding_DeleteLine();

	void eventKeyDownBinding_InsIncSelection();
	void eventKeyDownBinding_InsDecSelection();
	void eventKeyDownBinding_InsIncTrack();
	void eventKeyDownBinding_InsDecTrack();
	
	void eventKeyDownBinding_CutTrack();
	void eventKeyDownBinding_CopyTrack();
	void eventKeyDownBinding_PasteTrack();
	void eventKeyDownBinding_TransparentPasteTrack();

	void eventKeyDownBinding_CutPattern();
	void eventKeyDownBinding_CopyPattern();
	void eventKeyDownBinding_PastePattern();
	void eventKeyDownBinding_TransparentPastePattern();

	void eventKeyCharBinding_Undo();
	void eventKeyCharBinding_Redo();
	void eventKeyCharBinding_Cut();		// Operates on block
	void eventKeyCharBinding_Copy();	// Operates on block
	void eventKeyCharBinding_Paste();   // Operates on block
	void eventKeyCharBinding_TransparentPaste();
	void eventKeyCharBinding_SelectAll();
	void eventKeyCharBinding_MuteChannel();
	void eventKeyCharBinding_InvertMuting();
	void eventKeyCharBinding_Interpolate();

public:
	enum AdvanceCodes
	{
		AdvanceCodeJustUpdate,
		AdvanceCodeCursorUpWrappedStart,
		AdvanceCodeCursorDownWrappedEnd,
		AdvanceCodeCursorPageUpWrappedStart,
		AdvanceCodeCursorPageDownWrappedEnd,
		AdvanceCodeWrappedEnd,
		AdvanceCodeSelectNewRow
	};
	
private:
	virtual void editorNotification(EditorBase* sender, EditorBase::EditorNotifications notification);

	pp_int32 notifyUpdate(pp_int32 code = AdvanceCodeJustUpdate)
	{
		PPEvent e(eUpdated, &code, sizeof(code));						
		return eventListener->handleEvent(reinterpret_cast<PPObject*>(this), &e);
	}
};

#endif
