package gwj.dev.ffmpeg;

import android.Manifest;
import android.annotation.TargetApi;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.view.Surface;
import android.view.View;
import android.widget.TextView;
import android.widget.VideoView;


public class MainActivity extends AppCompatActivity implements View.OnClickListener{

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public native void decdoe(String input, String output);

    public native void playVideo(String input, Surface surface);


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

    private VideoView videoView;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());

        videoView = findViewById(R.id.video_view);
        findViewById(R.id.sample_change_format).setOnClickListener(this);
        findViewById(R.id.sample_play).setOnClickListener(this);
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


    @Override
    public void onClick(View v) {
        switch (v.getId()){
            case R.id.sample_play:
                String inputPath = "/sdcard/DCIM/Camera/VID_20180428_144437.mp4";
                Surface surface = videoView.getHolder().getSurface();
                playVideo(inputPath, surface);
                break;
            case R.id.sample_change_format:
                changeToYuvFormat();
                break;
        }
    }

    /**
     * 转换格式
     */
    private void changeToYuvFormat(){
        new View.OnClickListener() {
            @Override
            public void onClick(View v) {
//                String rootPath = Environment.getExternalStorageDirectory().toString();
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        String inputPath = "/sdcard/DCIM/Camera/VID_20180428_144437.mp4";
                        String outputPath = "/sdcard/output_n_yuv420p.yuv";
                        decdoe(inputPath, outputPath);
                    }
                }).start();
            }
        };
    }

}
