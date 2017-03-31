on run argv
	-- Constants
	set X_POS to 400
	set Y_POS to 100
	set BG_W to 512
	set BG_H to 290
	set TITLE_BAR_H to 15

	set diskImage to item 1 of argv

	tell application "Finder"
		tell disk diskImage
			-- Setup background and icon arrangement
			open
			set current view of container window to icon view
			set theViewOptions to the icon view options of container window
			set background picture of theViewOptions to file ".background:background.tif"
			set arrangement of theViewOptions to not arranged
			set icon size of theViewOptions to 72
			delay 5
			close

			-- Setup window decoration and icon positions
			open
			update without registering applications
			tell container window
				set sidebar width to 0
				set statusbar visible to false
				set toolbar visible to false
				set the bounds to {X_POS, Y_POS, X_POS + BG_W, Y_POS + BG_H + TITLE_BAR_H}

				-- Move the icons; this is really finicky, the coordinates don't seem
				-- to make much sense and if you go too far then ugly scrollbars will appear
				set position of item "Documentation" to {77, 60}
				set position of item "Example Songs" to {413, 60}
				set position of item "MilkyTracker.app" to {161, 195}
				set position of item "Applications" to {332, 195}

				-- Move these out of the way for users with Finder configured to show all files
				set position of item ".background" to {161, 500}
				set position of item ".fseventsd" to {332, 500}
			end tell
			update without registering applications
			delay 5
			close

			-- Show window one more time for a final check
			open
			delay 5
			close
		end tell
		delay 1
	end tell
end run
