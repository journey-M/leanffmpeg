package gwj.dev.ffmpeg;

import android.os.Bundle;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.FragmentManager;

public class VideoEditActivity extends AppCompatActivity {

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_video_edit);

    findViewById(R.id.fragment_holder);

    FragmentManager manager = getSupportFragmentManager();
    manager.beginTransaction().replace(R.id.fragment_holder, new VideoEditFragment()).commit();
  }
}
