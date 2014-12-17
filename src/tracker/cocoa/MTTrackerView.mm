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

// TODO: Implement doubleclick
// TODO: Implement/fix modifier keys

#import "MTTrackerView.h"

// Nice macro for in-lining shader source
#define GLSL(src) "#version 150 core\n" #src

@implementation MTTrackerView

// ---- Surface Dimensions ----
@synthesize pixelData;
@synthesize width;
@synthesize height;
@synthesize bpp;

// ----- OpenGL Variables -----
static BOOL textureReady = NO;

static GLuint textureName;
static GLuint vertexArrayName;
static GLuint vertexBufferName;
static GLuint elementBufferName;
static GLuint shaderProgramName;

static const GLfloat vertices[] =
{
//   __Vertex__     _Texture_
//  /          \   /         \
//    X      Y      U      V
	-1.0f,  1.0f,  0.0f,  0.0f, // top left
	 1.0f,  1.0f,  1.0f,  0.0f, // top right
	 1.0f, -1.0f,  1.0f,  1.0f, // bottom right
	-1.0f, -1.0f,  0.0f,  1.0f, // bottom left
};

static const GLuint elements[] =
{
	0, 1, 2,	// first triangle
	2, 3, 0,	// second triangle
};

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
}

// ----------------------------------------------
//  Initialises OpenGL VAO, VBO, EBO and shaders
// ----------------------------------------------
- (void)prepareOpenGL
{
	NSLog(@"Initialising OpenGL...");
	
	// Generate and bind VAO
	glGenVertexArrays(1, &vertexArrayName);
	glBindVertexArray(vertexArrayName);
	
	// Compile shaders
	shaderProgramName = [self compileShaders];
	glUseProgram(shaderProgramName);
	
	// Generate and bind VBO
	glGenBuffers(1, &vertexBufferName);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferName);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	
	// Generate and bind EBO
	glGenBuffers(1, &elementBufferName);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferName);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
	
	// Setup pointer to position vertices
	GLint posAttrib = glGetAttribLocation(shaderProgramName, "position");
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(posAttrib);
	
	// Setup pointer to texture coordinates vertices
	GLint texCoordAttrib = glGetAttribLocation(shaderProgramName, "texCoord");
	glVertexAttribPointer(texCoordAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(texCoordAttrib);
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
		 uniform sampler2D s;
		 in vec2 texCoord_out;
		 
		 out vec4 color;
		 
		 void main()
		 {
			 color = texture(s, texCoord_out);
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
	glGenTextures(1, &textureName);
	glBindTexture(GL_TEXTURE_2D, textureName);
	
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
		
		// Update texture
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_BGR, GL_UNSIGNED_BYTE, pixelData);
		
		// Draw surface quad from 2 triangles
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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
	PPPoint curPoint = [self translateMouseCoordinates:theEvent];
	
#if DEBUG
	NSLog(@"Mouse pressed at (%d, %d)", curPoint.x, curPoint.y);
#endif

	PPEvent myEvent(eLMouseDown, &curPoint, sizeof(PPPoint));
	RaiseEventSynchronized(&myEvent);
}

- (void)mouseUp:(NSEvent *)theEvent
{
	PPPoint curPoint = [self translateMouseCoordinates:theEvent];
	
#if DEBUG
	NSLog(@"Mouse released at (%d, %d)", curPoint.x, curPoint.y);
#endif
	
	PPEvent myEvent(eLMouseUp, &curPoint, sizeof(PPPoint));
	RaiseEventSynchronized(&myEvent);
}

- (void)rightMouseDown:(NSEvent *)theEvent
{
	PPPoint curPoint = [self translateMouseCoordinates:theEvent];
	
#if DEBUG
	NSLog(@"Right mouse pressed at (%d, %d)", curPoint.x, curPoint.y);
#endif
	
	PPEvent myEvent(eRMouseDown, &curPoint, sizeof(PPPoint));
	RaiseEventSynchronized(&myEvent);
}

