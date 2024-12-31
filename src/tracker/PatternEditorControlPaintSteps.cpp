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

	mp_sint32 i,j;

	// ;----------------- selection layout
	PatternEditorTools::Position selectionStart, selectionEnd;
	selectionStart = patternEditor->getSelection().start;
	selectionEnd = patternEditor->getSelection().end;

	PatternEditorTools::flattenSelection(selectionStart, selectionEnd);	

	// only entire instrument column is allowed
	selectionStart.inner = 0;
	selectionEnd.inner = 7;

	PatternEditorTools::Position& cursor = patternEditor->getCursor();

	if (cursor.inner < 0 || cursor.inner >= 8)
		cursor.inner = 0;

	// ;----------------- some constants
	const pp_uint32 fontCharWidth3x = font->getCharWidth()*3 + 1;
	const pp_uint32 fontCharWidth2x = font->getCharWidth()*2 + 1;
	const pp_uint32 fontCharWidth1x = font->getCharWidth()*1 + 1;	

	PatternTools* patternTools = &this->patternTools;

	// ;----------------- Little adjustment for scrolling in center
	if (properties.scrollMode == ScrollModeToCenter)
	{
		if ((size.height - (SCROLLBARWIDTH + ((signed)rowHeight+4)))/(signed)rowHeight > (pattern->rows - startIndex + 1) && startIndex > 0)
			startIndex--;
	}

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
	PPColor insColor = TrackerConfig::colorPatternEditorInstrument;
	PPColor volColor = TrackerConfig::colorPatternEditorVolume;
	PPColor effColor = TrackerConfig::colorPatternEditorEffect;
	PPColor opColor = TrackerConfig::colorPatternEditorOperand;
	PPColor hiLightPrimary = TrackerConfig::colorHighLight_1;
	PPColor hiLightSecondary = TrackerConfig::colorHighLight_2;	
	PPColor hiLightPrimaryRow = TrackerConfig::colorRowHighLight_1;
	PPColor hiLightSecondaryRow = TrackerConfig::colorRowHighLight_2;

	PPColor textColor = PPUIConfig::getInstance()->getColor(PPUIConfig::ColorStaticText);

	pp_int32 numVisibleChannels = patternEditor->getNumChannels();

	for (pp_int32 i2 = startIndex;; i2++)
	{
		i = i2 < 0 ? startIndex - i2 - 1: i2;

		pp_int32 px = location.x + SCROLLBARWIDTH;
		pp_int32 py = location.y + (i-startIndex) * rowHeight;

		// rows are already in invisible area => abort
		if (py >= location.y + size.height)
			break;

		pp_int32 row = i;

		if (i2 < 0 || i2 >= pattern->rows)
			continue;

		// draw position line
		bool currentLine = (row == songPos.row && songPosOrderListIndex == songPos.orderListIndex) ||
			(i >= 0 && i <= pattern->rows - 1 && i == songPos.row && songPos.orderListIndex == -1);
		if ( currentLine )
		{
			PPColor lineColor(TrackerConfig::colorThemeMain.r>>1, TrackerConfig::colorThemeMain.g>>1, TrackerConfig::colorThemeMain.b>>1);
			g->setColor(lineColor);
			for (pp_int32 k = 0; k < (pp_int32)rowHeight; k++)
				g->drawHLine(startx - (getRowCountWidth() + 4), startx+visibleWidth, py + k);
		}

		// draw cursor line
		if (i == cursor.row )
		{
			g->setColor(bCursor);			
			g->drawHLine(startx - (getRowCountWidth() + 4), startx+visibleWidth, py - 1);
			g->setColor(dCursor);			
			g->drawHLine(startx - (getRowCountWidth() + 4), startx+visibleWidth, py + (pp_int32)rowHeight);

			g->setColor(lineColor);			
			for (pp_int32 k = 0; k < (pp_int32)rowHeight; k++)
				g->drawHLine(startx - (getRowCountWidth() + 4), startx+visibleWidth, py + k);
		}

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

		g->drawString(name, px, py + rowHeight/3);

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

			sprintf(name, "%i", j+1);

			if (muteChannels[j])
				strcat(name, " <Mute>");

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
					pp_int32 endx = cursorPositions[selectionEnd.inner] + cursorSizes[selectionEnd.inner];
					g->fill(PPRect(px + startx, py - (i == cursor.row ? 1 : 0), px + endx, py + rowHeight + (i == cursor.row ? 1 : 0)));
				}
				else if (j == selectionStart.channel)
				{
					pp_int32 offset = cursorPositions[selectionStart.inner];
					g->fill(PPRect(px + offset, py - (i == cursor.row ? 1 : 0), px + slotSize, py + rowHeight + (i == cursor.row ? 1 : 0)));
				}
				else if (j == selectionEnd.channel)
				{
					pp_int32 offset = cursorPositions[selectionEnd.inner] + cursorSizes[selectionEnd.inner];
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
				g->fill(PPRect(px+1, py+1, px + slotSize -1, py + rowHeight -1));
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
			bool enabled = patternTools->getNote() != 0;
			if( currentLine )
				g->setColor( TrackerConfig::colorPatternEditorSelection ); 
			else g->setColor( enabled && !muteChannels[j] ? noteColor : TrackerConfig::colorRowHighLight_1 );

			if( !muteChannels[j] || enabled ){ 
				g->fill(PPRect(px+3, py +3, px + slotSize -3, py + rowHeight -3 ));
			}else g->drawHLine(px+4, px + slotSize - 4,py+rowHeight-2);
			g->drawVLine(py+4, py+rowHeight-4, px+2);
			g->drawVLine(py+4, py+rowHeight-4, px+slotSize-3);
			if (!(i % properties.highlightSpacingPrimary) && properties.highLightRowPrimary)
				g->setColor( noteColor);
			if (!(i % properties.highlightSpacingSecondary) && properties.highLightRowSecondary)
				g->setColor(effColor);
			g->drawHLine(px+7, px + slotSize - 7,py+5);

			px += fontCharWidth3x + properties.spacing;

			if (muteChannels[j])
			{
				PPColor insCol = insColor;
				insCol.scaleFixed(properties.muteFade);
				g->setColor(insCol);
			}
			else
				g->setColor(insColor);

			pp_uint32 i = patternTools->getInstrument();

			if (i)
				patternTools->convertToHex(name, i, 2);
			else 
			{
				name[0] = name[1] = '\xf4';
				name[2] = 0;
			}

			if (name[0] == '0')
				name[0] = '\xf4';

			px += fontCharWidth2x + properties.spacing;

			if (muteChannels[j])
			{
				PPColor volCol = volColor;
				volCol.scaleFixed(properties.muteFade);
				g->setColor(volCol);
			}
			else
				g->setColor(volColor);

			pp_int32 eff, op;

			name[0] = name[1] = '\xf4';
			name[2] = 0;
			if (pattern->effnum >= 2)
			{
				patternTools->getFirstEffect(eff, op);

				patternTools->convertEffectsToFT2(eff, op);

				pp_int32 volume = patternTools->getVolumeFromEffect(eff, op);

				patternTools->getVolumeName(name, volume);
			}

			px += fontCharWidth2x + properties.spacing;

			if (muteChannels[j])
			{
				PPColor effCol = effColor;
				effCol.scaleFixed(properties.muteFade);
				g->setColor(effCol);
			}
			else
				g->setColor(effColor);

			if (pattern->effnum == 1)
			{
				patternTools->getFirstEffect(eff, op);				
				patternTools->convertEffectsToFT2(eff, op);
			}
			else
			{
				patternTools->getNextEffect(eff, op);				
				patternTools->convertEffectsToFT2(eff, op);
			}

			if (eff == 0 && op == 0)
			{
				name[0] = properties.zeroEffectCharacter;
				name[1] = 0;
			}
			else
			{
				patternTools->getEffectName(name, eff);
			}

			px += fontCharWidth1x;

			if (muteChannels[j])
			{
				PPColor opCol = opColor;
				opCol.scaleFixed(properties.muteFade);
				g->setColor(opCol);
			}
			else
				g->setColor(opColor);

			if (eff == 0 && op == 0)
			{
				name[0] = name[1] = properties.zeroEffectCharacter;
				name[2] = 0;
			}
			else
			{
				patternTools->convertToHex(name, op, 2);
			}			

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
