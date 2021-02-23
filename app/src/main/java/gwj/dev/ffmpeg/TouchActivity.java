package gwj.dev.ffmpeg;

import android.os.Bundle;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.FragmentManager;
import gwj.dev.ffmpeg.videoEdit.TouchContainer;

public class TouchActivity extends AppCompatActivity {

  private TouchContainer touchContainer;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_touch_view);

    touchContainer = findViewById(R.id.touch_container);

    
  }
}
