/*

File: AIFFWriter.h

Author: QuickTime DTS

Change History (most recent first): <1> 11/10/05 initial release

© Copyright 2005 - 2006 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
consideration of your agreement to the following terms, and your use, installation,
modification or redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use, install, modify or
redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these
terms, Apple grants you a personal, non-exclusive license, under Apple's copyrights in
this original Apple software (the "Apple Software"), to use, reproduce, modify and
redistribute the Apple Software, with or without modifications, in source and/or binary
forms; provided that if you redistribute the Apple Software in its entirety and without
modifications, you must retain this notice and the following text and disclaimers in all
such redistributions of the Apple Software. Neither the name, trademarks, service marks
or logos of Apple Computer, Inc. may be used to endorse or promote products derived from
the Apple Software without specific prior written permission from Apple.  Except as
expressly stated in this notice, no other rights or licenses, express or implied, are
granted by Apple herein, including but not limited to any patent rights that may be
infringed by your derivative works or by other works in which the Apple Software may be
incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES,
EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF
NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE
APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE
USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER
CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#import <Cocoa/Cocoa.h>

#import <AudioToolbox/AudioToolbox.h>
#import <QTKit/QTKit.h>
#import <QuickTime/QuickTime.h>

// maximum size in frames of the MovieAudioExtractionFillBuffer calls
#define kMaxExtractionPacketCount 4096

// this object is busy exporting error code
#define kObjectInUseErr 1000

// constants for shouldContinueOperation delegate method
typedef enum {
	AIFFWriterExportBegin	 = 0,
	AIFFWriterExportPercent  = 10,
	AIFFWriterExportEnd		 = 20
} AIFFWriterExportOperationPhase;

// ProgressInfo object passed back to the delegate
// if it implements shouldContinueOperationWithProgressInfo:
// progressValue will contain a valid NSNumber object with a
// value between 0 and 1.0 when phase is AIFFWriterExportPercent
@interface AIFFWriterProgressInfo : NSObject
{
@private
    AIFFWriterExportOperationPhase  phase;	// one of the state enums
    NSNumber *progressValue;				// value is between 0 and 1.0 valid only for AIFFWriterExportPercent phase
    NSError  *exportStatus;
}

- (AIFFWriterExportOperationPhase)phase;
- (NSNumber *)progressValue;
- (NSError *)exportStatus;

@end

// invoked on a delegate to provide progress
@interface NSObject (AIFFWriterExportDelegate)

- (BOOL)shouldContinueOperationWithProgressInfo:(id)inProgressInfo;

@end

// AIFFWriter object
// A single instance of this object can be used to extract audio
// from a QuickTime movie (via a QTKit Movie object) and will write
// an AIFF file preserving the audio layout from the Movie
@interface AIFFWriter : NSObject
{
@private
	NSString 					*mFileName;                     // file name for the new .aiff file
    FSRef	  					mFileRef;                       // file reference for this file
    FSRef	  					mParentRef;                     
    
    QTMovie						*mQTMovie;                      // movie to extract audio from
    Float64						mMovieDuration;                 // movie duration
    Movie						mCloneMovie;                    // copy of the source movie for thread migration
    
	MovieAudioExtractionRef     mAudioExtractionSession;        // QT Audio Extraction Session Reference
    BOOL                        mExtractionComplete;            // are we done yet?
    BOOL                        mIsExporting;                   // is the object busy
    SInt64						mLocationInFile;                // location to write new data
    SInt64						mSamplesRemaining;              // how much more do we need to pull from the source?
    SInt64						mSamplesCompleated;             // hom much have we done - used to drive progress UI
    SInt64						mTotalNumberOfSamples;          // total number of samples to extract
    
	AudioStreamBasicDescription	mSourceASBD;                    // audio stream basic description of the source movie
    AudioStreamBasicDescription	mOutputASBD;                    // the asbd we're asking for
    AudioChannelLayout *		mExtractionLayoutPtr;           // the audio channel layout of the source
    UInt32                      mExtractionLayoutSize;          // the size of the audio chanel layout
    AudioFileID					mExportFileID;                  // file identifier for the new .aiff file
    
    NSLock *					mLock;                          // lock protecting reentrance
    
    AIFFWriterProgressInfo *    mProgressInfo;                  // progress info object passed to the progress callback
    
    id							mDelegate;                      // a delegate object to call with progress info...
    BOOL						mDelegateShouldContinueOp;      // ...but only if it actually implemented the callback
}

- (OSStatus)exportFromMovie:(QTMovie *)inMovie toFile:(NSString *)inFullPath;
- (BOOL)isExporting;

@end

// the client of AIFFWriter should set itself as a delegate if it
// wants to handle shouldContinueOperationWithProgressInfo
@interface AIFFWriter (AIFFWriterDelegate)

- (id)delegate;
- (void)setDelegate:(id)delegate;

@end
