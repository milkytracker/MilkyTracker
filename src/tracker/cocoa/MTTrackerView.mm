/*
 *  tracker/cocoa/MTTrackerView.mm
 *
 *  Copyright 2014 Dale Whinham
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

#import "MTTrackerView.h"

// Nice macro for in-lining shader source
#define GLSL(src) "#version 150 core\n" #src

// Width of border drawn when dragging and dropping
#define FOCUS_RING_WIDTH 4

@implementation MTTrackerView

// ---- Surface Dimensions ----
@synthesize pixelData;
@synthesize width;
@synthesize height;
@synthesize bpp;

// ----- OpenGL Variables -----
static BOOL textureReady = NO;

static GLuint uiVertexArrayName;
static GLuint uiVertexBufferName;
static GLuint uiTextureName;
static GLuint focusRingVertexArrayName;
static GLuint focusRingVertexBufferName;
static GLuint shaderProgramName;

static GLint texCoordAttrib = -1;
static GLint posAttrib = -1;
static GLint colorUniform = -1;

static const GLfloat uiVertices[] =
{
//   ___Pos.___     _Texture_
//  /          \   /         \
//    X      Y      U      V
	-1.0f, -1.0f,  0.0f,  1.0f, // bottom left
	-1.0f,  1.0f,  0.0f,  0.0f, // top left
	 1.0f, -1.0f,  1.0f,  1.0f, // bottom right
	 1.0f,  1.0f,  1.0f,  0.0f  // top right
};

static GLfloat focusRingVertices[] =
{
//    X      Y
	-1.0f, -1.0f,
	-0.9f, -0.9f,
	-1.0f,  1.0f,
	-0.9f,  0.9f,
	 1.0f,  1.0f,
	 0.9f,  0.9f,
	 1.0f, -1.0f,
	 0.9f, -0.9f,
	-1.0f, -1.0f,
	-0.9f, -0.9f
};

// --- Mouse/Key Variables ----
static const double MOUSE_REPEAT_DELAY			= 0.5;
static const double LEFT_MOUSE_REPEAT_INTERVAL	= 0.06;
static const double RIGHT_MOUSE_REPEAT_INTERVAL = 0.02;
static NSTimer* lMouseTimer;
static NSTimer* rMouseTimer;
static PPPoint curPoint;
static BOOL drawFocusRing;

// ----------------------------------------------
//  Use inverted view coordinates for mouse etc.
// ----------------------------------------------
- (BOOL)isFlipped
{
	return YES;
}

// ---------------------------------------------
//  Sets up OpenGL context upon UI construction
// ---------------------------------------------
- (void)awakeFromNib
{
	NSLog(@"Creating OpenGL context...");
	
	NSOpenGLPixelFormatAttribute attrs[] =
	{
		NSOpenGLPFADoubleBuffer,
		// Prevents GPU switching on dual-GPU machines
		NSOpenGLPFAAllowOfflineRenderers,
		NSOpenGLPFADepthSize, 24,
		// Switch to OpenGL 3.2
		NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
		0
	};
	
	NSOpenGLPixelFormat *pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
	
	if (!pf)
		NSLog(@"Failed to create OpenGL pixel format!");
	
	NSOpenGLContext* context = [[NSOpenGLContext alloc] initWithFormat:pf shareContext:nil];
	
	// Crash on legacy OpenGL function calls; assists with debugging
	CGLEnable((CGLContextObj) [context CGLContextObj], kCGLCECrashOnRemovedFunctions);

	// Apply pixel format and context
	[self setPixelFormat:pf];
	[self setOpenGLContext:context];
	
	// Enable Retina awareness
	[self setWantsBestResolutionOpenGLSurface:YES];
	
	// Register as a drag and drop receiver
	[self registerForDraggedTypes:[NSArray arrayWithObjects: NSFilenamesPboardType, nil]];
	
	// Register to receive frame resize notifications
	[self setPostsFrameChangedNotifications:YES];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(frameDidChangeNotification:)
												 name:NSViewFrameDidChangeNotification
											   object:self];
}

// --------------------------------------------------
//  Updates focus ring vertices if view size changes
// --------------------------------------------------
- (void)frameDidChangeNotification:(NSNotification *)notification
{
	[self updateFocusRingVertices];
}

// ----------------------------------------------
//  Initialises OpenGL VAO, VBO, EBO and shaders
// ----------------------------------------------
- (void)prepareOpenGL
{
	NSLog(@"Initialising OpenGL...");
	
	// Compile shaders
	shaderProgramName = [self compileShaders];
	glUseProgram(shaderProgramName);
	
	// Setup attributes
	texCoordAttrib = glGetAttribLocation(shaderProgramName, "texCoord");
	posAttrib = glGetAttribLocation(shaderProgramName, "position");
	colorUniform = glGetUniformLocation(shaderProgramName, "color");
	
	// Generate and bind UI VAO
	glGenVertexArrays(1, &uiVertexArrayName);
	glBindVertexArray(uiVertexArrayName);
	
	// Generate and bind VBO
	glGenBuffers(1, &uiVertexBufferName);
	glBindBuffer(GL_ARRAY_BUFFER, uiVertexBufferName);
	glBufferData(GL_ARRAY_BUFFER, sizeof uiVertices, uiVertices, GL_STATIC_DRAW);
	
	// Setup pointers to position and texture vertices
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof (GLfloat), 0);
	glVertexAttribPointer(texCoordAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof (GLfloat), (GLvoid*)(2 * sizeof (GLfloat)));
	glEnableVertexAttribArray(texCoordAttrib);
	glEnableVertexAttribArray(posAttrib);
	
	// Generate and bind focus ring VAO
	glGenVertexArrays(1, &focusRingVertexArrayName);
	glBindVertexArray(focusRingVertexArrayName);
	
	// Generate focus ring VBO
	glGenBuffers(1, &focusRingVertexBufferName);

	// Adjust focus ring vertices
	[self updateFocusRingVertices];
	
	// Setup pointer to position vertices
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 2 * sizeof (GLfloat), 0);
	glEnableVertexAttribArray(posAttrib);
}

// -------------------------------------------------------
//  Recalculates focus ring vertices based on view bounds
// -------------------------------------------------------
-(void)updateFocusRingVertices
{
	GLfloat innerX = 1.0f - FOCUS_RING_WIDTH / [self bounds].size.width;
	GLfloat innerY = 1.0f - FOCUS_RING_WIDTH / [self bounds].size.height;
	
	focusRingVertices[2] = focusRingVertices[6] = focusRingVertices[18] = -innerX;
	focusRingVertices[3] = focusRingVertices[15] = focusRingVertices[19] = -innerY;
	focusRingVertices[10] = focusRingVertices[14] = innerX;
	focusRingVertices[7] = focusRingVertices[11] = innerY;
	
	// Select VAO/VBO and update vertices
	glBindVertexArray(focusRingVertexArrayName);
	glBindBuffer(GL_ARRAY_BUFFER, focusRingVertexBufferName);
	glBufferData(GL_ARRAY_BUFFER, sizeof focusRingVertices, focusRingVertices, GL_STATIC_DRAW);
}

// ------------------------------------------------------
//  Compiles shaders and returns shader program identity
// ------------------------------------------------------
- (GLuint)compileShaders
{
	NSLog(@"Compiling shaders...");
	
	// Vertex shader program
	static const GLchar* vertexShaderSrc = GLSL
	(
		 in vec2 position;
		 in vec2 texCoord;
		 
		 out vec2 texCoord_out;
		 
		 void main()
		 {
			 gl_Position = vec4(position, 0.0f, 1.0f);
			 texCoord_out = texCoord;
		 }
	);
	
	// Fragment shader program
	static const GLchar* fragShaderSrc = GLSL
	(
		 in vec2 texCoord_out;
		 
		 out vec4 fragColor;
		 
		 uniform vec4 color;
		 uniform sampler2D s;
		 
		 void main()
		 {
			 fragColor = mix(texture(s, texCoord_out), color, color.w);
		 }
	);
	
	GLuint vertexShaderName;
	GLuint fragmentShaderName;
	GLuint program;
	
	// Compile vertex shader
	vertexShaderName = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderName, 1, &vertexShaderSrc, NULL);
	glCompileShader(vertexShaderName);
	
	// Compile fragment shader
	fragmentShaderName = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderName, 1, &fragShaderSrc, NULL);
	glCompileShader(fragmentShaderName);
	
	// Attach shaders to program and link
	program = glCreateProgram();
	glAttachShader(program, vertexShaderName);
	glAttachShader(program, fragmentShaderName);
	glLinkProgram(program);
	
	// We have a valid program; shaders no longer needed
	glDeleteShader(vertexShaderName);
	glDeleteShader(fragmentShaderName);
	return program;
}

// --------------------------------------------------------
//  Called by display device once we know dimensions of UI
// --------------------------------------------------------
- (void)initTexture
{
	// Generate a texture for display
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &uiTextureName);
	glBindTexture(GL_TEXTURE_2D, uiTextureName);
	
	// Texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	// Allocate texture storage
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);
	
	// Make OpenGL aware of row length for partial updates
	glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
	
	textureReady = YES;
}

// ---------------------------------------------------
//  View redrawing routine with optional 'dirty' area
// ---------------------------------------------------
- (void)drawRect:(NSRect)dirtyRect
{
	// Adjust viewport
	NSRect bounds = [self convertRectToBacking:[self bounds]];
	glViewport(0, 0, bounds.size.width, bounds.size.height);

	// Just blank the screen if we don't have valid pixel data/texture
	if (!pixelData || !textureReady)
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	
	else
	{
		// Clamp dirty size to w/h of the screen/texture
		GLint x = dirtyRect.origin.x;
		GLint y = dirtyRect.origin.y;
		GLint w = dirtyRect.size.width  > width  ? width  : dirtyRect.size.width;
		GLint h = dirtyRect.size.height > height ? height : dirtyRect.size.height;
		
		// Set skip value for partial update
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, y * width + x);
		
		glBindVertexArray(uiVertexArrayName);
		
		// Update texture
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_BGR, GL_UNSIGNED_BYTE, pixelData);
		
		// Draw surface quad from triangle strip
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		// Draw focus ring if necessary
		if (drawFocusRing)
		{
			// Set color uniform to system highlight color
			NSColor* lineColor = [[NSColor selectedControlColor] colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
			glUniform4f(colorUniform,  [lineColor redComponent], [lineColor greenComponent], [lineColor blueComponent], 1.0f);
			
			glBindVertexArray(focusRingVertexArrayName);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 10);
			
			// Zero color uniform so we use texture color again
			glUniform4f(colorUniform, 0.0f, 0.0f, 0.0f, 0.0f);
		}
	}
	
	// Flip buffers
	[[self openGLContext] flushBuffer];
}

// -----------------------------------------------------------------
//  Scales mouse coordinates with respect to actual view dimensions
// -----------------------------------------------------------------
- (PPPoint)translateMouseCoordinates:(NSEvent*)theEvent
{
	NSPoint flippedPoint = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	CGSize viewBounds = [self frame].size;
	
	// Scale point based on GUI pixel dimensions
	flippedPoint.x = flippedPoint.x / viewBounds.width * width;
	flippedPoint.y = flippedPoint.y / viewBounds.height * height;
	
	PPPoint p;
	p.x = roundf(flippedPoint.x);
	p.y = roundf(flippedPoint.y);
	
	// Clip coords to window bounds
	if (p.x > width)
		p.x = width;
	else if (p.x < 0)
		p.x = 0;
	
	if (p.y > height)
		p.y = height;
	else if (p.y < 0)
		p.y = 0;
	
	return p;
}

#pragma mark Mouse events
- (void)mouseDown:(NSEvent *)theEvent
{
	lMouseTimer = [NSTimer scheduledTimerWithTimeInterval:MOUSE_REPEAT_DELAY target:self selector:@selector(mouseHeld:) userInfo:nil repeats:NO];
	curPoint = [self translateMouseCoordinates:theEvent];
	
#if DEBUG
	NSLog(@"Left mouse pressed at (%d, %d)", curPoint.x, curPoint.y);
#endif

	PPEvent myEvent(eLMouseDown, &curPoint, sizeof (PPPoint));
	RaiseEventSynchronized(&myEvent);
}

- (void)mouseUp:(NSEvent *)theEvent
{
	// Cancel repeat timer
	[lMouseTimer invalidate];
	lMouseTimer = nil;
	curPoint = [self translateMouseCoordinates:theEvent];
	
	// Use OS double click tracking (respects user's double click speed setting)
	if (theEvent.clickCount == 2)
	{
#if DEBUG
		NSLog(@"Left mouse double clicked at (%d, %d)", curPoint.x, curPoint.y);
#endif
		PPEvent myEvent(eLMouseDoubleClick, &curPoint, sizeof (PPPoint));
		RaiseEventSynchronized(&myEvent);
	}
	
#if DEBUG
	NSLog(@"Left mouse released at (%d, %d)", curPoint.x, curPoint.y);
#endif
	
	PPEvent myEvent(eLMouseUp, &curPoint, sizeof (PPPoint));
	RaiseEventSynchronized(&myEvent);
}

- (void)mouseHeld:(NSTimer *)timer
{
	lMouseTimer = [NSTimer scheduledTimerWithTimeInterval:LEFT_MOUSE_REPEAT_INTERVAL target:self selector:@selector(mouseHeld:) userInfo:nil repeats:NO];
	PPEvent myEvent(eLMouseRepeat, &curPoint, sizeof (PPPoint));
	RaiseEventSynchronized(&myEvent);
}

- (void)rightMouseDown:(NSEvent *)theEvent
{
	rMouseTimer = [NSTimer scheduledTimerWithTimeInterval:MOUSE_REPEAT_DELAY target:self selector:@selector(rightMouseHeld:) userInfo:nil repeats:NO];
	curPoint = [self translateMouseCoordinates:theEvent];
	
	// Use OS double click tracking (respects user's double click speed setting)
	if (theEvent.clickCount == 2)
	{
#if DEBUG
		NSLog(@"Right mouse double clicked at (%d, %d)", curPoint.x, curPoint.y);
#endif
		PPEvent myEvent(eRMouseDoubleClick, &curPoint, sizeof(PPPoint));
		RaiseEventSynchronized(&myEvent);
	}
	
#if DEBUG
	NSLog(@"Right mouse pressed at (%d, %d)", curPoint.x, curPoint.y);
#endif
	
	PPEvent myEvent(eRMouseDown, &curPoint, sizeof(PPPoint));
	RaiseEventSynchronized(&myEvent);
}

- (void)rightMouseUp:(NSEvent *)theEvent
{
	// Cancel repeat timer
	[rMouseTimer invalidate];
	rMouseTimer = nil;
	curPoint = [self translateMouseCoordinates:theEvent];
	
#if DEBUG
	NSLog(@"Right mouse released at (%d, %d)", curPoint.x, curPoint.y);
#endif
	
	PPEvent myEvent(eRMouseUp, &curPoint, sizeof(PPPoint));
	RaiseEventSynchronized(&myEvent);
}

- (void)rightMouseHeld:(NSTimer *)timer
{
	rMouseTimer = [NSTimer scheduledTimerWithTimeInterval:RIGHT_MOUSE_REPEAT_INTERVAL target:self selector:@selector(rightMouseHeld:) userInfo:nil repeats:NO];
	PPEvent myEvent(eRMouseRepeat, &curPoint, sizeof(PPPoint));
	RaiseEventSynchronized(&myEvent);
}

- (void)otherMouseDown:(NSEvent *)theEvent
{
	PPEvent myEvent(eMMouseDown, &curPoint, sizeof(PPPoint));
	RaiseEventSynchronized(&myEvent);
}

- (void)otherMouseUp:(NSEvent *)theEvent
{
	PPEvent myEvent(eMMouseUp, &curPoint, sizeof(PPPoint));
	RaiseEventSynchronized(&myEvent);
}

- (void)scrollWheel:(NSEvent *)theEvent
{
	curPoint = [self translateMouseCoordinates:theEvent];
	
#if DEBUG
	NSLog(@"Scroll wheel event: Delta x %.2f, y %.2f @ (%d,%d)", theEvent.deltaX, theEvent.deltaY, curPoint.x, curPoint.y);
#endif
	
	TMouseWheelEventParams mouseWheelParams;
	mouseWheelParams.pos.x = curPoint.x;
	mouseWheelParams.pos.y = curPoint.y;
	mouseWheelParams.deltaX = -theEvent.deltaX;
	mouseWheelParams.deltaY = theEvent.deltaY;
	
	PPEvent myEvent(eMouseWheelMoved, &mouseWheelParams, sizeof(TMouseWheelEventParams));
	RaiseEventSynchronized(&myEvent);
}

- (void)mouseMoved:(NSEvent *)theEvent
{
	curPoint = [self translateMouseCoordinates:theEvent];
	
#if DEBUG
	NSLog(@"Mouse moved to (%d, %d)", curPoint.x, curPoint.y);
#endif
	
	PPEvent myEvent(eMouseMoved, &curPoint, sizeof(PPPoint));
	RaiseEventSynchronized(&myEvent);
}

- (void)mouseDragged:(NSEvent *)theEvent
{
	curPoint = [self translateMouseCoordinates:theEvent];
	
#if DEBUG
	NSLog(@"Left mouse dragged to (%d, %d)", curPoint.x, curPoint.y);
#endif
	
	PPEvent myEvent(eLMouseDrag, &curPoint, sizeof(PPPoint));
	RaiseEventSynchronized(&myEvent);
}

- (void)rightMouseDragged:(NSEvent *)theEvent
{
	curPoint = [self translateMouseCoordinates:theEvent];
	
#if DEBUG
	NSLog(@"Right mouse dragged to (%d, %d)", curPoint.x, curPoint.y);
#endif
	
	PPEvent myEvent(eRMouseDrag, &curPoint, sizeof(PPPoint));
	RaiseEventSynchronized(&myEvent);
}

#pragma mark Keyboard events
- (void)keyDown:(NSEvent *)theEvent
{
	pp_uint16 character = theEvent.characters.length > 0 ? [theEvent.characters characterAtIndex:0] : 0;

	// Remap backspace
	if (character == NSDeleteCharacter)
		character = NSBackspaceCharacter;
	
	pp_uint16 chr[] = { [MTKeyTranslator toVK:theEvent.keyCode],
						[MTKeyTranslator toSC:theEvent.keyCode],
						 character };
	
#if DEBUG
	NSLog(@"Key pressed: Keycode=%d, VK=%d, SC=%d, Char=%d (%c)", theEvent.keyCode, chr[0], chr[1], chr[2], chr[2]);
#endif
	
	// Is the key an ASCII character?
	if (chr[2] >= 32 && chr[2] <= 127)
	{
		// Yes: Send an eKeyChar event
		PPEvent myEvent(eKeyChar, &chr[2], sizeof(pp_uint16));
		RaiseEventSynchronized(&myEvent);
	}
	PPEvent myEvent(eKeyDown, &chr, sizeof(chr));
	RaiseEventSynchronized(&myEvent);
}

- (void)keyUp:(NSEvent *)theEvent
{
	pp_uint16 chr[] = { [MTKeyTranslator toVK:theEvent.keyCode],
						[MTKeyTranslator toSC:theEvent.keyCode],
						theEvent.characters.length > 0 ? [theEvent.characters characterAtIndex:0] : 0 };
#if DEBUG
	NSLog(@"Key released: Keycode=%d, VK=%d, SC=%d, Char=%c", theEvent.keyCode, chr[0], chr[1], chr[2]);
#endif
	
	PPEvent myEvent(eKeyUp, &chr, sizeof(chr));
	RaiseEventSynchronized(&myEvent);
}

- (void)flagsChanged:(NSEvent*)theEvent
{
	unsigned long flags = [theEvent modifierFlags];
	BOOL keyDown = NO;
	
	pp_uint16 chr[] = { [MTKeyTranslator toVK:theEvent.keyCode],
						[MTKeyTranslator toSC:theEvent.keyCode] };
	
	switch ([theEvent keyCode])
	{
		// Both Shift keys behave as modifiers
		case kVK_Shift:
		case kVK_RightShift:
			if (flags & NSShiftKeyMask)
			{
				keyDown = YES;
				setKeyModifier(KeyModifierSHIFT);
			}
			else
				clearKeyModifier(KeyModifierSHIFT);
			break;
			
		// Only Left Command is used as a modifier
		case kVK_Command:
			if (flags & NSCommandKeyMask)
				setKeyModifier(KeyModifierCTRL);
			else
				clearKeyModifier(KeyModifierCTRL);
			// Break omitted intentionally
		case kVK_RightCommand:
			keyDown = flags & NSCommandKeyMask ? YES : NO;
			break;
			
		// Only Left Option is used as a modifier
		case kVK_Option:
			if (flags & NSAlternateKeyMask)
				setKeyModifier(KeyModifierALT);
			else
				clearKeyModifier(KeyModifierALT);
			// Break omitted intentionally
		case kVK_RightOption:
			keyDown = flags & NSAlternateKeyMask ? YES : NO;
			break;
			
		case kVK_CapsLock:
			keyDown = YES;
			break;
			
		default:
			return;
	}
	
#if DEBUG
	NSLog(@"Modifier %s: Keycode=%d, VK=%d, SC=%d", keyDown ? "pressed" : "released", theEvent.keyCode, chr[0], chr[1]);
#endif
	
	PPEvent myEvent(keyDown ? eKeyDown : eKeyUp, &chr, sizeof(chr));
	RaiseEventSynchronized(&myEvent);
}

#pragma mark Drag and drop events
- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
	NSPasteboard* pboard = [sender draggingPasteboard];
	
	if ([[pboard types] containsObject:NSFilenamesPboardType])
	{
		drawFocusRing = YES;
		[self setNeedsDisplay: YES];
		return NSDragOperationGeneric;
	}
	
	return NSDragOperationNone;
}

- (void)draggingExited:(id<NSDraggingInfo>)sender
{
	drawFocusRing = NO;
	[self setNeedsDisplay: YES];
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
	NSPasteboard* pboard = [sender draggingPasteboard];
 
	if ([[pboard types] containsObject:NSFilenamesPboardType])
	{
		drawFocusRing = NO;
		[self setNeedsDisplay: YES];
		NSArray* files = [pboard propertyListForType:NSFilenamesPboardType];
		[[NSApp delegate] application:NSApp openFiles:files];
		return YES;
	}
	
	return NO;
}
@end
