package gwj.dev.ffmpeg.videoEdit;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.AttributeSet;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import androidx.annotation.Nullable;
import java.io.File;
import java.util.ArrayList;

public class PreviewContainer extends LinearLayout {
  ArrayList<String> imageList = new ArrayList<>();

  private final int SINGLE_WIDTH = 150;

  public PreviewContainer(Context context) {
    super(context);
  }

  public PreviewContainer(Context context,
      @Nullable AttributeSet attrs) {
    super(context, attrs);
  }

  public PreviewContainer(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
    super(context, attrs, defStyleAttr);
  }

  public void loadImages(String pathDir) {
    File dir = new File(pathDir);
    String tmp[];
    if (dir.isDirectory()) {
      tmp = dir.list();
      for (int i = 0; i < tmp.length; i++) {
        imageList.add(tmp[i]);
      }
    }

    notifyChanged();
  }

  public void notifyChanged() {
    this.removeAllViews();
    ArrayList<String> imageList = caculateImages(480);
    if (imageList != null && imageList.size() > 0) {
      for (int i = 0; i < imageList.size(); i++) {
        ImageView imageView = new ImageView(getContext());
        imageView.setLayoutParams(new LinearLayout.LayoutParams(SINGLE_WIDTH, SINGLE_WIDTH));
        Bitmap bmp = BitmapFactory.decodeFile("/sdcard/zzjk/" + imageList.get(i));
        imageView.setImageBitmap(bmp);
        this.addView(imageView);
      }
    }

    ViewGroup.LayoutParams params = this.getLayoutParams();
    params.width = 480;
    this.setLayoutParams(params);
  }

  public ArrayList<String> caculateImages(int totalLen) {
    int imageNum = totalLen / SINGLE_WIDTH;
    if (totalLen % SINGLE_WIDTH > 0) {
      imageNum++;
    }

    int span = imageList.size() / imageNum;
    ArrayList<String> listShows = new ArrayList<>();
    int index = 0;
    while (index < imageList.size()) {
      listShows.add(imageList.get(index));
      index = index + span;
    }

    //添加最后一个图片
    listShows.add(imageList.get(imageList.size() - 1));
    return listShows;
  }
}
