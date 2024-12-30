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

	g->setRect(location.x+SCROLLBARWIDTH, location.y+SCROLLBARWIDTH, 
			   location.x + size.width - SCROLLBARWIDTH, location.y + size.height - SCROLLBARWIDTH);

	g->setColor(bgColor);
	g->setColor(lineColor); // removeme

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
	if (properties.scrollMode == ScrollModeToCenter)
	{
		if ((size.height - (SCROLLBARWIDTH + ((signed)font->getCharHeight()+4)))/(signed)font->getCharHeight() > (pattern->rows - startIndex + 1) && startIndex > 0)
			startIndex--;
	}

	// ;----------------- start painting rows
	pp_int32 startx = location.x + SCROLLBARWIDTH + getRowCountWidth() + 4;
	
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

		pp_int32 py = location.y + (i-startIndex) * font->getCharHeight() + SCROLLBARWIDTH + (font->getCharHeight() + 4);

		// rows are already in invisible area => abort
		if (py >= location.y + size.height)
			break;
			
		pp_int32 row = i;

		if (properties.prospective && properties.scrollMode == ScrollModeStayInCenter && currentOrderlistIndex != -1)
		{
			if (i < 0)
			{
				previousRowIndex--;
				if (previousRowIndex < 0)
				{
					previousPatternIndex--;
					if (previousPatternIndex >= 0)
					{
						pattern = &module->phead[module->header.ord[previousPatternIndex]];
						previousRowIndex = pattern->rows-1;
						
						noteColor.set(TrackerConfig::colorThemeMain.r, TrackerConfig::colorThemeMain.g, TrackerConfig::colorThemeMain.b);
						insColor = volColor = effColor = opColor = noteColor;
					}
					else
					{
						continue;
					}
				}
				
				songPosOrderListIndex = previousPatternIndex;
				row = previousRowIndex;
			}
			else if (i >= this->pattern->rows)
			{
				nextRowIndex++;
				if (nextRowIndex == pattern->rows && nextPatternIndex < module->header.ordnum)
				{
					nextPatternIndex++;
					if (nextPatternIndex < module->header.ordnum)
					{
						pattern = &module->phead[module->header.ord[nextPatternIndex]];
						nextRowIndex = 0;
						
						// Outside current range display colors of main theme
						noteColor.set(TrackerConfig::colorThemeMain.r, TrackerConfig::colorThemeMain.g, TrackerConfig::colorThemeMain.b);
						insColor = volColor = effColor = opColor = noteColor;
					}
					else 
					{
						continue;
					}
				}
				else if (nextPatternIndex >= module->header.ordnum)
				{
					continue;
				}
				
				songPosOrderListIndex = nextPatternIndex;
				row = nextRowIndex;
			}
			else
			{
				songPosOrderListIndex = currentOrderlistIndex;
				pattern = this->pattern;
				
				// inside current range display colors as usual
				noteColor = TrackerConfig::colorPatternEditorNote;
				insColor = TrackerConfig::colorPatternEditorInstrument;
				volColor = TrackerConfig::colorPatternEditorVolume;
				effColor = TrackerConfig::colorPatternEditorEffect;
				opColor = TrackerConfig::colorPatternEditorOperand;
			}
		}
		else
		{
			if (i2 < 0 || i2 >= pattern->rows)
				continue;

			row = i;
		}

		// draw rows
		if (!(i % properties.highlightSpacingPrimary) && properties.highLightRowPrimary)
		{
			g->setColor(hiLightPrimaryRow);			
			for (pp_int32 k = 0; k < (pp_int32)font->getCharHeight(); k++)
				g->drawHLine(startx - (getRowCountWidth() + 4), startx+visibleWidth, py + k);
		}
		else if (!(i % properties.highlightSpacingSecondary) && properties.highLightRowSecondary)
		{
			g->setColor(hiLightSecondaryRow);			
			for (pp_int32 k = 0; k < (pp_int32)font->getCharHeight(); k++)
				g->drawHLine(startx - (getRowCountWidth() + 4), startx+visibleWidth, py + k);
		}
		
		// draw position line
		if ((row == songPos.row && songPosOrderListIndex == songPos.orderListIndex) ||
			(i >= 0 && i <= pattern->rows - 1 && i == songPos.row && songPos.orderListIndex == -1))
		{
			PPColor lineColor(TrackerConfig::colorThemeMain.r>>1, TrackerConfig::colorThemeMain.g>>1, TrackerConfig::colorThemeMain.b>>1);
			g->setColor(lineColor);
			for (pp_int32 k = 0; k < (pp_int32)font->getCharHeight(); k++)
				g->drawHLine(startx - (getRowCountWidth() + 4), startx+visibleWidth, py + k);
		}

		// draw cursor line
		if (i == cursor.row)
		{
			g->setColor(bCursor);			
			g->drawHLine(startx - (getRowCountWidth() + 4), startx+visibleWidth, py - 1);
			g->setColor(dCursor);			
			g->drawHLine(startx - (getRowCountWidth() + 4), startx+visibleWidth, py + (pp_int32)font->getCharHeight());

			g->setColor(lineColor);			
			for (pp_int32 k = 0; k < (pp_int32)font->getCharHeight(); k++)
				g->drawHLine(startx - (getRowCountWidth() + 4), startx+visibleWidth, py + k);
		}
		
		// draw rows
		if (!(i % properties.highlightSpacingPrimary))
			g->setColor(hiLightPrimary);
		else if (!(i % properties.highlightSpacingSecondary))
			g->setColor(hiLightSecondary);
		else
			g->setColor(textColor);

		if (properties.hexCount)
			PatternTools::convertToHex(name, myMod(row, pattern->rows), properties.prospective ? 2 : PatternTools::getHexNumDigits(pattern->rows-1));
		else
			PatternTools::convertToDec(name, myMod(row, pattern->rows), properties.prospective ? 3 : PatternTools::getDecNumDigits(pattern->rows-1));
		
		g->drawString(name, px, py);

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
				
				PPRect rect(px, py, px+slotSize, py + font->getCharHeight()+1);
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
				
				if (selectionStart.channel == selectionEnd.channel && j == selectionStart.channel)
				{
					pp_int32 startx = cursorPositions[selectionStart.inner];
					pp_int32 endx = cursorPositions[selectionEnd.inner] + cursorSizes[selectionEnd.inner];
					g->fill(PPRect(px + startx, py - (i == cursor.row ? 1 : 0), px + endx, py + font->getCharHeight() + (i == cursor.row ? 1 : 0)));
				}
				else if (j == selectionStart.channel)
				{
					pp_int32 offset = cursorPositions[selectionStart.inner];
					g->fill(PPRect(px + offset, py - (i == cursor.row ? 1 : 0), px + slotSize, py + font->getCharHeight() + (i == cursor.row ? 1 : 0)));
				}
				else if (j == selectionEnd.channel)
				{
					pp_int32 offset = cursorPositions[selectionEnd.inner] + cursorSizes[selectionEnd.inner];
					g->fill(PPRect(px, py - (i == cursor.row ? 1 : 0), px + offset, py + font->getCharHeight() + (i == cursor.row ? 1 : 0)));
				}
				else
				{
					g->fill(PPRect(px, py - (i == cursor.row ? 1 : 0), px + slotSize, py + font->getCharHeight() + (i == cursor.row ? 1 : 0)));
				}
			}

			// --------------------- draw cursor ---------------------
			if (j == cursor.channel &&
				i == cursor.row)
			{
				if (hasFocus || !properties.showFocus)
					g->setColor(TrackerConfig::colorPatternEditorCursor);
				else
					g->setColor(PPUIConfig::getInstance()->getColor(PPUIConfig::ColorGrayedOutSelection));
					
				for (pp_int32 k = cursorPositions[cursor.inner]; k < cursorPositions[cursor.inner]+cursorSizes[cursor.inner]; k++)
					g->drawVLine(py, py + font->getCharHeight(), px + k);

				PPColor c = g->getColor();
				PPColor c2 = c;
				c.scaleFixed(32768);
				c2.scaleFixed(87163);
				g->setColor(c2);
				g->drawHLine(px + cursorPositions[cursor.inner], px + cursorPositions[cursor.inner]+cursorSizes[cursor.inner], py - 1);
				g->setColor(c);
				g->drawHLine(px + cursorPositions[cursor.inner], px + cursorPositions[cursor.inner]+cursorSizes[cursor.inner], py + font->getCharHeight());
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

			g->setColor(noteCol);
			patternTools->getNoteName(name, patternTools->getNote());
			g->drawString(name,px, py);

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

			g->drawString(name,px, py);
			
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

			g->drawString(name,px, py);
			
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

			g->drawString(name,px, py);

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

			g->drawString(name,px, py);
		}
	}
	
	for (j = startPos; j < numVisibleChannels; j++)
	{

		pp_int32 px = (location.x + (j-startPos) * slotSize + SCROLLBARWIDTH) + (getRowCountWidth() + 4);
			
		// columns are already in invisible area => abort
		if (px >= location.x + size.width)
			break;

		px += fontCharWidth3x + properties.spacing;
		px += fontCharWidth2x*3-1 + properties.spacing*2;
		px += fontCharWidth1x;

		g->setColor(*borderColor);
		
		g->drawVLine(location.y, location.y + size.height, px+1);
		
		g->setColor(bColor);
		
		g->drawVLine(location.y, location.y + size.height, px);
		
		g->setColor(dColor);
		
		g->drawVLine(location.y, location.y + size.height, px+2);
	}

	// ;----------------- Margin lines
	// draw margin vertical line
	g->setColor(*borderColor);
		
	pp_int32 px = location.x + SCROLLBARWIDTH;
	px+=getRowCountWidth() + 1;
	g->drawVLine(location.y, location.y + size.height, px+1);
	
	g->setColor(bColor);	
	g->drawVLine(location.y, location.y + size.height, px);
	
	g->setColor(dColor);	
	g->drawVLine(location.y, location.y + size.height, px+2);
	
	// draw margin horizontal lines
	for (j = 0; j < visibleWidth / slotSize + 1; j++)
	{		
		pp_int32 px = (location.x + j * slotSize + SCROLLBARWIDTH) + (getRowCountWidth() + 4) - 1;
		
		// columns are already in invisible area => abort
		if (px >= location.x + size.width)
			break;
		
		pp_int32 py = location.y + SCROLLBARWIDTH;
		
		py+=font->getCharHeight() + 1;
		
		// Did we reach the maximum number of channels already?
		// no: just draw seperate horizontal line segments between the vertical margin lines
		if (startPos + j < numVisibleChannels)
		{
			g->setColor(*borderColor);	
			g->drawHLine(px, px + slotSize - 1, py+1);
			
			g->setColor(bColor);	
			g->drawHLine(px + 1, px + slotSize - 2, py);
			
			g->setColor(dColor);	
			g->drawHLine(px + 1, px + slotSize - 2, py+2);
		}
		// yes: draw the horizontal margin line completely to the right and abort loop
		else
		{
			g->setColor(*borderColor);	
			g->drawHLine(px, location.x + size.width, py+1);
			
			g->setColor(bColor);	
			g->drawHLine(px + 1, location.x + size.width, py);
			
			g->setColor(dColor);	
			g->drawHLine(px + 1, location.x + size.width, py+2);
			break;
		}
	}
	
	// --------------------- draw moved selection ---------------------
	
	if (properties.advancedDnd && hasValidSelection() && moveSelection)
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
			
			pp_int32 x1 = (location.x + (j1 - startPos) * slotSize + SCROLLBARWIDTH) + cursorPositions[selectionStart.inner] + (getRowCountWidth() + 4);
			pp_int32 y1 = (location.y + (i1 - startIndex) * font->getCharHeight() + SCROLLBARWIDTH) + (font->getCharHeight() + 4);
			
			pp_int32 x2 = (location.x + (j2 - startPos) * slotSize + SCROLLBARWIDTH) + cursorPositions[selectionEnd.inner]+cursorSizes[selectionEnd.inner] + (getRowCountWidth() + 3);
			pp_int32 y2 = (location.y + (i2 - startIndex) * font->getCharHeight() + SCROLLBARWIDTH) + (font->getCharHeight() * 2 + 2);
			
			// use a different color for cloning the selection instead of moving it
			if (::getKeyModifier() & selectionKeyModifier)
				g->setColor(hiLightPrimary);
			else
				g->setColor(textColor);
			
			const pp_int32 dashLen = 6;
			
			// inner dashed lines
			g->drawHLineDashed(x1, x2, y1, dashLen, 3);
			g->drawHLineDashed(x1, x2, y2, dashLen, 3 + y2 - y1);
			g->drawVLineDashed(y1, y2, x1, dashLen, 3);
			g->drawVLineDashed(y1, y2+2, x2, dashLen, 3 + x2 - x1);
			
			// outer dashed lines
			g->drawHLineDashed(x1-1, x2+1, y1-1, dashLen, 1);
			g->drawHLineDashed(x1-1, x2, y2+1, dashLen, 3 + y2 - y1);
			g->drawVLineDashed(y1-1, y2+1, x1-1, dashLen, 1);
			g->drawVLineDashed(y1-1, y2+2, x2+1, dashLen, 3 + x2 - x1);
		}
		
	}
	
	// draw scrollbars
	hTopScrollbar->paint(g);
	hBottomScrollbar->paint(g);
	vLeftScrollbar->paint(g); 	
	vRightScrollbar->paint(g); 
}
