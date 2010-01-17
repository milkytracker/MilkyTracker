/*
    MilkyTracker - Android port
    Copyright (C) 2009 Sergiy Pylypenko <pelya> (generic code)
    Copyright (C) 2009 Guillaume Legris (MilkyTracker adaptations and improvements)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

package org.milkytracker;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.content.Context;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.PowerManager;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;

public class MainActivity extends Activity {

    public static String APPLICATION_NAME = "milkytracker";

    static {
	System.loadLibrary(APPLICATION_NAME);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
	super.onCreate(savedInstanceState);

	// Initialization of the native side
	initSDL();

	// Write a demo file on the SDCard, if available
	try {
	    writeResources(APPLICATION_NAME, R.raw.milky, "milky2.xm");
	} catch (IOException e) {
	    Log.w(APPLICATION_NAME, "Can't create file on the SDCard", e);
	}

    }

    private void writeResources(String directory, int resourceId, String name) throws IOException {

	File sdDirectory = new File("/sdcard");
	if (!sdDirectory.isDirectory()) {
	    throw new IOException(sdDirectory + " is not a directory");
	}

	File subDir = new File(sdDirectory, directory);
	if (!subDir.exists()) {
	    subDir.mkdir();
	}

	File resourceFile = null;
	resourceFile = new File(subDir, name);
	if (resourceFile.exists()) {
	    return;
	}
	resourceFile.createNewFile();

	InputStream is = getResources().openRawResource(resourceId);
	OutputStream os = new FileOutputStream(resourceFile);

	if (is != null) {
	    byte[] buffer = new byte[1000];
	    BufferedInputStream bis = new BufferedInputStream(is);
	    int numberOfBytesRead = 0;
	    while ((numberOfBytesRead = bis.read(buffer)) != -1) {
		os.write(buffer, 0, numberOfBytesRead);
	    }

	}

	os.flush();
	os.close();

    }

    public void initSDL() {
	mAudioThread = new AudioThread(this);
	mGLView = new DemoGLSurfaceView(this);
	setContentView(mGLView);
	// Receive keyboard events
	mGLView.setFocusableInTouchMode(true);
	mGLView.setFocusable(true);
	mGLView.requestFocus();
	PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
	wakeLock = pm.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK, APPLICATION_NAME);
	wakeLock.acquire();
    }

    @Override
    protected void onPause() {
	// TODO: if application pauses it's screen is messed up
	if (wakeLock != null)
	    wakeLock.release();
	super.onPause();
	if (mGLView != null)
	    mGLView.onPause();
    }

    @Override
    protected void onResume() {
	if (wakeLock != null)
	    wakeLock.acquire();
	super.onResume();
	if (mGLView != null)
	    mGLView.onResume();
    }

    @Override
    protected void onStop() {
	if (wakeLock != null)
	    wakeLock.release();
	if (mAudioThread != null) {
	    mAudioThread.interrupt();
	    try {
		mAudioThread.join();
	    } catch (java.lang.InterruptedException e) {
	    }
	    ;
	}
	if (mGLView != null)
	    mGLView.exitApp();
	super.onStop();
	finish();
    }

    @Override
    public boolean onKeyDown(int keyCode, final KeyEvent event) {
	// Overrides Back key to use in our app
	if (mGLView != null)
	    mGLView.nativeKey(keyCode, 1);
	if (keyCode == KeyEvent.KEYCODE_BACK)
	    onStop();
	return true;
    }

    @Override
    public boolean onKeyUp(int keyCode, final KeyEvent event) {
	if (mGLView != null)
	    mGLView.nativeKey(keyCode, 0);
	return true;
    }

    private DemoGLSurfaceView mGLView = null;
    private AudioThread mAudioThread = null;
    private PowerManager.WakeLock wakeLock = null;

}

class DemoRenderer implements GLSurfaceView.Renderer {

    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
	nativeInit();
    }

    public void onSurfaceChanged(GL10 gl, int w, int h) {
	//gl.glViewport(0, 0, w, h);
	nativeResize(w, h);
    }

    public void onDrawFrame(GL10 gl) {
	nativeRender();
    }

    public void exitApp() {
	nativeDone();
    };

    private static native void nativeInit();

    private static native void nativeResize(int w, int h);

    private static native void nativeRender();

    private static native void nativeDone();

}

class DemoGLSurfaceView extends GLSurfaceView {

    public DemoGLSurfaceView(Activity context) {
	super(context);
	mParent = context;
	mRenderer = new DemoRenderer();
	setRenderer(mRenderer);
    }

    @Override
    public boolean onTouchEvent(final MotionEvent event) {
	// TODO: add multitouch support (added in Android 2.0 SDK)
	int action = -1;
	if (event.getAction() == MotionEvent.ACTION_DOWN)
	    action = 0;
	if (event.getAction() == MotionEvent.ACTION_UP)
	    action = 1;
	if (event.getAction() == MotionEvent.ACTION_MOVE)
	    action = 2;

	//int action = 2; // Move

	if (action >= 0) {
	    int w = getWidth();
	    int h = getHeight();
	    //Log.v(MainActivity.APPLICATION_NAME, "onTouchEvent: w=" + w + " h= " + h);

	    // TODO Should not be hardcoded but retrieved from the native side (i.e add JNI methods)
	    int nativeW = 320;
	    int nativeH = 240;

	    int x = nativeW * ((int) event.getX()) / w;
	    int y = nativeH * ((int) event.getY()) / h;
	    //Log.v(MainActivity.APPLICATION_NAME, "onTouchEvent: ((int)event.getX())=" + ((int) event.getX()) + " ((int)event.getY())="
		//    + ((int) event.getY()));
	    //Log.v(MainActivity.APPLICATION_NAME, "onTouchEvent: x=" + x + " y=" + y);
	    nativeMouse(x, y, action);
	}
	return true;
    }

    public void exitApp() {
	mRenderer.exitApp();
    };

    @Override
    public boolean onKeyDown(int keyCode, final KeyEvent event) {
	nativeKey(keyCode, 1);
	return true;
    }

    @Override
    public boolean onKeyUp(int keyCode, final KeyEvent event) {
	nativeKey(keyCode, 0);
	return true;
    }

    DemoRenderer mRenderer;
    Activity mParent;

    public static native void nativeMouse(int x, int y, int action);

    public static native void nativeKey(int keyCode, int down);
}

class AudioThread extends Thread {

    private Activity mParent;
    private AudioTrack mAudio;
    private byte[] mAudioBuffer;
    private ByteBuffer mAudioBufferNative;

    public AudioThread(Activity parent) {
	mParent = parent;
	mAudio = null;
	mAudioBuffer = null;
	this.setPriority(Thread.MAX_PRIORITY);
	this.start();
    }

    @Override
    public void run() {
	while (!isInterrupted()) {
	    if (mAudio == null) {
		int[] initParams = nativeAudioInit();
		if (initParams == null) {
		    try {
			sleep(200);
		    } catch (java.lang.InterruptedException e) {
		    }
		} else {
		    int rate = initParams[0];
		    int channels = initParams[1];
		    channels = (channels == 1) ? AudioFormat.CHANNEL_CONFIGURATION_MONO : AudioFormat.CHANNEL_CONFIGURATION_STEREO;
		    int encoding = initParams[2];
		    encoding = (encoding == 1) ? AudioFormat.ENCODING_PCM_16BIT : AudioFormat.ENCODING_PCM_8BIT;
		    int bufSize = AudioTrack.getMinBufferSize(rate, channels, encoding);
		    if (initParams[3] > bufSize)
			bufSize = initParams[3];
		    mAudioBuffer = new byte[bufSize];
		    nativeAudioInit2(mAudioBuffer);
		    mAudio = new AudioTrack(AudioManager.STREAM_MUSIC, rate, channels, encoding, bufSize, AudioTrack.MODE_STREAM);
		    mAudio.play();
		}
	    } else {
		int len = nativeAudioBufferLock();
		if (len > 0)
		    mAudio.write(mAudioBuffer, 0, len);
		if (len < 0)
		    break;
		nativeAudioBufferUnlock();
	    }
	}
	if (mAudio != null) {
	    mAudio.stop();
	    mAudio.release();
	    mAudio = null;
	}
    }

    private static native int[] nativeAudioInit();

    private static native int nativeAudioInit2(byte[] buf);

    private static native int nativeAudioBufferLock();

    private static native int nativeAudioBufferUnlock();
}
