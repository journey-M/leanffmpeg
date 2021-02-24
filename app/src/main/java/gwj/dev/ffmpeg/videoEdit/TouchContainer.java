package gwj.dev.ffmpeg.videoEdit;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;

public class TouchContainer extends LinearLayout {
  LinearLayout container;

  public TouchContainer(Context context) {
    this(context, null);
  }

  public TouchContainer(Context context, AttributeSet attrs) {
    this(context, attrs, -1);
  }

  public TouchContainer(Context context, AttributeSet attrs, int defStyleAttr) {
    super(context, attrs, defStyleAttr);
    init();
  }

  private void init() {
    container = new LinearLayout(getContext());
    container.setOrientation(LinearLayout.VERTICAL);
    LayoutParams params = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
        ViewGroup.LayoutParams.WRAP_CONTENT);
    this.addView(container, params);
    addTimeLine();
    addTestViews();
  }

  private void addTimeLine() {
    TimeLine line = new TimeLine(getContext());
    LayoutParams params = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
        ViewGroup.LayoutParams.WRAP_CONTENT);
    container.addView(line, params);

    line.setTimeLength(2);
  }

  private void addTestViews() {
    VPreviewContainer vpreview = new VPreviewContainer(getContext());
    LayoutParams params = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
        ViewGroup.LayoutParams.WRAP_CONTENT);
    container.addView(vpreview, params);
    vpreview.setTimeLength(2);

    vpreview.loadImages("/sdcard/zzjk");
    vpreview.notifyChanged();
  }

  void notifyScales(long id, float ratio) {
    View child;
    for (int i = 0; i < container.getChildCount(); i++) {
      child = container.getChildAt(i);
      if (child instanceof IHorizentalScale) {
        ((IHorizentalScale) child).onHorizentalScal(id, ratio);
      }
    }
  }

  // 属性变量
  private float scale = 1; // 伸缩比例

  // 移动过程中临时变量
  private float actionX;
  private float actionY;
  private float spacing;
  private long touchId = -1;
  private int moveType; // 0=未选择，1=拖动，2=缩放

  @Override
  public boolean onInterceptTouchEvent(MotionEvent ev) {
    getParent().requestDisallowInterceptTouchEvent(true);
    return super.onInterceptTouchEvent(ev);
  }

  @Override
  public boolean onTouchEvent(MotionEvent event) {
    switch (event.getAction() & MotionEvent.ACTION_MASK) {
      case MotionEvent.ACTION_DOWN:
        moveType = 1;
        actionX = event.getRawX();
        actionY = event.getRawY();
        break;
      case MotionEvent.ACTION_POINTER_DOWN:
        moveType = 2;
        spacing = getSpacing(event);
        touchId = System.currentTimeMillis();
        break;
      case MotionEvent.ACTION_MOVE:
        if (moveType == 1) {
          actionX = event.getRawX();
          actionY = event.getRawY();
        } else if (moveType == 2) {
          scale = getSpacing(event) / spacing;
          setSubLength(touchId, scale);
        }
        break;
      case MotionEvent.ACTION_UP:
      case MotionEvent.ACTION_POINTER_UP:
        moveType = 0;
    }
    return true;
    //return super.onTouchEvent(event);
  }

  // 触碰两点间距离
  private float getSpacing(MotionEvent event) {
    //通过三角函数得到两点间的距离
    float x = event.getX(0) - event.getX(1);
    float y = event.getY(0) - event.getY(1);
    return (float) Math.sqrt(x * x + y * y);
  }

  private void setSubLength(long id, float scale) {
    Log.e("tag", "scale ---- " + scale);
    notifyScales(id, scale);
  }
}
