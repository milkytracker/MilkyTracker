on run argv
	set diskImage to item 1 of argv

	tell application "Finder"
		tell disk diskImage
			open
			set current view of container window to icon view
			set theViewOptions to the icon view options of container window
			set background picture of theViewOptions to file ".background:background.tif"
			set arrangement of theViewOptions to not arranged
			set icon size of theViewOptions to 72
			tell container window
				set sidebar width to 0
				set statusbar visible to false
				set toolbar visible to false
				set the bounds to {400, 100, 912, 400}
				set position of item "Documentation" to {72, 60}
				set position of item "Example Songs" to {440, 60}
				set position of item "MilkyTracker.app" to {171, 195}
				set position of item "Applications" to {341, 195}
			end tell
			update without registering applications
		end tell
	end tell
end run
