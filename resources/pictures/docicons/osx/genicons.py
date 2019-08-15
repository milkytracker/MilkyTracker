#!/usr/bin/env python
#
# genicons.py
#
# MilkyTracker document icons generation script for OSX
# Dale Whinham 25/7/2015
#
# To run this script you will need Python 2.7 and PyObjC, which should both be included with Mac OS X.
#
# If you have upgraded Python using homebrew or similar and are missing PyObjC, install it like so:
#     $ pip install -U pyobjc-core
#     $ pip install -U pyobjc
#
# You will also need Docerator 2.0. Download and unzip it to a directory called 'docerator' in the same directory as this script.
#     $ wget https://docerator.googlecode.com/files/docerator-2.0.zip
#     $ unzip docerator-2.0.zip -d docerator
#     $ rm docerator-2.0.zip

from os import listdir, remove, path
from shutil import rmtree
from subprocess import call
import sys

DOCERATOR_PATH = "docerator"
APPICON_PATH = "../../carton.png"
MAKEICNS_PATH = "docerator/makeicns"
SYS_DOCICON_PATH = "/System/Library/CoreServices/CoreTypes.bundle/Contents/Resources/GenericDocumentIcon.icns"
ICONSET_DIR = "GenericDocumentIcon.iconset"
ICNS_FILE = "GenericDocumentIcon.icns"

# Add docerator's folder to the system path so we can import it from another folder
sys.path.append(DOCERATOR_PATH)
import docerator

# Rects calculated by running flow (included with Docerator) on a template designed in a graphics editor (I used Pixelmator).
# The template is the OSX document icon with the 'carton' pasted on top as follows:
# 128x128: Centred horizontally, translated 33px from bottom edge, 60% scale.
# 256x256: Centred horizontally, translated 66px from bottom edge, 120% scale.
# 512x512: Centred horizontally, translated 132px from bottom edge, 240% scale.
RECTS = {
	128: (1.4138, 0.5835, -7.3221, 0.5953),
	256: (2.4603, 0.5952, -14.6274, 0.5975),
	512: (5.9676, 0.5970, -29.7340, 0.6000)
}

# Dictionary of file types and their extensions
FILETYPES = {
	'MilkyTracker-Protracker-Module': 'MOD',
	'MilkyTracker-Ultratracker-Module': 'ULT',
	'MilkyTracker-PolyTracker-Module': 'PTM',
	'MilkyTracker-Cubic-Tiny-XM-Module': 'MXM',
	'MilkyTracker-Fasttracker-2-Extended-Module': 'XM',
	'MilkyTracker-Disorder_Tracker-2-Module': 'PLM',
	'MilkyTracker-Game-Music-Creator-Module': 'GMC',
	'MilkyTracker-Digibooster-Module': 'DIGI',
	'MilkyTracker-Digibooster-Pro-Module': 'DBM',
	'MilkyTracker-Velvet-Studio-Module': 'AMS',
	'MilkyTracker-Digitracker-3.0-Module': 'MDL',
	'MilkyTracker-Multitracker-Module': 'MTM',
	'MilkyTracker-Epic-Megagames-MASI-Module': 'PSM',
	'MilkyTracker-ScreamTracker-3-Module': 'S3M',
	'MilkyTracker-ScreamTracker-2-Module': 'STM',
	'MilkyTracker-Imago-Orpheus-Module': 'IMF',
	'MilkyTracker-Oktalyzer-Module': 'OKT',
	'MilkyTracker-MikMod-Module': 'UNI',
	'MilkyTracker-Digital-Tracker-Module': 'DTM',
	'MilkyTracker-Asylum-Music-Format-Module': 'AMF',
	'MilkyTracker-Digisound-Interface-Kit-Module': 'DSM',
	'MilkyTracker-General-Digimusic-Module': 'GDM',
	'MilkyTracker-Farandole-Composer-Module': 'FAR',
	'MilkyTracker-Impulse-Tracker-Module': 'IT',
	'MilkyTracker-Composer-669-Module': '669',
	'MilkyTracker-Fasttracker-2-Extended-Instrument': 'XI',
	'MilkyTracker-Gravis-Ultrasound-Patch': 'PAT',
	'MilkyTracker-8SVX-Sample': '8SVX',
	'MilkyTracker-8SVX-Sample-IFF': 'IFF',
	'MilkyTracker-WAV-Sample': 'WAV',
	'MilkyTracker-Fasttracker-2-Extended-Pattern': 'XP',
	'MilkyTracker-Fasttracker-2-Extended-Track': 'XT'
}

# Get system document icns file and extract individual sizes
call(["iconutil", "-c", "iconset", "-o", ICONSET_DIR, SYS_DOCICON_PATH])

# Remove Mavericks-onwards high-resolution '2x' variants, as Docerator currently cannot handle them
for f in listdir(ICONSET_DIR):
	if "@2x" in f:
		remove(path.join(ICONSET_DIR, f))

# Rebuild local copy of .icns file without '2x' variants
call(["iconutil", "-c", "icns", "-o", ICNS_FILE, ICONSET_DIR])

# Remove previously extracted icons
rmtree(ICONSET_DIR)

# Generate document icons
for name, extension in FILETYPES.iteritems():
	print(name)
	docerator.makedocicon(	outname='%s.icns' % name,
							appicon=APPICON_PATH,
							text=extension,
							sizes=[512, 256, 128],
							makeicns=MAKEICNS_PATH,
							background=ICNS_FILE,
							rects=RECTS
	)

# Clean up temporary document background file
remove(ICNS_FILE)
