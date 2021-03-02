package gwj.dev.ffmpeg;

import android.graphics.Bitmap;
import android.media.AudioAttributes;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.Toast;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import gwj.dev.ffmpeg.videoEdit.VPreviewContainer;
import gwj.dev.ffmpeg.videoEdit.VideoAPI;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class VideoEditFragment extends Fragment implements View.OnClickListener {

  private VideoAPI videoAPI;
  private LinearLayout previewContainer;
  private LinearLayout metarailContainer;
  private Button btnOpen;
  private SurfaceView surfacePreview;
  private SeekBar progressBar;
  private Button btnthumbs;
  private Button btn_jiazai;

  AudioTrack audioTrack;

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

  boolean ispaly = false;
  int totalSize = 0;
  private List<byte[]> listBytes = new ArrayList<>();

  private void init() {
    init_audio_track_player();



    //编辑器初始化
    videoAPI = new VideoAPI();
    videoAPI.setListener(new VideoAPI.IAudioDataCallback() {
      @Override
      public void onReceiveAudioData(int size, byte[] data) {

        if (totalSize < 4096 * 10){
          totalSize = totalSize + size;
          listBytes.add(data);
          return;
        }
        if (audioTrack != null && audioTrack.getState() == AudioTrack.STATE_INITIALIZED && !ispaly){
          byte[] b = new byte[4096*10];
          for(int i=0 ; i< listBytes.size(); i++){
            System.arraycopy(listBytes.get(0), 0, b, i*4096, 4096 );
            audioTrack.write(b, 0, totalSize);
            ispaly = true;
          }
        }
      }
    });

    //videoAPI.openVideoFile("/sdcard/bb.mp4");
    videoAPI.openVideoFile("/sdcard/cc.mp4");
    //videoAPI.openVideoFile("/sdcard/a.mp4");


  }

  private void init_audio_track_player(){
    // ************ 流播放 ************
    int SAMPLE_RATE_INHZ = 48000;
    int AUDIO_FORMAT = AudioFormat.ENCODING_PCM_16BIT;
    final int minBufferSize = AudioTrack.getMinBufferSize(SAMPLE_RATE_INHZ,
        AudioFormat.CHANNEL_OUT_STEREO, AUDIO_FORMAT);
    // 创建 AudioTrack 对象
    //audioTrack = new AudioTrack(
    //    new AudioAttributes.Builder()
    //        .setUsage(AudioAttributes.USAGE_MEDIA)
    //        .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
    //        .build(),
    //    new AudioFormat.Builder().setSampleRate(SAMPLE_RATE_INHZ)
    //        .setEncoding(AUDIO_FORMAT)
    //        .setChannelMask(AudioFormat.CHANNEL_OUT_STEREO)
    //        .build(),
    //    minBufferSize,
    //    AudioTrack.MODE_STREAM,
    //    AudioManager.AUDIO_SESSION_ID_GENERATE
    //);

    int nb_channals = 1;
    int channaleConfig;//通道数
    if (nb_channals == 1) {
      channaleConfig = AudioFormat.CHANNEL_OUT_MONO;
    } else if (nb_channals == 2) {
      channaleConfig = AudioFormat.CHANNEL_OUT_STEREO;
    }else {
      channaleConfig = AudioFormat.CHANNEL_OUT_MONO;
    }
    int buffersize=AudioTrack.getMinBufferSize(SAMPLE_RATE_INHZ,
        channaleConfig, AudioFormat.ENCODING_PCM_16BIT);
    audioTrack = new AudioTrack(AudioManager.STREAM_MUSIC,SAMPLE_RATE_INHZ,channaleConfig,
        AudioFormat.ENCODING_PCM_16BIT,buffersize,AudioTrack.MODE_STREAM);

    // 检查初始化是否成功
    if(audioTrack.getState() == AudioTrack.STATE_UNINITIALIZED){
      Toast.makeText(getActivity(),"AudioTrack初始化失败！",Toast.LENGTH_SHORT).show();
      return;
    }
    // 播放
    audioTrack.play();
    //子线程中文件流写入
    //new Handler().post(new Runnable() {
    //  @Override
    //  public void run() {
    //    try {
    //      final File file = new File("/sdcard/cc.pcm");
    //      FileInputStream fileInputStream = new FileInputStream(file);
    //      byte[] tempBuffer = new byte[minBufferSize];
    //      while (fileInputStream.available() > 0) {
    //        int readCount = fileInputStream.read(tempBuffer);
    //        if (readCount == AudioTrack.ERROR_INVALID_OPERATION ||
    //            readCount == AudioTrack.ERROR_BAD_VALUE) {
    //          continue;
    //        }
    //        if (readCount != 0 && readCount != -1) {
    //          audioTrack.write(tempBuffer, 0, readCount);
    //        }
    //      }
    //      fileInputStream.close();
    //    } catch (IOException ioe) {
    //      ioe.printStackTrace();
    //    }
    //  }
    //});
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
    //videoAPI.play(0);
    videoAPI.realTimePreview(surfacePreview.getHolder().getSurface());
  }

  private void loadVideoInfos(){

    VPreviewContainer previewContainer = new VPreviewContainer(getContext());

    metarailContainer.addView(previewContainer);
    previewContainer.loadImages("/sdcard/zzjk");

  }
}
