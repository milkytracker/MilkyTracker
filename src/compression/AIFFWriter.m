/*

File: AIFFWriter.m

Author: QuickTime DTS

Change History (most recent first): 
    
    <2> 03/24/06 must pass NSError objects to exportCompleted
    <1> 11/10/05 initial release

© Copyright 2005-2006 Apple Computer, Inc. All rights reserved.

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

/*

AIFFWriter is a simple class encapsulating the functionality of two sets of APIs; QuickTime's
Audio Extraction API's and Core Audio's Audio File APIs. It let's clients of this class perform
audio extraction from a QTKit QTMovie object to AIFF files using a single method call.

Note that this class uses the default extraction channel layout which is the aggregate
channel layout of the movie (for example, all Rights mixed together, all Left Surrounds mixed together, etc).

Because we're writing a file the stream description for the output is modified from the default
(32-bit float, de-interleaved) to 16-bit, interleaved big endian. The sample rate will be set
to the highest sample rate found in the movie.

For more information regarding newer audio capabilities of QuickTime, please see the QuickTime 7 Update Guide:

http://developer.apple.com/documentation/QuickTime/Conceptual/QT7UpdateGuide/Chapter02/chapter_2_section_6.html

*/

#import "AIFFWriter.h"

#pragma mark ---- AIFFWriterProgressInfo ----

// AIFFWriterProgressInfo is the object passed back to the progress callback
// if implemented by the client. It contains information regarding the current
// state of the export session which can be used to drive a UI.
@interface AIFFWriterProgressInfo (Private)

- (void)setPhase:(AIFFWriterExportOperationPhase)value;
- (void)setProgressValue:(NSNumber *)value;
- (void)setExportStatus:(NSError *)status;

@end

@implementation AIFFWriterProgressInfo
- (AIFFWriterExportOperationPhase)phase {
    return phase;
}

- (NSNumber *)progressValue {
    return [[progressValue retain] autorelease];
}

- (void)setPhase:(AIFFWriterExportOperationPhase)value {
    phase = value;
}

- (void)setProgressValue:(NSNumber *)value {
    if (progressValue != value) {
        [progressValue release];
        progressValue = [value copy];
    }
}

- (NSError *)exportStatus {
    return [[exportStatus retain] autorelease];
}

- (void)setExportStatus:(NSError *)status {
    if (exportStatus != status) {
        [exportStatus release];
        exportStatus = [status copy];
    }
}

- (void) dealloc
{
    [progressValue release];
    [exportStatus release];
    
    [super dealloc];
}

@end

#pragma mark ---- AIFFWriter private interface ----

@interface AIFFWriter (Private)

- (OSStatus)extractAudioToFile:(SInt64 *)ioNumSamples;
- (OSStatus)getDefaultExtractionInfo;
- (OSStatus)configureExtractionSessionWithMovie:(Movie)inMovie;

- (void)exportOnMainThreadCallBack:(id)inObject;
- (void)exportExtractionOnWorkerThread:(id)inObject;
- (void)setMovieExtractionDuration;
- (void)exportCompletedNotification:(NSError *)inError;

@end

@implementation AIFFWriter

#pragma mark ---- initialization/dealocation ----

- (id)init;
{
	if (self = [super init]) {
    
        mLock = [[NSLock alloc] init];
        mProgressInfo = [[AIFFWriterProgressInfo alloc] init];
    }

	return self;
}

- (void)dealloc
{
	if (mFileName) {
    	[mFileName release];
    }
    
	if (mAudioExtractionSession){
		MovieAudioExtractionEnd(mAudioExtractionSession);
    }
    
    if (mQTMovie) {
    	[mQTMovie release];
    }
    
    if (mExtractionLayoutPtr) {
    	free(mExtractionLayoutPtr);
    }
    
    [mLock release];
    
    [mProgressInfo release];
    
    [super dealloc]; 
}

#pragma mark ---- public ----

