package gwj.dev.ffmpeg;

import android.Manifest;
import android.annotation.TargetApi;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceView;
import android.view.View;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;
import gwj.dev.ffmpeg.glplayer.PlayInOpenGlActivity;
import gwj.dev.ffmpeg.ffmpegCmd.FFmpegCmd;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

  /**
   * A native method that is implemented by the 'native-lib' native library,
   * which is packaged with this application.
   */
  public native String stringFromJNI();

  public native void decdoe(String input, String output);

  public native void playVideo(String input, Surface surface);

  public native void playVideoInPosixThread(String input, Surface surface);

  public native void playAudio(String input, String output);

  // Used to load the 'native-lib' library on applica0tion startup.
  static {
    System.loadLibrary("native-lib");
    System.loadLibrary("x264");
    System.loadLibrary("avcodec");
    System.loadLibrary("avfilter");
    System.loadLibrary("avformat");
    System.loadLibrary("avutil");
    System.loadLibrary("swresample");
    System.loadLibrary("swscale");
  }

  private SurfaceView surfaceView;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);

    surfaceView = findViewById(R.id.surface_view);
    findViewById(R.id.sample_change_format).setOnClickListener(this);
    findViewById(R.id.sample_play).setOnClickListener(this);
    findViewById(R.id.sample_play_c_thread).setOnClickListener(this);
    findViewById(R.id.sample_play_audio).setOnClickListener(this);
    findViewById(R.id.sample_push).setOnClickListener(this);
    findViewById(R.id.sample_edit_video).setOnClickListener(this);
    findViewById(R.id.sample_cut_video).setOnClickListener(this);
    findViewById(R.id.sample_play_ingles).setOnClickListener(this);
  }

  @Override
  protected void onResume() {
    super.onResume();
    requestPermission();
  }

  @TargetApi(Build.VERSION_CODES.M)
  private void requestPermission() {

    if (ContextCompat.checkSelfPermission(this,
        Manifest.permission.READ_EXTERNAL_STORAGE)
        != PackageManager.PERMISSION_GRANTED) {
      requestPermissions(new String[] {
          Manifest.permission.WRITE_EXTERNAL_STORAGE,
          Manifest.permission.READ_EXTERNAL_STORAGE
      }, 1);
    }
  }

  @Override
  public void onClick(View v) {
    switch (v.getId()) {
      case R.id.sample_play_ingles:
        playInGles();
        break;
      case R.id.sample_play:
        playVideo();
        break;
      case R.id.sample_change_format:
        changeToYuvFormat();
        break;
      case R.id.sample_play_audio:
        playAudio();
        break;
      case R.id.sample_play_c_thread:
        playVideoInCThread();
        break;
      case R.id.sample_push:
        pushVideoStream();
        break;
      case R.id.sample_edit_video:
        gotoVideoEdit();
        break;
      case R.id.sample_cut_video:
        ffmpegTest();
        break;
    }
  }

  private void playInGles() {
    Intent intent = new Intent(this, PlayInOpenGlActivity.class);
    startActivity(intent);
  }

  String fileName = "sdcard/cc.mp4";

  private void playVideo() {
    new Thread(new Runnable() {
      @Override
      public void run() {
        String inputPath = fileName;
        Surface surface = surfaceView.getHolder().getSurface();
        playVideo(inputPath, surface);
      }
    }).start();
  }

  private void playVideoInCThread() {
    String inputPath = fileName;
    Surface surface = surfaceView.getHolder().getSurface();
    playVideoInPosixThread(inputPath, surface);
  }

  /**
   * 转换格式
   */
  private void changeToYuvFormat() {
    new Thread(new Runnable() {
      @Override
      public void run() {
        String inputPath = fileName;
        String outputPath = "/sdcard/output_n_yuv420p.yuv";
        decdoe(inputPath, outputPath);
      }
    }).start();
  }

  /**
   * 播放音乐
   */
  private void playAudio() {
    new Thread(new Runnable() {
      @Override
      public void run() {
        String inputPath = fileName;
        String outputPath = "/sdcard/output_n_audio.pcm";
        playAudio(inputPath, outputPath);
      }
    }).start();
  }

  private void ffmpegTest() {
    new Thread() {
      @Override
      public void run() {
        long startTime = System.currentTimeMillis();
        String input = "/sdcard/a.mp3";
        String output = "/sdcard/aoutput.mp3";

        //String cmd = "ffmpeg -y -i %s -vn -acodec copy -ss %s -t %s %s";
        //String result = String.format(cmd, input, "00:00:10", "00:00:20", output);

        String result =
            "ffmpeg -ss 4 -i /sdcard/cc.mp4 -f image2 -r 1 -t 1 -s 256x256 /sdcard/a.png";
        FFmpegCmd.runCmd(result.split(" "));
        Log.d("FFmpegTest", "run: 耗时：" + (System.currentTimeMillis() - startTime));
      }
    }.start();
  }

  private void pushVideoStream() {
    Intent intent = new Intent(this, PushStreamActivity.class);
    startActivity(intent);
  }

  private void gotoVideoEdit() {
    Intent intent = new Intent(this, VideoEditActivity.class);
    startActivity(intent);
  }
}
