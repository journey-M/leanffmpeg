package gwj.dev.ffmpeg;

import android.graphics.Bitmap;
import android.nfc.Tag;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
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

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_video_edit);

    // Example of a call to a native method
    previewContainer = findViewById(R.id.video_edit_container);
    btnOpen = findViewById(R.id.open);
    btnOpen.setOnClickListener(this);

    init();
  }

  private void init() {
    videoAPI = new VideoAPI();

    //videoAPI.openVideoFile("/sdcard/bb.mp4");
    videoAPI.openVideoFile("/sdcard/cc.mp4");
    //videoAPI.openVideoFile("/sdcard/a.mp4");
    //File fTest = new File("/sdcard/a.tt.txt");
    //if (!fTest.exists()) {
    //  try {
    //    fTest.createNewFile();
    //    FileOutputStream fos = new FileOutputStream(fTest);
    //    String content = "test nnnnnn";
    //    fos.write(content.getBytes());
    //    fos.close();
    //  } catch (IOException e) {
    //    e.printStackTrace();
    //  }
    //}
  }

  private final String TAG = "VEDIT";

  @Override
  public void onClick(View v) {
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
}
