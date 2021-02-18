package gwj.dev.ffmpeg;

import android.graphics.Bitmap;
import android.nfc.Tag;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.SeekBar;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;
import gwj.dev.ffmpeg.videoEdit.VideoAPI;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;

public class VideoEditActivity extends AppCompatActivity implements View.OnClickListener {

  private VideoAPI videoAPI;
  private LinearLayout previewContainer;
  private Button btnOpen;
  private SurfaceView surfacePreview;
  private SeekBar progressBar;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_video_edit);

    // Example of a call to a native method
    previewContainer = findViewById(R.id.video_edit_container);
    surfacePreview = findViewById(R.id.surface_preview);
    progressBar = findViewById(R.id.progress);
    btnOpen = findViewById(R.id.open);
    btnOpen.setOnClickListener(this);
    findViewById(R.id.play).setOnClickListener(this);
    progressBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
      @Override
      public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

        int last = (int) (17.0f * progress / 100);
        videoAPI.seekPreviewPostion(last, surfacePreview.getHolder().getSurface());
      }

      @Override
      public void onStartTrackingTouch(SeekBar seekBar) {

      }

      @Override
      public void onStopTrackingTouch(SeekBar seekBar) {

      }
    });

    init();
  }

  private void init() {
    videoAPI = new VideoAPI();

    //videoAPI.openVideoFile("/sdcard/bb.mp4");
    videoAPI.openVideoFile("/sdcard/cc.mp4");
    //videoAPI.openVideoFile("/sdcard/a.mp4");

    //Bitmap.Config.ARGB_8888

  }

  private final String TAG = "VEDIT";

  @Override
  public void onClick(View v) {
    switch (v.getId()) {
      case R.id.play:
        play();
        break;
      case R.id.open:
        preview();
        break;
    }
  }

  private void preview() {
    Log.e(TAG, "before =" + System.currentTimeMillis());
    ArrayList<Bitmap> bitmaps = videoAPI.getVideoPreviews(0, 1, 20);
    Log.e(TAG, "after  =" + System.currentTimeMillis());
    if (bitmaps != null && bitmaps.size() > 0) {
      for (int i = 0; i < bitmaps.size(); i++) {
        ImageView imageView = new ImageView(this);
        imageView.setLayoutParams(new LinearLayout.LayoutParams(350, 350));
        imageView.setImageBitmap(bitmaps.get(i));
        previewContainer.addView(imageView);
      }
    }
  }

  private void play() {

    //videoAPI.openVideoFile()

  }
}
