package gwj.dev.ffmpeg.videoEdit;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.util.Log;
import android.widget.LinearLayout;
import android.widget.TextView;
import androidx.annotation.Nullable;

public class TimeLine extends TextView implements IHorizentalScale {

  String subs[] = new String[] { "" };

  /**
   * 默认20秒
   */
  private int length = 20;
  private int minDotSpace = 0;
  private float scaleDotSpace = 0;
  private int fSpace = 5;

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
    minDotSpace = SINGLE_WIDTH * 10 / 12;
    scaleDotSpace = minDotSpace;
  }

  public void setTimeLength(int len) {
    this.length = len;
    LinearLayout.LayoutParams params = (LinearLayout.LayoutParams) getLayoutParams();
    params.width = SINGLE_WIDTH * 10 * length;
    setLayoutParams(params);
  }

  Paint paint = new Paint();

  @Override
  protected void onDraw(Canvas canvas) {
    int j = 0;
    for (int i = 0; i < getWidth(); i = (int) (i + scaleDotSpace)) {
      //canvas.drawRect(new Rect(i, 20, i + 20, 40), paint);
      String text = drawfText(j++);
      paint.setTextSize(32);
      canvas.drawText(text, i, 35, paint);
    }
    super.onDraw(canvas);
  }

  /**
   * 0f  5f 10f  15f 20f 25f
   */
  private String drawfText(int i) {
    float frameNum = getFrameNumber(i);

    return frame2Time(frameNum);
    //if (i %2 ==0){
    //  return  (i/2 )* fSpace+ "f";
    //}
    //return "~";
  }

  private String frame2Time(float framNum) {

    if (framNum % 30 == 0) {
      return framNum / 30 + "s";
    }
    if (framNum % 1 == 0) {
      return framNum + "f";
    }
    return "~";
  }

  private float getFrameNumber(int i) {
    return (float) i * 5 / 2;
  }


  private long lastTouchId = -1;
  private int lastWidth = 0;

  @Override
  public void onHorizentalScal(long id, float porcent) {
    if (lastTouchId != id) {
      lastTouchId = id;
      lastWidth = this.getWidth();
    }
    //再次缩小后小于最小的值，则恢复默认
    if (porcent > 1) {
      float tmpSpace = scaleDotSpace * porcent;
      if (tmpSpace > minDotSpace) {
        scaleDotSpace = minDotSpace;
        setTimeLength(length);
        this.invalidate();
        return;
      }
    }

    scaleDotSpace = scaleDotSpace * porcent;
    if (scaleDotSpace < minDotSpace/2){
      scaleDotSpace = minDotSpace/2;
    }

    int nWidth = (int) (lastWidth * porcent);
    Log.e("tag", "new  width  = " + nWidth + " -----scale =  " + scaleDotSpace);
    LinearLayout.LayoutParams params = (LinearLayout.LayoutParams) getLayoutParams();
    params.width = nWidth;
    this.setLayoutParams(params);

    this.invalidate();
  }
}
