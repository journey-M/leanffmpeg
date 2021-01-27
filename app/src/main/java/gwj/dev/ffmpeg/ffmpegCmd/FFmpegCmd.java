package gwj.dev.ffmpeg.ffmpegCmd;

public class FFmpegCmd {

  static {
    System.loadLibrary("avdevice");
    System.loadLibrary("avutil");
    System.loadLibrary("avcodec");
    System.loadLibrary("swresample");
    System.loadLibrary("avformat");
    System.loadLibrary("swscale");
    System.loadLibrary("avfilter");
    System.loadLibrary("postproc");
    System.loadLibrary("ffcmd");
  }

  private static native int run(int cmdLen, String[] cmd);

  public static int runCmd(String[] cmd) {
    return run(cmd.length, cmd);
  }
}
