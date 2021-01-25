package gwj.dev.ffmpeg;

import android.Manifest;
import android.annotation.TargetApi;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;

public class VideoEditActivity extends AppCompatActivity {

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

  private LinearLayout previewContainer;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_video_edit);

    // Example of a call to a native method
    TextView tv = (TextView) findViewById(R.id.sample_text);
    previewContainer = findViewById(R.id.video_edit_container);
  }

  private void init() {

  }
}
