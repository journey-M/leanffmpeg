package gwj.dev.ffmpeg.videoEdit;

import android.graphics.Bitmap;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import java.util.ArrayList;
import java.util.List;

public class VideoAPI {

  static {
    System.loadLibrary("videoEdit-lib");

    System.loadLibrary("x264");
    System.loadLibrary("avcodec");
    System.loadLibrary("avfilter");
    System.loadLibrary("avformat");
    System.loadLibrary("avutil");
    System.loadLibrary("swresample");
    System.loadLibrary("swscale");
  }

  public native int openVideoFile(String path);

  public native ArrayList<Bitmap> getVideoPreviews(int start, int interval, int maxcount);

  public native void seekPreviewPostion(int pos, Surface surface);

  public native void play(float time);
}
