void PatternEditorControl::paintSteps(PPGraphicsAbstract* g)
{
	if (!isVisible())
		return;

	// ;----------------- everything all right?
	validate();

	// ;----------------- colors
	PPColor lineColor;
	pp_int32 instr = 0;

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
	char fx1[32];
	char fx2[32];
	char channelInstr[128] = {0};
	pp_uint32 statusHeight = 0;
	mp_sint32 i,j;
	PPString statusLine;

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
	
	const PPColor colors[] =    // 16 + pastelcolorpallete non-white non-green
	{
		opColor,
		volColor,
		effColor,
		insColor, 
		PPColor(255, 192, 203), // Pastel Magenta
		PPColor(255, 105, 180), // Pastel Pink
		PPColor(245, 222, 179), // Light Beige
		PPColor(230, 150, 210), // Soft Plum
		PPColor(230, 190, 255), // Pastel Purple
		PPColor(245, 130, 130), // Pastel Coral
		PPColor(240, 210, 210), // Pale Pink
		PPColor(255, 205, 210), // Light Blush
		PPColor(255, 230, 230), // Pastel Salmon
		PPColor(220, 180, 220), // Pastel Lilac
		PPColor(255, 200, 200), // Light Rose
		PPColor(250, 220, 230), // Soft Peach
	};
	PPColor stepColor;

	//TrackerConfig::colorPatternEditorSelection ); 

	PPColor textColor = PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText);

	pp_int32 numVisibleChannels = patternEditor->getNumChannels();

	stepColor = textColor;

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
			g->setColor(hiLightPrimaryRow);			
		else if (!(i % properties.highlightSpacingSecondary))
			g->setColor(hiLightSecondaryRow);			
		else
			g->setColor(TrackerConfig::colorRowHighLight_1);

		if (properties.hexCount)
			PatternTools::convertToHex(name, myMod(row, pattern->rows), properties.prospective ? 2 : PatternTools::getHexNumDigits(pattern->rows-1));
		else
			PatternTools::convertToDec(name, myMod(row, pattern->rows), properties.prospective ? 3 : PatternTools::getDecNumDigits(pattern->rows-1));

		g->drawString(name, px+1, py + font->getCharHeight() + 6 );


		// draw channels
		for (j = startPos; j < numVisibleChannels; j++)
		{

			pp_int32 px = (location.x + (j-startPos) * slotSize + SCROLLBARWIDTH) + (getRowCountWidth() + 4);
			
			// columns are already in invisible area => abort
			if (px >= location.x + size.width)
				break;
			
			pp_int32 py = location.y + SCROLLBARWIDTH;

			if (menuInvokeChannel == j)
				g->setColor(255-dColor.r, 255-dColor.g, 255-dColor.b);
			else
				g->setColor(dColor);

			{
				PPColor nsdColor = g->getColor(), nsbColor = g->getColor();
				
				if (menuInvokeChannel != j)
				{
					// adjust not so dark color
					nsdColor.scaleFixed(50000);
					
					// adjust bright color
					nsbColor.scaleFixed(80000);
				}
				else
				{
					// adjust not so dark color
					nsdColor.scaleFixed(30000);
					
					// adjust bright color
					nsbColor.scaleFixed(60000);
				}
				
				PPRect rect(px+2, py, px+slotSize-2, py + font->getCharHeight()+3);
				g->fillVerticalShaded(rect, nsbColor, nsdColor, false);
				
			}
			
			if (muteChannels[j])
			{
				g->setColor(128, 128, 128);
			}
			else
			{
				if (!(j&1))
					g->setColor(hiLightPrimary);
				else
					g->setColor(textColor);
					
				if (j == menuInvokeChannel)
				{
					PPColor col = g->getColor();
					col.r = textColor.r - col.r;
					col.g = textColor.g - col.g;
					col.b = textColor.b - col.b;
					col.clamp();
					g->setColor(col);
				}
			}

			sprintf(name, "%i", j+1);

			if (muteChannels[j])
				strcat(name, "M");

			g->drawString(name, px + (slotSize>>1)-(((pp_int32)strlen(name)*font->getCharWidth())>>1), py+1);
		}

		g->setFont(PPFont::getFont(PPFont::FONT_TINY));
		drawStatus( statusLine, opColor, g, cursor, font, px + 2);
		g->setFont(font);

		py += font->getCharHeight() + 4;

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
			bool hasFX   = false;
			patternTools->getFirstEffect(eff, op); // important: call before getNextEffect
			if( eff != 0 ) hasFX = true;
			patternTools->getNextEffect(eff, op);				
			if( eff != 0 ) hasFX = true;

			instr        = patternTools->getInstrument();
			bool isCursor = cursor.row == i && cursor.channel == j;
			bool showValues = false; // future

			// update step colors if instrument changes
			if( instr != 0 ){
				channelInstr[ j ] = instr;
			}
			if( isNoteOff ) channelInstr[ j ] = 0;
			stepColor = colors[ channelInstr[ j ] ];

			g->setColor( TrackerConfig::colorRowHighLight_1 );
			if( currentLine ){
				g->setColor( textColor );
			}

			if (!(i % properties.highlightSpacingPrimary) && properties.highLightRowPrimary)
				g->setColor( hiLightPrimary);
			if (!(i % properties.highlightSpacingSecondary) && properties.highLightRowSecondary)
				g->setColor( hiLightSecondary);

			if( (enabled || hasFX ) && !isNoteOff && !muteChannels[j]){
				g->setColor( stepColor );
			}

			if( !muteChannels[j] && enabled ){
				if( isNoteOff ) g->setColor( TrackerConfig::colorRowHighLight_1 );
				g->fill(PPRect(px+3, py +3, px + slotSize -3, py + rowHeight -3 ));
				if( currentLine ) g->setColor(noteColor);
			}else{
				g->drawHLine(px+3, px + slotSize - 3,py+3);
				g->drawHLine(px+3, px + slotSize - 3,py+rowHeight-4);
			}
			g->drawVLine(py+4, py+rowHeight-4, px+2);
			g->drawVLine(py+4, py+rowHeight-4, px+slotSize-3);

			// draw instr
			px += properties.spacing;
				
			g->setColor( enabled ? bgColor : stepColor );
			sprintf(name," ");
			sprintf(fx1," ");
			sprintf(fx2," ");

			if (instr && !isNoteOff && instr > 0) sprintf(name,"%i",instr);
			if( name[0] != ' ' && cursor.inner == 1 || cursor.inner == 2 ){
				g->drawString(name,px+5, py + rowHeight-font->getCharHeight()-5);
			}

			// draw FX
			g->setFont(PPFont::getFont(PPFont::FONT_TINY));

			sprintf(name,"  ");
			if (pattern->effnum >= 2)
			{
				patternTools->getFirstEffect(eff, op);
				patternTools->convertEffectsToFT2(eff, op);
				pp_int32 volume = patternTools->getVolumeFromEffect(eff, op);
				pp_int32 sliderWidth = (pp_int32)( float(volume) * float(slotSize/80.0f) );
				if( eff != 0  ){
					patternTools->getEffectName(name, eff);
					patternTools->getEffectDescription( name, name[0]);
				}
				if( volume != 0 ){
					/* // *FUTURE* show sliders
					 * if( showValues ){
					 * g->setColor(lineColor);
					 * g->fill(PPRect(px+3, py + 5, px + slotSize -3, py + 9 ));
					 * g->setColor(insColor);
					 * g->fill(PPRect(px+4, py + 6, px + sliderWidth -4, py + 8 ));
					 * g->setColor(bgColor);
					 */
				}
				sprintf(fx1,"%.6s",name);
			}

			sprintf(name,"  ");
			if( pattern->effnum >= 1){
				patternTools->getFirstEffect(eff, op); // important: call before getNextEffect
				patternTools->getNextEffect(eff, op);				
				patternTools->convertEffectsToFT2(eff, op);
				if( eff != 0  ){
					patternTools->getEffectName(name, eff);
					patternTools->getEffectDescription( name, name[0]);
					
					/* // *FUTURE* show sliders
					 * if( showValues ){
					 *   pp_int32 sliderWidth = (pp_int32)( float(op) * float(slotSize/80.0f) );
					 *   g->setColor(lineColor);
					 *   g->fill(PPRect(px+3, py + 5, px + slotSize -3, py + 9 ));
					 *   g->setColor(insColor);
					 *   g->fill(PPRect(px+4, py + 6, px + sliderWidth -4, py + 8 ));
					 *   g->setColor(bgColor);
					 * }
					*/
				}
				sprintf(fx2,"%.6s",name);
			}

			g->setColor( enabled ? bgColor : stepColor );
			if( cursor.inner == 3 || cursor.inner == 4 ){
				g->drawString(fx1,px+4, py+ font->getCharHeight()-1);
			}
			if( cursor.inner >= 5 || eff == 13 ){ // show breaks too
				g->drawString(fx2,px+4, py+ font->getCharHeight()-1);
			}

			if( isNoteOff ){
				g->setFont(font);
				g->setColor( stepColor );
				g->drawHLine(px+3, px + slotSize - 3,py+3);
			}

			if( cursor.inner == 0 && enabled ){
				patternTools->getNoteName(name, patternTools->getNote());
				g->setColor(bgColor);
				g->drawString(name,px+4, py+ font->getCharHeight()-1);
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