// main method call that will produce an AIFF file - it will try to
// export the movie on a separate thread but if it can't will schedule
// callbacks on the main thread
- (OSStatus)exportFromMovie:(QTMovie *)inMovie toFile:(NSString *)inFullPath
{
    BOOL continueExport = YES;
    Handle cloneHandle = NULL;
    NSString *directory;
    
    OSStatus err = noErr;
    
    // sanity
    if (nil == inMovie || nil == inFullPath) return paramErr;
    
    // if we're busy already doing an export return
    if (![mLock tryLock]) return kObjectInUseErr;
    
    mIsExporting = YES;
    
    // if the client implemented a progress proc. call it now
    if (TRUE == mDelegateShouldContinueOp) {
        [mProgressInfo setPhase:AIFFWriterExportBegin];
        [mProgressInfo setProgressValue:nil];
        [mProgressInfo setExportStatus:nil];
        
        continueExport = [[self delegate] shouldContinueOperationWithProgressInfo:mProgressInfo];

        if (NO == continueExport) goto bail;
    }
    
    directory = [inFullPath stringByDeletingLastPathComponent];
    
    mFileName = [[NSString alloc] initWithString:[inFullPath lastPathComponent]];
    
	// retain the QTMovie object passed in, we need it for the duration of
    // the export regardless of what the client decides to do with it
    mQTMovie = [inMovie retain];
 
    // if the file already exists, delete it
    err = FSPathMakeRef((const UInt8*)[inFullPath fileSystemRepresentation], &mFileRef, false);
    if (err == noErr) {
        err = FSDeleteObject(&mFileRef);
        if (err) goto bail;
    }
    
    err = FSPathMakeRef((const UInt8*)[directory fileSystemRepresentation], &mParentRef, NULL);
    if (err) goto bail;
    
    // set the movies extraction duration in floating-point seconds
    [self setMovieExtractionDuration];
    
	while (mIsExporting)
		[self exportOnMainThreadCallBack:nil];

bail:

    if (cloneHandle) DisposeHandle(cloneHandle);
    
    if (err) [self exportCompletedNotification:err];
    
	return err;
}

- (BOOL) isExporting
{
    return mIsExporting;
}

#pragma mark ---- private ----

