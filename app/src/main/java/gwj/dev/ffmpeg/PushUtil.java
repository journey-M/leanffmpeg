package gwj.dev.ffmpeg;

public class PushUtil {


    static {
        System.loadLibrary("x264");
        System.loadLibrary("push-lib");
    }

    public native void initStreamParams(int width, int height, int bitrate, int fps);

    public native void encodeStream(byte [] data);


}
