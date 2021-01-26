package gwj.dev.ffmpeg.videoEdit;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.widget.TextView;
import androidx.annotation.Nullable;

public class TimeLine extends TextView implements IHorizentalScale {

  String text =
      "1---2---3---1---2---3---1---2---3---1---2---3---1---2---3---1---2---3---1---2---3---4";

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
    //setText(text);
  }

  float newRatio = 1.0f;
  int space = 100;

  @Override
  public void onHorizentalScal(float porcent) {
    int width = getWidth();
    //float nWidth = width * porcent;
    //setWidth((int) nWidth);
    newRatio = porcent;

    paint.setAntiAlias(true);
    paint.setStrokeWidth(3);
    paint.setColor(Color.RED);
    paint.setTextSize(40.0f);
    paint.setTextAlign(Paint.Align.CENTER);
    this.invalidate();
  }

  Paint paint = new Paint();

  @Override
  protected void onDraw(Canvas canvas) {
    for (int i = 0; i < getWidth(); i = i + ((int) (space * newRatio))) {
      canvas.drawRect(new Rect(i, 20, i + 20, 40), paint);
    }
    super.onDraw(canvas);
  }
}