// this callback is scheduled on the main thread - In order to keep from locking up the UI,
// it does one slice of export, writes it to file and then reschedule itself
-(void)exportOnMainThreadCallBack:(id)inObject
{
    BOOL continueExport = YES;
    
	OSStatus err;
    
	// prepare for extraction if this is the first entry
	if (NULL == mAudioExtractionSession) {
    
		err = [self configureExtractionSessionWithMovie: [mQTMovie quickTimeMovie]];
        if (err) goto bail;
	}
     
    // create the file
    if (0 == mExportFileID) {
        err = AudioFileCreate(&mParentRef, (CFStringRef)mFileName, kAudioFileAIFFType, &mOutputASBD, 0, &mFileRef, &mExportFileID);
        if (err) goto bail;

		// set the channel labels we grabbed from the source
		if (NULL != mExtractionLayoutPtr) {
            err = AudioFileSetProperty(mExportFileID, 
                                       kAudioFilePropertyChannelLayout,
                                       mExtractionLayoutSize,
                                       (void *)mExtractionLayoutPtr);
        	if (err) goto bail;
        }
    }
	
	// on entry if there's no samples left we're done
    if (mSamplesRemaining == 0) mExtractionComplete = YES;
    
    // perform some extraction
    if (!mExtractionComplete) {
    
        // if the client implemented a progress proc. call it now
        if (TRUE == mDelegateShouldContinueOp) {
        	NSNumber *progressValue = [NSNumber numberWithFloat:(float)((float)mSamplesCompleated / (float)mTotalNumberOfSamples)];
            
            [mProgressInfo setPhase:AIFFWriterExportPercent];
            [mProgressInfo setProgressValue:progressValue];
            [mProgressInfo setExportStatus:nil];
            
            continueExport = [[self delegate] shouldContinueOperationWithProgressInfo:mProgressInfo];
            if (NO == continueExport) { err = userCanceledErr; }
        }
    
        // read numSamplesThisSlice number of samples
        SInt64 numSamplesThisSlice = mSamplesRemaining;
        
        if ((numSamplesThisSlice > kMaxExtractionPacketCount) || (numSamplesThisSlice == -1))
            numSamplesThisSlice = kMaxExtractionPacketCount;

        // extract the audio and write it to the file
        err = [self extractAudioToFile:&numSamplesThisSlice];	
        if (err) goto bail;
        
        if (mSamplesRemaining != -1) {
            mSamplesRemaining -= numSamplesThisSlice;
            mSamplesCompleated += numSamplesThisSlice;
            
            if (mSamplesRemaining == 0) mExtractionComplete = YES;
        }
    }

bail:
	if (err || mExtractionComplete) {
	
        // we're done either way so close the file
        if (mExportFileID) AudioFileClose(mExportFileID);
        
        if (err && mExportFileID) {
            // if we erred out, delete the file
            FSDeleteObject(&mFileRef);
            mExportFileID = 0;
        }
        
		// call the completion routine to clean up
        [self exportCompletedNotification:[NSError errorWithDomain:NSOSStatusErrorDomain code:err userInfo:nil]];
        
	}
	/* else {
    
		// reschedule to perform this routine again on the next run loop cycle
		[self performSelectorOnMainThread:@selector(exportOnMainThreadCallBack:)
										  withObject:(id)nil
										  waitUntilDone:NO];
	}*/
}

// this method will be performed on a background thread
- (void)exportExtractionOnWorkerThread:(id)inObject
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    BOOL continueExport = YES;
	
    OSStatus err;

	[NSThread setThreadPriority:[NSThread threadPriority]+.1];
    
	// attach the movie to this thread
	err = EnterMoviesOnThread(0);
	if (err) goto bail;;
	
	err = AttachMovieToCurrentThread(mCloneMovie);
	if (err) goto bail;
    
	// prepare for extraction
	if (NULL == mAudioExtractionSession) {
		 
		err = [self configureExtractionSessionWithMovie:mCloneMovie];
        if (err) goto done;
	}
     
    // create the file
    if (0 == mExportFileID) {
        err = AudioFileCreate(&mParentRef, (CFStringRef)mFileName, kAudioFileAIFFType, &mOutputASBD, 0, &mFileRef, &mExportFileID);
        if (err) goto done;

		// set the channel labels we grabbed from the source
		if (NULL != mExtractionLayoutPtr) {
            err = AudioFileSetProperty(mExportFileID, 
                                       kAudioFilePropertyChannelLayout,
                                       mExtractionLayoutSize,
                                       (void *)mExtractionLayoutPtr);
        	if (err) goto done;
        }
    }
    
    // loop until stopped from an external event, or finished the entire extraction
	while (YES == continueExport && NO == mExtractionComplete) {
	
        if (mSamplesRemaining == 0) mExtractionComplete = YES;
        
        if (!mExtractionComplete) {
        
            // if the client implemented a progress proc. call it now we wait for the
            // progress fuction to return before continuing so we can check the return code
            if (TRUE == mDelegateShouldContinueOp) {
                NSNumber *progressValue = [NSNumber numberWithFloat:(float)((float)mSamplesCompleated / (float)mTotalNumberOfSamples)];
                
                [mProgressInfo setPhase:AIFFWriterExportPercent];
                [mProgressInfo setProgressValue:progressValue];
                [mProgressInfo setExportStatus:nil];
                
                [[self delegate] performSelectorOnMainThread:@selector(shouldContinueOperationWithProgressInfo:)
                                                withObject:(id)mProgressInfo
												waitUntilDone:YES];
                
                continueExport = [[self delegate] shouldContinueOperationWithProgressInfo:mProgressInfo];
                if (NO == continueExport) { err = userCanceledErr; break; }
            }
        
            // read numSamplesThisSlice number of samples
            SInt64 numSamplesThisSlice = mSamplesRemaining;
            
            if ((numSamplesThisSlice > kMaxExtractionPacketCount) || (numSamplesThisSlice == -1))
                numSamplesThisSlice = kMaxExtractionPacketCount;

            // extract the audio and write it to the file
            err = [self extractAudioToFile:&numSamplesThisSlice];	
            if (err) break;
            
            if (mSamplesRemaining != -1) {
                mSamplesRemaining -= numSamplesThisSlice;
                mSamplesCompleated += numSamplesThisSlice;
            }
        }
    }

