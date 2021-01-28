package gwj.dev.ffmpeg.glplayer;

import android.content.Context;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class VideoSurface extends SurfaceView implements SurfaceHolder.Callback {
  private static final String TAG = "VideoSurface";

  static {
    System.loadLibrary("vglplayer");

    System.loadLibrary("x264");
    System.loadLibrary("avcodec");
    System.loadLibrary("avfilter");
    System.loadLibrary("avformat");
    System.loadLibrary("avutil");
    System.loadLibrary("swresample");
    System.loadLibrary("swscale");
  }

  public VideoSurface(Context context) {
    super(context);
    Log.v(TAG, "VideoSurface");

    getHolder().addCallback(this);
  }

  @Override
  public void surfaceChanged(SurfaceHolder holder, int format, int width,
      int height) {
    Log.v(TAG, "surfaceChanged, format is " + format + ", width is "
        + width + ", height is" + height);
    //nativeStopPlayer();
    //setSurface(holder.getSurface());
  }

  @Override
  public void surfaceCreated(SurfaceHolder holder) {
    Log.v(TAG, "surfaceCreated");
    setSurface(holder.getSurface(), "/sdcard/cc.mp4");
  }

  @Override
  public void surfaceDestroyed(SurfaceHolder holder) {
    Log.v(TAG, "surfaceDestroyed");
    //nativeStopPlayer();
  }

  public int pausePlayer() {
    return nativePausePlayer();
  }

  public int resumePlayer() {
    return nativeResumePlayer();
  }

  public int stopPlayer() {
    return nativeStopPlayer();
  }

  public native int setSurface(Surface view, String path);

  public native int nativePausePlayer();

  public native int nativeResumePlayer();

  public native int nativeStopPlayer();
}
