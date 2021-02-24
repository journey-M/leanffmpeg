package gwj.dev.ffmpeg.videoEdit;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.AttributeSet;
import android.util.Log;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import androidx.annotation.Nullable;
import com.bumptech.glide.Glide;
import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class VPreviewContainer extends LinearLayout implements IHorizentalScale {
  ArrayList<String> imageList = new ArrayList<>();
  ArrayList<String> listShows = new ArrayList<>();
  private List<ImageView> listImageViews = new ArrayList<>();

  private int length = 20;
  private int maxLength = 0;

  public VPreviewContainer(Context context) {
    super(context);
  }

  public VPreviewContainer(Context context,
      @Nullable AttributeSet attrs) {
    super(context, attrs);
  }

  public VPreviewContainer(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
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
  }

  public void notifyChanged() {
    this.removeAllViews();
    ArrayList<String> imageList = caculateImages(this.getWidth());
    if (imageList != null && imageList.size() > 0) {
      for (int i = 0; i < imageList.size(); i++) {
        ImageView imageView = getCachedImageView(i);
        imageView.setVisibility(VISIBLE);
        Glide.with(imageView).load("/sdcard/zzjk/" + imageList.get(i))
            .centerCrop().into(imageView);
        this.addView(imageView);
      }
    }
  }

  private ImageView getCachedImageView(int pos) {
    if (listImageViews.size() > pos) {
      return listImageViews.get(pos);
    }
    ImageView imageView = new ImageView(getContext());
    imageView.setLayoutParams(new LinearLayout.LayoutParams(SINGLE_WIDTH, SINGLE_WIDTH));
    this.listImageViews.add(imageView);
    return imageView;
  }

  public ArrayList<String> caculateImages(int totalLen) {
    if (totalLen <= 0) {
      totalLen = 1000;
    }
    listShows.clear();
    int imageNum = totalLen / SINGLE_WIDTH;
    if (totalLen % SINGLE_WIDTH > 0) {
      imageNum++;
    }

    Log.e("tag", "total-----" + imageNum);
    int span = imageList.size() / imageNum;
    if (span == 0){
      span++;
    }
    int index = 0;
    while (index < imageList.size()) {
      listShows.add(imageList.get(index));
      index = index + span;
    }

    //添加最后一个图片
    listShows.add(imageList.get(imageList.size() - 1));
    return listShows;
  }

  private long lastTouchId = -1;
  private int lastWidth = 0;

  @Override
  public void setTimeLength(int len) {
    this.length = len;
    LinearLayout.LayoutParams params = (LinearLayout.LayoutParams) getLayoutParams();
    maxLength = SINGLE_WIDTH * 10 * length;
    params.width = maxLength;
    setLayoutParams(params);
    Log.e("tag", "max width = " + maxLength);
  }

  @Override
  public void onHorizentalScal(long id, float porcent) {
    //float nWidth = width * porcent;
    //setWidth((int) nWidth);
    if (lastTouchId != id) {
      lastTouchId = id;
      lastWidth = this.getWidth();
    }

    int tmpLen = (int) (lastWidth * porcent);
    if(porcent > 1){
      if (tmpLen >  maxLength){
        tmpLen = maxLength;
      }
    }
    Log.e("tag", "new  width  = " + tmpLen);
    LayoutParams params = (LayoutParams) getLayoutParams();
    params.width = tmpLen;
    this.setLayoutParams(params);

    notifyChanged();
  }
}