done:

    // detach the exported movie from this thread
	DetachMovieFromCurrentThread(mCloneMovie);
    ExitMoviesOnThread(); 
    
    if (mExportFileID) AudioFileClose(mExportFileID);
 	
    if (err && mExportFileID) {
    	// if we erred out, delete the file
        FSDeleteObject(&mFileRef);
        mExportFileID = 0;
    }

bail:
	// call the completion routine to clean up on the main thread
	[self performSelectorOnMainThread:@selector(exportCompletedNotification:)
                                                withObject:(id)[NSError errorWithDomain:NSOSStatusErrorDomain code:err userInfo:nil]
												waitUntilDone:NO];
	
    [pool release];
}

// extract a slice of PCM audio and write it to an AIFF file - audio extraction proceeds serially
// from the last position, 'ploc' specifies the file offset that this buffer should be written to
// could be optimized by supplying a buffer, but for now it is simply allocated and released in each call
- (OSStatus)extractAudioToFile:(SInt64 *)ioNumSamples
{
	AudioBufferList	bufList;		
	UInt32			bufsize;
	char			*buffer = NULL;
	UInt32			flags;
	UInt32			numFrames;
    
    OSStatus		err;

	numFrames = *ioNumSamples;
	
	bufsize = (numFrames * mOutputASBD.mBytesPerFrame);
	buffer = (char *)malloc(bufsize);
	if (NULL == buffer) {
		err = memFullErr;
		goto bail;
	}

	// always extract interleaved data, since that's all we can write to an AIFF file
	bufList.mNumberBuffers = 1;
	bufList.mBuffers[0].mNumberChannels = mOutputASBD.mChannelsPerFrame;
	bufList.mBuffers[0].mDataByteSize = bufsize;
	bufList.mBuffers[0].mData = buffer;

	// read the number of requested samples from the movie
	err = MovieAudioExtractionFillBuffer(mAudioExtractionSession, &numFrames, &bufList, &flags);
	if (err) goto bail;

	// write it to the file
	if (numFrames > 0) {
		err = AudioFileWritePackets(mExportFileID,
        							false,
                                    numFrames * mOutputASBD.mBytesPerPacket,
									NULL,
                                    mLocationInFile,
                                    &numFrames,
                                    buffer);
		if (err) goto bail;							
		
        mLocationInFile += numFrames;
	}
		
bail:
	if (NULL != buffer) free(buffer);
	
    if (err) numFrames = 0;
    
	*ioNumSamples = numFrames;
	
    mExtractionComplete = (flags & kQTMovieAudioExtractionComplete);
	
    return err;
}

