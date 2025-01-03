void PatternEditorControl::paintSteps(PPGraphicsAbstract* g)
{
	if (!isVisible())
		return;

	// ;----------------- everything all right?
	validate();

	// ;----------------- colors
	PPColor lineColor;

	if (hasFocus || !properties.showFocus)
		lineColor = *cursorColor;
	else
		lineColor.r = lineColor.g = lineColor.b = 64;

	PPColor bColor = *borderColor, dColor = *borderColor, bCursor = lineColor, dCursor = lineColor;
	// adjust dark color
	dColor.scaleFixed(32768);
	// adjust bright color
	bColor.scaleFixed(87163);
	// adjust dark color
	dCursor.scaleFixed(32768);
	// adjust bright color
	bCursor.scaleFixed(87163);

	g->setRect(location.x+ SCROLLBARWIDTH, location.y+SCROLLBARWIDTH, 
			location.x + size.width - SCROLLBARWIDTH, location.y + size.height - SCROLLBARWIDTH);

	g->setColor(bgColor);

	g->fill();

	g->setFont(font);

	// ;----------------- not going any further with invalid pattern
	if (pattern == NULL)
		return;

	// ;----------------- make layout extents
	adjustExtents();

	char name[32];
	char label[32];

	mp_sint32 i,j;

	// ;----------------- selection layout
	PatternEditorTools::Position selectionStart, selectionEnd;
	selectionStart = patternEditor->getSelection().start;
	selectionEnd = patternEditor->getSelection().end;

	PatternEditorTools::flattenSelection(selectionStart, selectionEnd);	

	// only entire instrument column is allowed
	if (selectionStart.inner >= 1 && selectionStart.inner<=2)
		selectionStart.inner = 1;
	if (selectionEnd.inner >= 1 && selectionEnd.inner<=2)
		selectionEnd.inner = 2;
	// only entire volume column is allowed
	if (selectionStart.inner >= 3 && selectionStart.inner<=4)
		selectionStart.inner = 3;
	if (selectionEnd.inner >= 3 && selectionEnd.inner<=4)
		selectionEnd.inner = 4;

	PatternEditorTools::Position& cursor = patternEditor->getCursor();

	if (cursor.inner < 0 || cursor.inner >= 8)
		cursor.inner = 0;

	// ;----------------- some constants
	const pp_uint32 fontCharWidth3x = font->getCharWidth()*3 + 1;
	const pp_uint32 fontCharWidth2x = font->getCharWidth()*2 + 1;
	const pp_uint32 fontCharWidth1x = font->getCharWidth()*1 + 1;	

	PatternTools* patternTools = &this->patternTools;

	// ;----------------- Little adjustment for scrolling in center
	// always assume properties.scrollMode == ScrollModeToCenter)
	if ((size.height - (SCROLLBARWIDTH + ((signed)rowHeight+4)))/(signed)rowHeight > (pattern->rows - startIndex + 1) && startIndex > 0)
		startIndex--;

	// ;----------------- start painting rows
	pp_int32 startx = location.x + (SCROLLBARWIDTH + getRowCountWidth()) + 4;

	pp_int32 previousPatternIndex = currentOrderlistIndex;
	pp_int32 previousRowIndex = 0;

	pp_int32 nextPatternIndex = currentOrderlistIndex;
	pp_int32 nextRowIndex = this->pattern->rows-1;

	pp_int32 songPosOrderListIndex = currentOrderlistIndex;

	TXMPattern* pattern = this->pattern;

	// ----------------- colors ----------------- 
	PPColor noteColor = TrackerConfig::colorPatternEditorNote;
	PPColor insColor = TrackerConfig::colorSampleEditorWaveform;  // colorThemeMain
	PPColor volColor = TrackerConfig::colorPatternEditorVolume;
	PPColor effColor = TrackerConfig::colorPatternEditorEffect;
	PPColor opColor = TrackerConfig::colorPatternEditorOperand;
	PPColor hiLightPrimary = TrackerConfig::colorHighLight_1;
	PPColor hiLightSecondary = TrackerConfig::colorHighLight_2;	
	PPColor hiLightPrimaryRow = TrackerConfig::colorRowHighLight_1;
	PPColor hiLightSecondaryRow = TrackerConfig::colorRowHighLight_2;
	//TrackerConfig::colorPatternEditorSelection ); 

	PPColor textColor = PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText);

	pp_int32 numVisibleChannels = patternEditor->getNumChannels();

	for (pp_int32 i2 = startIndex;; i2++)
	{
		i = i2 < 0 ? startIndex - i2 - 1: i2;

		pp_int32 px = location.x + SCROLLBARWIDTH;
		pp_int32 py = location.y+20 + (i-startIndex) * rowHeight;

		// rows are already in invisible area => abort
		if (py >= location.y + size.height)
			break;

		pp_int32 row = i;

		if (i2 < 0 || i2 >= pattern->rows)
			continue;

		// draw position line
		bool currentLine = (row == songPos.row && songPosOrderListIndex == songPos.orderListIndex) ||
			(i >= 0 && i <= pattern->rows - 1 && i == songPos.row && songPos.orderListIndex == -1);

		// draw rows
		if (!(i % properties.highlightSpacingPrimary))
			g->setColor(textColor);
		else if (!(i % properties.highlightSpacingSecondary))
			g->setColor(textColor);
		else
			g->setColor(TrackerConfig::colorRowHighLight_1);

		if (properties.hexCount)
			PatternTools::convertToHex(name, myMod(row, pattern->rows), properties.prospective ? 2 : PatternTools::getHexNumDigits(pattern->rows-1));
		else
			PatternTools::convertToDec(name, myMod(row, pattern->rows), properties.prospective ? 3 : PatternTools::getDecNumDigits(pattern->rows-1));

		g->drawString(name, px+1, py + rowHeight/3);

		// draw channels
		for (j = startPos; j <  numVisibleChannels; j++)
		{
			pp_int32 px = (location.x + (j-startPos) * slotSize + SCROLLBARWIDTH) + (getRowCountWidth() + 4);

			// columns are already in invisible area => abort
			if (px >= location.x + size.width)
				break;

			pp_int32 py = location.y + SCROLLBARWIDTH;


			PPRect rect(px, py, px+slotSize-1, py + font->getCharHeight()+1 );
			g->fillVerticalShaded(rect, bgColor, bgColor, false);

			g->setColor(TrackerConfig::colorRowHighLight_1);

			if (j == menuInvokeChannel)
			{
				PPColor col = g->getColor();
				col.r = textColor.r - col.r;
				col.g = textColor.g - col.g;
				col.b = textColor.b - col.b;
				col.clamp();
				g->setColor(col);
			}


			if (muteChannels[j])
			  sprintf(name, "M");
			else sprintf(name, "%i", j+1);

			g->drawString(name, px + (slotSize>>1)-(((pp_int32)strlen(name)*font->getCharWidth())>>1), py+1);
		}

		for (j = startPos; j < numVisibleChannels; j++)
		{
			pp_int32 px = (j-startPos) * slotSize + startx;

			// columns are already in invisible area => abort
			if (px >= location.x + size.width)
				break;

			if (j >= selectionStart.channel && i >= selectionStart.row &&
					j <= selectionEnd.channel && i <= selectionEnd.row && i < this->pattern->rows)
			{
				g->setColor(*selectionColor);

				if ((row == songPos.row && songPosOrderListIndex == songPos.orderListIndex) ||
						(i >= 0 && i <= pattern->rows - 1 && i == songPos.row && songPos.orderListIndex == -1))
				{
					PPColor c = g->getColor();
					c.r = (TrackerConfig::colorThemeMain.r + c.r)>>1;
					c.g = (TrackerConfig::colorThemeMain.g + c.g)>>1;
					c.b = (TrackerConfig::colorThemeMain.b + c.b)>>1;
					c.clamp();
					g->setColor(c);
				}

				if (i == cursor.row)
				{
					PPColor c = g->getColor();
					c.r+=lineColor.r;
					c.g+=lineColor.g;
					c.b+=lineColor.b;
					c.clamp();
					g->setColor(c);
				}
				// clamp to full channel
				selectionStart.inner = 0;
				selectionEnd.inner = 7;

				if (selectionStart.channel == selectionEnd.channel && j == selectionStart.channel)
				{
					pp_int32 startx = cursorPositions[selectionStart.inner];
					pp_int32 endx = slotSize; 
					g->fill(PPRect(px + startx, py - (i == cursor.row ? 1 : 0), px + endx, py + rowHeight + (i == cursor.row ? 1 : 0)));
				}
				else if (j == selectionStart.channel)
				{
					pp_int32 offset = 0;
					g->fill(PPRect(px + offset, py - (i == cursor.row ? 1 : 0), px + slotSize, py + rowHeight + (i == cursor.row ? 1 : 0)));
				}
				else if (j == selectionEnd.channel)
				{
					pp_int32 offset = slotSize; 
					g->fill(PPRect(px, py - (i == cursor.row ? 1 : 0), px + offset, py + rowHeight + (i == cursor.row ? 1 : 0)));
				}
				else
				{
					g->fill(PPRect(px, py - (i == cursor.row ? 1 : 0), px + slotSize, py + rowHeight + (i == cursor.row ? 1 : 0)));
				}
			}

			// --------------------- draw cursor ---------------------
			if (j == cursor.channel && i == cursor.row )
			{

				if (hasFocus || !properties.showFocus)
					g->setColor(TrackerConfig::colorPatternEditorCursor);
				else
					g->setColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorGrayedOutSelection));
				g->fill(PPRect(px, py+1, px + slotSize, py + rowHeight -1));
			}


			patternTools->setPosition(pattern, j, row);

			PPColor noteCol = noteColor;

			// Show notes in red if outside PT 3 octaves
			if(properties.ptNoteLimit
					&& ((patternTools->getNote() >= 71 && patternTools->getNote() < patternTools->getNoteOffNote())
						|| patternTools->getNote() < 36))
			{
				noteCol.set(0xff,00,00);
			}

			if (muteChannels[j])
			{
				noteCol.scaleFixed(properties.muteFade);
			}

			// show enabled steps
			bool isNoteOff = patternTools->getNote() == patternTools->getNoteOffNote();

			// determine whether step should be lit up
			bool enabled = patternTools->getNote() != 0;
			pp_int32 eff = 0;
			pp_int32 op  = 0;
			patternTools->getFirstEffect(eff, op); // important: call before getNextEffect
			if( eff != 0 && op != 0 ) enabled = true;
			patternTools->getNextEffect(eff, op);				
			if( eff != 0 && op != 0 ) enabled = true;

			bool isCursor = cursor.row == i && cursor.channel == j;
			bool isInstr = cursor.inner < 3;
			bool isFX1   = cursor.inner == 3 || cursor.inner == 4;
			bool isFX2   = cursor.inner > 4;
			bool showValues = false;

			g->setColor( TrackerConfig::colorRowHighLight_1 );
			if( currentLine ){
				g->setColor( textColor );
			}
			if( (enabled && !isNoteOff) && !muteChannels[j] ){ 
				if( isInstr ) g->setColor( insColor );
				if( isFX1 || isFX2  ) g->setColor( opColor );
			}
			if( isCursor ){
				g->setColor( noteColor );
			}

		
			if( !muteChannels[j] && !isNoteOff){
				g->fill(PPRect(px+3, py +3, px + slotSize -3, py + rowHeight -3 ));
				if( currentLine ) g->setColor(noteColor);
			}else{
				g->drawHLine(px+3, px + slotSize - 3,py+3);
				g->drawHLine(px+3, px + slotSize - 3,py+rowHeight-4);
			}
			g->drawVLine(py+4, py+rowHeight-4, px+2);
			g->drawVLine(py+4, py+rowHeight-4, px+slotSize-3);
			if (!(i % properties.highlightSpacingPrimary) && properties.highLightRowPrimary)
				g->setColor( noteColor);
			if (!(i % properties.highlightSpacingSecondary) && properties.highLightRowSecondary)
				g->setColor(effColor);

			// draw instr
			px += properties.spacing;
			g->setColor(insColor);
			pp_uint32 i = patternTools->getInstrument();
				
			g->setFont(PPFont::getFont(PPFont::FONT_TINY));
			g->setColor(bgColor);
			sprintf(name," ");
			sprintf(label," ");

			if (i && isInstr && enabled && !isNoteOff){
				patternTools->convertToHex(name, i, 2);
				g->drawString(name,px+4, py + rowHeight-font->getCharHeight()-3);

			}
			if( isFX1 && enabled){

				if (pattern->effnum >= 2)
				{
					patternTools->getFirstEffect(eff, op);
					patternTools->convertEffectsToFT2(eff, op);
					pp_int32 volume = patternTools->getVolumeFromEffect(eff, op);
					pp_int32 sliderWidth = (pp_int32)( float(volume) * float(slotSize/80.0f) );
					if( volume != 0 ){
						patternTools->getVolumeName(name, volume);
						sprintf(label,"vol");
						g->setColor(lineColor);
						g->fill(PPRect(px+3, py + 5, px + slotSize -3, py + 9 ));
						g->setColor(insColor);
						g->fill(PPRect(px+4, py + 6, px + sliderWidth -4, py + 8 ));
						g->setColor(bgColor);
					}
				}
			}

			if( isFX2 && pattern->effnum >= 1){
				if( showValues ) sprintf(name,"  ");
				sprintf(label,"  ");
				patternTools->getFirstEffect(eff, op); // important: call before getNextEffect
				patternTools->getNextEffect(eff, op);				
				patternTools->convertEffectsToFT2(eff, op);
				if( eff != 0 && op != 0 ){
					patternTools->getEffectName(label, eff);
					switch( label[0] ){
						case '0': sprintf(label,"arp"); break; 
						case '1': sprintf(label,"porta up"); break; 
						case '2': sprintf(label,"porta dn"); break; 
						case '3': sprintf(label,"porta to"); break; 
						case '4': sprintf(label,"vibrato"); break; 
						case '5': sprintf(label,"porta to/"); break; 
						case '6': sprintf(label,"vibrato/"); break; 
						case '7': sprintf(label,"tremolo"); break; 
						case '8': sprintf(label,"pan2"); break; 
						case '9': sprintf(label,"start"); break; 
						case 'A': sprintf(label,"vol /\\"); break; 
						case 'B': sprintf(label,"jump"); break; 
						case 'C': sprintf(label,"vol2"); break; 
						case 'D': sprintf(label,"break"); break; 
						case 'E': sprintf(label,"subcmd"); break; 
						case 'F': sprintf(label,"BPM"); break; 
						case 'G': sprintf(label,"Gvol"); break; 
						case 'H': sprintf(label,"Gvolfade"); break; 
						case 'K': sprintf(label,"keyoff"); break; 
						case 'L': sprintf(label,"Envpos"); break; 
						case 'P': sprintf(label,"panslide"); break; 
						case 'R': sprintf(label,"retrigfade"); break; 
						case 'T': sprintf(label,"tremor"); break; 
						case 'X': sprintf(label,"fineporta"); break; 
					}
					pp_int32 sliderWidth = (pp_int32)( float(op) * float(slotSize/80.0f) );
					g->setColor(lineColor);
					g->fill(PPRect(px+3, py + 5, px + slotSize -3, py + 9 ));
					g->setColor(insColor);
					g->fill(PPRect(px+4, py + 6, px + sliderWidth -4, py + 8 ));
					g->setColor(bgColor);
				}

			}

			if( name[0] != ' ' && ( isInstr || (isFX1 || isFX2) && showValues ) ){
				g->drawString(name,px+4, py + rowHeight-font->getCharHeight()-3);
			}
			g->setColor( lineColor );
			g->drawString(label,px+4, py + rowHeight-font->getCharHeight()-10);

			if( isNoteOff && isInstr ){
				g->setColor( lineColor );
				sprintf(name,"off");
				g->drawString(name,px+4, py + rowHeight-font->getCharHeight()-4);
				g->setFont(font);
				g->setColor( TrackerConfig::colorSampleEditorWaveform );
				g->drawHLine(px+3, px + slotSize - 3,py+3);
			}

			g->setFont(font);

		}
	}

	// --------------------- draw moved selection ---------------------

	if (hasValidSelection() && moveSelection)
	{
		pp_int32 moveSelectionRows = moveSelectionFinalPos.row - moveSelectionInitialPos.row;
		pp_int32 moveSelectionChannels = moveSelectionFinalPos.channel - moveSelectionInitialPos.channel;

		pp_int32 i1 = selectionStart.row + moveSelectionRows;
		pp_int32 j1 = selectionStart.channel + moveSelectionChannels;
		pp_int32 i2 = selectionEnd.row + moveSelectionRows;
		pp_int32 j2 = selectionEnd.channel + moveSelectionChannels;

		if (i2 >= 0 && j2 >= 0 && i1 < pattern->rows && j1 < numVisibleChannels)
		{
			i1 = PPTools::clamp(i1, 0, pattern->rows);
			i2 = PPTools::clamp(i2, 0, pattern->rows);
			j1 = PPTools::clamp(j1, 0, numVisibleChannels);
			j2 = PPTools::clamp(j2, 0, numVisibleChannels);

			pp_int32 x1 = (location.x + (j1-startPos) * slotSize + SCROLLBARWIDTH) + cursorPositions[selectionStart.inner] + (getRowCountWidth() + 4);
			pp_int32 y1 = (location.y + (i1-startIndex) * rowHeight + SCROLLBARWIDTH) + (rowHeight + 4);

			pp_int32 x2 = (location.x + (j2-startPos) * slotSize + SCROLLBARWIDTH) + cursorPositions[selectionEnd.inner]+cursorSizes[selectionEnd.inner] + (getRowCountWidth() + 3);
			pp_int32 y2 = (location.y + (i2-startIndex) * rowHeight + SCROLLBARWIDTH) + (rowHeight*2 + 2);

			// use a different color for cloning the selection instead of moving it
			if (::getKeyModifier() & selectionKeyModifier)
				g->setColor(hiLightPrimary);
			else
				g->setColor(textColor);

			const pp_int32 dashLen = 6;

			// inner dashed lines
			g->drawHLineDashed(x1, x2, y1, dashLen, 3);
			g->drawHLineDashed(x1, x2, y2, dashLen, 3+y2-y1);
			g->drawVLineDashed(y1, y2, x1, dashLen, 3);
			g->drawVLineDashed(y1, y2+2, x2, dashLen, 3+x2-x1);

			// outer dashed lines
			g->drawHLineDashed(x1-1, x2+1, y1-1, dashLen, 1);
			g->drawHLineDashed(x1-1, x2, y2+1, dashLen, 3+y2-y1);
			g->drawVLineDashed(y1-1, y2+1, x1-1, dashLen, 1);
			g->drawVLineDashed(y1-1, y2+2, x2+1, dashLen, 3+x2-x1);
		}

	}

	// draw scrollbars
	hTopScrollbar->paint(g);
	hBottomScrollbar->paint(g);
	vLeftScrollbar->paint(g); 	
	vRightScrollbar->paint(g); 
}
