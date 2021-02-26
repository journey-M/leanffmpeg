package gwj.dev.ffmpeg.videoEdit;

import android.graphics.Bitmap;
import android.view.Surface;
import java.util.ArrayList;

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

  public native ArrayList<String> createThumbs(String vPath, String outPath);

  public native void play(float time);

 public native void realTimePreview(Surface surface);

}