// get the default extraction layout for this movie, expanded into individual channel descriptions
// NOTE: the channel layout returned by this routine must be deallocated by the client
// If 'asbd' is non-NULL, fill it with the default extraction asbd, which contains the
// highest sample rate among the sound tracks that will be contributing.
//	'outLayoutSize' and 'asbd' may be nil.
- (OSStatus)getDefaultExtractionInfo
{
	OSStatus err;
	
	// get the size of the extraction output layout
	err = MovieAudioExtractionGetPropertyInfo(mAudioExtractionSession,
                                              kQTPropertyClass_MovieAudioExtraction_Audio,
											  kQTMovieAudioExtractionAudioPropertyID_AudioChannelLayout,
											  NULL,
                                              &mExtractionLayoutSize,
                                              NULL);
	if (err) goto bail;

	// allocate memory for the layout
	mExtractionLayoutPtr = (AudioChannelLayout *)calloc(1, mExtractionLayoutSize);
	if (NULL == mExtractionLayoutPtr)  { err = memFullErr; goto bail; }

	// get the layout for the current extraction configuration
	err = MovieAudioExtractionGetProperty(mAudioExtractionSession,
                                          kQTPropertyClass_MovieAudioExtraction_Audio,
										  kQTMovieAudioExtractionAudioPropertyID_AudioChannelLayout,
										  mExtractionLayoutSize,
                                          mExtractionLayoutPtr,
                                          NULL);
	if (err) goto bail;
	
    // get the audio stream basic description
    err = MovieAudioExtractionGetProperty(mAudioExtractionSession,
                                          kQTPropertyClass_MovieAudioExtraction_Audio,
                                          kQTMovieAudioExtractionAudioPropertyID_AudioStreamBasicDescription,
                                          sizeof(AudioStreamBasicDescription),
                                          &mSourceASBD,
                                          NULL);    

bail:
	
	return err;				
}

// this method prepare the specified movie for extraction by opening an extraction session, configuring
// and setting the output ASBD and the output layout if one exists - it also sets the start time to 0
// and calculates the total number of samples to export
- (OSStatus) configureExtractionSessionWithMovie:(Movie)inMovie
{
	OSStatus err;
	
	// open a movie audio extraction session
	err = MovieAudioExtractionBegin(inMovie, 0, &mAudioExtractionSession);
	if (err) goto bail;
	
    err = [self getDefaultExtractionInfo];
	if (err) goto bail;
    
    // set the output ASBD to 16-bit interleaved PCM big-endian integers
    // we start with the default ASBD which has set the sample rate to the
    // highest rate among all audio tracks 
    mOutputASBD = mSourceASBD;
    mOutputASBD.mFormatID = kAudioFormatLinearPCM;
    mOutputASBD.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger |
                               kAudioFormatFlagIsBigEndian |
                               kAudioFormatFlagIsPacked;
    mOutputASBD.mFramesPerPacket = 1;
    mOutputASBD.mBitsPerChannel = 16;
    mOutputASBD.mBytesPerFrame = 2 * mOutputASBD.mChannelsPerFrame;
    mOutputASBD.mBytesPerPacket = 2 * mOutputASBD.mChannelsPerFrame;

	// set the extraction ASBD
	err = MovieAudioExtractionSetProperty(mAudioExtractionSession,
                                          kQTPropertyClass_MovieAudioExtraction_Audio,
                                          kQTMovieAudioExtractionAudioPropertyID_AudioStreamBasicDescription,
                                          sizeof(mOutputASBD),
                                          &mOutputASBD);
	if (err) goto bail;		

	// set the output layout
	if (mExtractionLayoutPtr) {
		err = MovieAudioExtractionSetProperty(mAudioExtractionSession,
                                              kQTPropertyClass_MovieAudioExtraction_Audio,
                                              kQTMovieAudioExtractionAudioPropertyID_AudioChannelLayout,
                                              mExtractionLayoutSize,
                                              mExtractionLayoutPtr);
		if (err) goto bail;
	}

    // set the extraction start time - we always start at zero, but you don't have to
	TimeRecord startTime = { 0, 0, GetMovieTimeScale(inMovie), GetMovieTimeBase(inMovie) };
	
   	err = MovieAudioExtractionSetProperty(mAudioExtractionSession,
    									  kQTPropertyClass_MovieAudioExtraction_Movie,
                                          kQTMovieAudioExtractionMoviePropertyID_CurrentTime,
                                          sizeof(TimeRecord), &startTime);
	if (err) goto bail;
    
    // set the number of total samples to export
    mSamplesRemaining = mMovieDuration ? (mMovieDuration * mOutputASBD.mSampleRate) : -1;
    mTotalNumberOfSamples = mSamplesRemaining;

bail:
    	
	return err;
}

