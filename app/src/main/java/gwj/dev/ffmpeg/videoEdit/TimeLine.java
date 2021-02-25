package gwj.dev.ffmpeg.videoEdit;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.widget.LinearLayout;
import android.widget.TextView;
import androidx.annotation.Nullable;

public class TimeLine extends TextView implements IHorizentalScale {

  public interface ITimeChangeListener {
    void onTimeChanged(SpanSize spanSize, float distance);
  }

  class SpanSize {

    int fspans[] = { 5, 10, 15, 30, 60, 90, 150, 300, 600 };

    int currentSize = 5;

    public int getCurrentSize() {
      return currentSize;
    }

    public int increateSpan() {
      int index = -1;
      for (int i = 0; i < fspans.length; i++) {
        if (fspans[i] == currentSize) {
          index = i;
          break;
        }
      }
      //最大则返回最大的
      if (index == fspans.length - 1) {
        return currentSize;
      }
      currentSize = fspans[index + 1];
      return currentSize;
    }

    public int decressSpan() {
      int index = -1;
      for (int i = 0; i < fspans.length; i++) {
        if (fspans[i] == currentSize) {
          index = i;
          break;
        }
      }
      //最大则返回最大的
      if (index == 0) {
        return currentSize;
      }
      currentSize = fspans[index - 1];
      return currentSize;
    }

    public boolean isMax() {
      return currentSize == fspans[fspans.length - 1];
    }

    public boolean isMin() {
      return currentSize == fspans[0];
    }

    public float nextScaleBigValueDis() {
      int fspans[] = { 5, 10, 15, 30, 60, 90, 150, 300, 600 };
      if (currentSize == 5
          || currentSize == 15
          || currentSize == 30
          || currentSize == 150
          || currentSize == 300) {
        return 0.5f;
      } else if (currentSize == 10 || currentSize == 60) {
        return 0.6666666f;
      } else if (currentSize == 90) {
        return 0.6f;
      }
      return 1.0f;
    }

    public float nextScaleSmallValueDis() {
      if (currentSize == 10
          || currentSize == 30
          || currentSize == 60
          || currentSize == 300
          || currentSize == 600) {
        return 0.5f;
      } else if (currentSize == 15 || currentSize == 90) {
        return 0.6666666f;
      } else if (currentSize == 150) {
        return 0.6f;
      }
      return 1.0f;
    }
  }

  /**
   * 默认20秒
   */
  private float totalTime = 20;
  private int standDotSpace = 0;
  private float scaleDotSpace = 0;
  private SpanSize spanSize = new SpanSize();
  private ITimeChangeListener lisntner;

  public TimeLine(Context context) {
    this(context, null);
  }

  public TimeLine(Context context, @Nullable AttributeSet attrs) {
    this(context, attrs, -1);
  }

  public TimeLine(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
    super(context, attrs, defStyleAttr);

    init();
  }

  private void init() {
    standDotSpace = SINGLE_WIDTH * NUM_IMAGES_EVERY_SECODN / NUM_DISTANCE_EVERY_SECODN;
    scaleDotSpace = standDotSpace;
  }

  public void setTotalTime(float len) {
    this.totalTime = len;
    LinearLayout.LayoutParams params = (LinearLayout.LayoutParams) getLayoutParams();
    params.width = (int) (SINGLE_WIDTH * NUM_IMAGES_EVERY_SECODN * totalTime);
    setLayoutParams(params);
  }

  Paint paint = new Paint();

  @Override
  protected void onDraw(Canvas canvas) {
    int j = 0;
    for (int i = 0; i < getWidth(); i = (int) (i + scaleDotSpace)) {
      String text = drawfText(j++);
      paint.setTextSize(32);
      paint.setTextAlign(Paint.Align.CENTER);
      canvas.drawText(text, i, 35, paint);
    }
    super.onDraw(canvas);
  }

  /**
   * 0f  5f 10f  15f 20f 25f
   */
  private String drawfText(int distanceNum) {
    if (distanceNum % 2 == 0) {
      if (distanceNum == 0) {
        return "0";
      }
      int framNumber = distanceNum * spanSize.getCurrentSize() / 2;
      if (framNumber >= 30) {
        if (framNumber % 30 == 0) {
          return framNumber / 30 + "s";
        }
      }
      return framNumber + "f";
    } else {
      return "~";
    }
  }

  private long lastTouchId = -1;
  private int lastWidth = 0;

  @Override
  public void onHorizentalScal(long id, float percent) {
    if (lastTouchId != id) {
      lastTouchId = id;
      lastWidth = this.getWidth();
    }

    if (percent > 1 && spanSize.isMin()) {
      return;
    }
    if (percent < 1 && spanSize.isMax()) {
      return;
    }

    //再次缩小后小于最小的值，则恢复默认
    float tmpSpace = scaleDotSpace * percent;
    float scale_value = spanSize.nextScaleBigValueDis();
    if (percent < 0) {
      scale_value = spanSize.nextScaleSmallValueDis();
    }
    //Log.e("tag", "next scale ==" +( percent <0 ? " small ->":" big ->" )+  scale_value);
    if (tmpSpace <= standDotSpace * scale_value) {
      scaleDotSpace = standDotSpace;
      spanSize.increateSpan();
    } else if (tmpSpace >= standDotSpace * (1 + scale_value)) {
      scaleDotSpace = standDotSpace;
      spanSize.decressSpan();
    } else {
      scaleDotSpace = tmpSpace;
    }

    int nWidth = (int) (lastWidth * percent);
    LinearLayout.LayoutParams params = (LinearLayout.LayoutParams) getLayoutParams();
    params.width = nWidth;
    this.setLayoutParams(params);
    this.invalidate();

    if (lisntner != null) {
      lisntner.onTimeChanged(spanSize, scaleDotSpace);
    }
  }

  public void setLisntner(ITimeChangeListener lisntner) {
    this.lisntner = lisntner;
  }
}
