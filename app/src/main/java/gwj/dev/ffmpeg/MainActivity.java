package gwj.dev.ffmpeg;

import android.Manifest;
import android.annotation.TargetApi;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.TextView;


public class MainActivity extends AppCompatActivity implements View.OnClickListener{

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public native void decdoe(String input, String output);

    public native void playVideo(String input, Surface surface);

    public native void playAudio(String input, String output);


    // Used to load the 'native-lib' library on applica0tion startup.
    static {
        System.loadLibrary("native-lib");

        System.loadLibrary("avcodec");
        System.loadLibrary("avfilter");
        System.loadLibrary("avformat");
        System.loadLibrary("avutil");
        System.loadLibrary("swresample");
        System.loadLibrary("swscale");
    }

    private SurfaceView surfaceView;
    private SurfaceHolder mholder;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());

        surfaceView = findViewById(R.id.surface_view);
        mholder = surfaceView.getHolder();
        mholder.addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                Log.e("TAG","surfaceCreated");
//                playVideo();
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {

            }
        });
        findViewById(R.id.sample_change_format).setOnClickListener(this);
        findViewById(R.id.sample_play).setOnClickListener(this);
        findViewById(R.id.sample_play_audio).setOnClickListener(this);
        requestPermission();

    }

    @TargetApi(Build.VERSION_CODES.M)
    private void requestPermission() {
        if (ContextCompat.checkSelfPermission(this,
                Manifest.permission.WRITE_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE,
                    Manifest.permission.READ_EXTERNAL_STORAGE}, 1);

        }
    }

    String fileName = "VID_20180503_145004.mp4";
//    String fileName = "VID_20180503_144955.mp4";

    @Override
    public void onClick(View v) {
        switch (v.getId()){
            case R.id.sample_play:
                playVideo();
                break;
            case R.id.sample_change_format:
                changeToYuvFormat();
                break;
            case R.id.sample_play_audio:
                playAudio();
                break;
        }
    }

    private void playVideo(){
        new Thread(new Runnable() {
            @Override
            public void run() {
                String inputPath = "/sdcard/DCIM/Camera/"+fileName;
                Surface surface = surfaceView.getHolder().getSurface();
                playVideo(inputPath, surface);
            }
        }).start();

    }

    /**
     * 转换格式
     */
    private void changeToYuvFormat(){
        new Thread(new Runnable() {
            @Override
            public void run() {
                String inputPath = "/sdcard/DCIM/Camera/"+fileName;
                String outputPath = "/sdcard/output_n_yuv420p.yuv";
                decdoe(inputPath, outputPath);
            }
        }).start();
    }


    /**
     * 播放音乐
     */
    private void playAudio(){
        new Thread(new Runnable() {
            @Override
            public void run() {
                String inputPath = "/sdcard/DCIM/Camera/"+fileName;
                String outputPath = "/sdcard/output_n_audio.pcm";
                playAudio(inputPath, outputPath);
            }
        }).start();
    }
}
