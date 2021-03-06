package gwj.dev.ffmpeg.videoEdit;

import android.content.Context;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

/**
 * 完整的 支持touch事件的view
 */
public class TouchContainer_back extends LinearLayout {
  LinearLayout container;

  public TouchContainer_back(Context context) {
    this(context, null);
  }

  public TouchContainer_back(Context context, AttributeSet attrs) {
    this(context, attrs, -1);
  }

  public TouchContainer_back(Context context, AttributeSet attrs, int defStyleAttr) {
    super(context, attrs, defStyleAttr);
    init();
  }

  private void init() {
    container = new LinearLayout(getContext());
    container.setOrientation(LinearLayout.VERTICAL);
    this.addView(container);
    addTimeLine();
    addTestViews();
  }

  private void addTimeLine() {
    TimeLine line = new TimeLine(getContext());
    container.addView(line);
  }

  private void addTestViews() {
    for (int i = 0; i < 8; i++) {
      TextView tvTest1 = new TextView(getContext());
      tvTest1.setText(
          "title 测试页title 测试页\"title 测试页\"title 测试页\"title 测试页\"title 测试页\"title 测试页\"title 测试页\"title 测试页\"title 测试页\"title 测试页\"title 测试页\"title 测试页\"title 测试页\"title 测试页\"title 测试页\"title 测试页\"title 测试页\"");
      container.addView(tvTest1);
    }
  }

  // 属性变量
  private float translationX; // 移动X
  private float translationY; // 移动Y
  private float scale = 1; // 伸缩比例
  private float rotation; // 旋转角度

  // 移动过程中临时变量
  private float actionX;
  private float actionY;
  private float spacing;
  private float degree;
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
        degree = getDegree(event);
        break;
      case MotionEvent.ACTION_MOVE:
        if (moveType == 1) {
          translationX = translationX + event.getRawX() - actionX;
          translationY = translationY + event.getRawY() - actionY;
          setTranslationX(translationX);
          setTranslationY(translationY);
          actionX = event.getRawX();
          actionY = event.getRawY();
        } else if (moveType == 2) {
          scale = scale * getSpacing(event) / spacing;
          setScaleX(scale);
          setScaleY(scale);
          rotation = rotation + getDegree(event) - degree;
          if (rotation > 360) {
            rotation = rotation - 360;
          }
          if (rotation < -360) {
            rotation = rotation + 360;
          }
          setRotation(rotation);
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

  // 取旋转角度
  private float getDegree(MotionEvent event) {
    //得到两个手指间的旋转角度
    //double delta_x = event.getX(0) - event.getX(1);
    //double delta_y = event.getY(0) - event.getY(1);
    //double radians = Math.atan2(delta_y, delta_x);
    //return (float) Math.toDegrees(radians);
    return 0;
  }
}