// calculate the duration of the longest audio track in the movie
// if the audio tracks end at time N and the movie is much 
// longer we don't want to keep extracting - the API will happily
// return zeroes until it reaches the movie duration
-(void)setMovieExtractionDuration
{
    TimeValue maxDuration = 0;
    UInt8 i;

    SInt32 trackCount = GetMovieTrackCount([mQTMovie quickTimeMovie]);
    
    if (trackCount) {
        for (i = 1; i < trackCount + 1; i++) {
            Track aTrack = GetMovieIndTrackType([mQTMovie quickTimeMovie],
                                                i,
                                                SoundMediaType,
                                                movieTrackMediaType);
            if (aTrack) {
                TimeValue aDuration = GetTrackDuration(aTrack);
            
                if (aDuration > maxDuration) maxDuration = aDuration;
            }
        }
        
        mMovieDuration = (Float64)maxDuration / (Float64)GetMovieTimeScale([mQTMovie quickTimeMovie]);
    }
}

// this completion method gets called at the end of the extraction session or may be called
// earlier if an error has occured - its main purpose is to clean up the world so an AIFFWriter
// instance can be used over and over again
//
// in this particular case if we completed successfully we launch QTPlayer with the .aif file
// and if an error occurs we pass it back to the client though the progress info object
- (void) exportCompletedNotification:(NSError *)inError
{
    
	/*if (noErr == [inError code]) {
    	CFURLRef url = CFURLCreateFromFSRef(kCFAllocatorDefault, &mFileRef);
    	
        NSWorkspace *ws = [NSWorkspace sharedWorkspace];
        
		[ws openFile:[(NSURL *)url path] withApplication:@"QuickTime Player"];
        
        CFRelease(url);
    }*/
    
	if (mFileName) {
    	[mFileName release];
        mFileName = nil;
    }
    
	if (mAudioExtractionSession){
		MovieAudioExtractionEnd(mAudioExtractionSession);
        mAudioExtractionSession = NULL;
    }
    
    if (mQTMovie) {
    	[mQTMovie release];
        mQTMovie = nil;
    }
    
    mMovieDuration = 0;
    
    if (mCloneMovie) {
        DisposeMovie(mCloneMovie);
        mCloneMovie = NULL;
    }
    
    mExtractionComplete = NO;
    mIsExporting = NO;
    mLocationInFile = 0;
	mSamplesRemaining = 0;
    mSamplesCompleated = 0;
    mTotalNumberOfSamples = 0;
    
    if (mExtractionLayoutPtr) {
    	free(mExtractionLayoutPtr);
        mExtractionLayoutPtr = NULL;
    }
    
    mExtractionLayoutSize = 0;
   	mExportFileID = 0;
    
	[mLock unlock];

    // if the client implemented a progress proc. call it now
    if (TRUE == mDelegateShouldContinueOp) {
        
        [mProgressInfo setPhase:AIFFWriterExportEnd];
        [mProgressInfo setProgressValue:nil];
        [mProgressInfo setExportStatus:([inError code] ? inError : nil)];
        
        [[self delegate] shouldContinueOperationWithProgressInfo:mProgressInfo];
    }
}

#pragma mark ---- delegate ----

// methods to support setting up a delegate of this class
- (id)delegate
{
    return mDelegate;
}

- (void)setDelegate:(id)inDelegate
{
    mDelegate = inDelegate;
    
    mDelegateShouldContinueOp = [mDelegate respondsToSelector:@selector(shouldContinueOperationWithProgressInfo:)];
}

@end