- (void)rightMouseUp:(NSEvent *)theEvent
{
	PPPoint curPoint = [self translateMouseCoordinates:theEvent];
	
#if DEBUG
	NSLog(@"Right mouse released at (%d, %d)", curPoint.x, curPoint.y);
#endif
	
	PPEvent myEvent(eRMouseUp, &curPoint, sizeof(PPPoint));
	RaiseEventSynchronized(&myEvent);
}

- (void)scrollWheel:(NSEvent *)theEvent
{
	PPPoint curPoint = [self translateMouseCoordinates:theEvent];
	
#if DEBUG
	NSLog(@"Scroll wheel event: Delta x %.2f, y %.2f @ (%d,%d)", theEvent.scrollingDeltaX, theEvent.scrollingDeltaY, curPoint.x, curPoint.y);
#endif
	
	TMouseWheelEventParams mouseWheelParams;
	mouseWheelParams.pos.x = curPoint.x;
	mouseWheelParams.pos.y = curPoint.y;
	mouseWheelParams.delta = theEvent.scrollingDeltaY;
	
	PPEvent myEvent(eMouseWheelMoved, &mouseWheelParams, sizeof(TMouseWheelEventParams));
	RaiseEventSynchronized(&myEvent);
}

- (void)mouseMoved:(NSEvent *)theEvent
{
	PPPoint curPoint = [self translateMouseCoordinates:theEvent];
	
#if DEBUG
	NSLog(@"Mouse moved to (%d, %d)", curPoint.x, curPoint.y);
#endif
	
	PPEvent myEvent(eMouseMoved, &curPoint, sizeof(PPPoint));
	RaiseEventSynchronized(&myEvent);
}

- (void)mouseDragged:(NSEvent *)theEvent
{
	PPPoint curPoint = [self translateMouseCoordinates:theEvent];
	
#if DEBUG
	NSLog(@"Left mouse dragged to (%d, %d)", curPoint.x, curPoint.y);
#endif
	
	PPEvent myEvent(eLMouseDrag, &curPoint, sizeof(PPPoint));
	RaiseEventSynchronized(&myEvent);
}

- (void)rightMouseDragged:(NSEvent *)theEvent
{
	PPPoint curPoint = [self translateMouseCoordinates:theEvent];
	
#if DEBUG
	NSLog(@"Right mouse dragged to (%d, %d)", curPoint.x, curPoint.y);
#endif
	
	PPEvent myEvent(eRMouseDrag, &curPoint, sizeof(PPPoint));
	RaiseEventSynchronized(&myEvent);
}

#pragma mark Keyboard events
- (void)keyDown:(NSEvent *)theEvent
{
	pp_uint16 character = [theEvent.characters characterAtIndex:0];

	// Remap backspace
	if (character == NSDeleteCharacter)
		character = NSBackspaceCharacter;
	
	pp_uint16 chr[] = { [MTKeyTranslator toVK:theEvent.keyCode],
						[MTKeyTranslator toSC:theEvent.keyCode],
						 character};
	
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
						[theEvent.characters characterAtIndex:0] };
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
						[MTKeyTranslator toSC:theEvent.keyCode],
						0 };
	
	switch ([theEvent keyCode])
	{
		case kVK_Shift:
		case kVK_RightShift:
			keyDown = flags & NSShiftKeyMask ? YES : NO;
			break;
			
		case kVK_Command:
			keyDown = flags & NSCommandKeyMask ? YES : NO;
			break;
			
		case kVK_Option:
		case kVK_RightOption:
			keyDown = flags & NSAlternateKeyMask ? YES : NO;
			break;
			
		case kVK_Control:
		case kVK_RightControl:
			keyDown = flags & NSControlKeyMask ? YES : NO;
			break;
			
		case kVK_CapsLock:
			keyDown = YES;
			break;
	}
	
	PPEvent myEvent(keyDown ? eKeyDown : eKeyUp, &chr, sizeof(chr));
	
#if DEBUG
	NSLog(@"Modifier %s: Keycode=%d, VK=%d, SC=%d, Char=%c", keyDown ? "pressed" : "released", theEvent.keyCode, chr[0], chr[1], chr[2]);
#endif
	
	RaiseEventSynchronized(&myEvent);
}
@end
