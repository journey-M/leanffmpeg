package gwj.dev.ffmpeg;

import android.graphics.Bitmap;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import gwj.dev.ffmpeg.videoEdit.VPreviewContainer;
import gwj.dev.ffmpeg.videoEdit.VideoAPI;
import java.util.ArrayList;

public class VideoEditFragment extends Fragment implements View.OnClickListener {

  private VideoAPI videoAPI;
  private LinearLayout previewContainer;
  private LinearLayout metarailContainer;
  private Button btnOpen;
  private SurfaceView surfacePreview;
  private SeekBar progressBar;
  private Button btnthumbs;
  private Button btn_jiazai;
  @Nullable
  @Override
  public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container,
      @Nullable Bundle savedInstanceState) {
    View view = LayoutInflater.from(getContext()).inflate(R.layout.fragment_video_edit, null);
    // Example of a call to a native method
    previewContainer = view.findViewById(R.id.video_edit_container);
    metarailContainer = view.findViewById(R.id.container);
    surfacePreview = view.findViewById(R.id.surface_preview);
    progressBar = view.findViewById(R.id.progress);
    btnOpen = view.findViewById(R.id.open);
    btnthumbs = view.findViewById(R.id.thumbs);
    btn_jiazai = view.findViewById(R.id.jiazai);
    btnOpen.setOnClickListener(this);
    btnthumbs.setOnClickListener(this);
    btn_jiazai.setOnClickListener(this);
    view.findViewById(R.id.play).setOnClickListener(this);
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
    return view;
  }

  private void init() {
    videoAPI = new VideoAPI();

    //videoAPI.openVideoFile("/sdcard/bb.mp4");
    videoAPI.openVideoFile("/sdcard/cc.mp4");
    //videoAPI.openVideoFile("/sdcard/a.mp4");


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
      case R.id.thumbs:
        videoAPI.createThumbs("", "");
        break;
      case R.id.jiazai:
        loadVideoInfos();
        break;
    }
  }

  private void preview() {
    Log.e(TAG, "before =" + System.currentTimeMillis());
    ArrayList<Bitmap> bitmaps = videoAPI.getVideoPreviews(0, 1, 20);
    Log.e(TAG, "after  =" + System.currentTimeMillis());
    if (bitmaps != null && bitmaps.size() > 0) {
      for (int i = 0; i < bitmaps.size(); i++) {
        ImageView imageView = new ImageView(getContext());
        imageView.setLayoutParams(new LinearLayout.LayoutParams(350, 350));
        imageView.setImageBitmap(bitmaps.get(i));
        previewContainer.addView(imageView);
      }
    }
  }

  private void play() {
    //videoAPI.openVideoFile()
  }

  private void loadVideoInfos(){

    VPreviewContainer previewContainer = new VPreviewContainer(getContext());

    metarailContainer.addView(previewContainer);
    previewContainer.loadImages("/sdcard/zzjk");

  }
}
