package gwj.dev.ffmpeg;

import android.Manifest;
import android.content.res.Configuration;
import android.graphics.PixelFormat;
import android.hardware.Camera;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.Window;
import android.view.WindowManager;

import androidx.appcompat.app.AppCompatActivity;
import java.io.File;
import java.io.RandomAccessFile;

/**
 * 视频采集和推流的测试页面
 */

public class DemoH264Activity extends AppCompatActivity implements SurfaceHolder.Callback {

  private SurfaceView mSurfaceView = null;
  private SurfaceHolder mSurfaceHolder = null;
  private Camera mCamera = null;
  private boolean mPreviewRunning = false;

  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    getWindow().setFormat(PixelFormat.TRANSLUCENT);
    requestWindowFeature(Window.FEATURE_NO_TITLE);
    getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
        WindowManager.LayoutParams.FLAG_FULLSCREEN);
    setContentView(R.layout.activity_camera);
    mSurfaceView = (SurfaceView) this.findViewById(R.id.surface);
    mSurfaceHolder = mSurfaceView.getHolder();
    mSurfaceHolder.addCallback(this);
    mSurfaceHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);

    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
      requestPermissions(
          new String[] { Manifest.permission.CAMERA, Manifest.permission.WRITE_EXTERNAL_STORAGE },
          1);
    }
  }

  @Override
  public void surfaceChanged(SurfaceHolder holder, int format, int width,
      int height) {
    if (mPreviewRunning) {
      mCamera.stopPreview();
    }
    Camera.Parameters p = mCamera.getParameters();
    p.setPreviewSize(352, 288);
    mCamera.setPreviewCallback(new H264Encoder(352, 288));
    mCamera.setParameters(p);
    try {
      mCamera.setPreviewDisplay(holder);
    } catch (Exception ex) {
    }
    mCamera.startPreview();
    mPreviewRunning = true;
  }

  @Override
  public void surfaceCreated(SurfaceHolder holder) {
    mCamera = Camera.open();
  }

  @Override
  public void surfaceDestroyed(SurfaceHolder holder) {
    if (mCamera != null) {
      mCamera.setPreviewCallback(null);
      mCamera.stopPreview();
      mPreviewRunning = false;
      mCamera.release();
      mCamera = null;
    }
  }

  public void onConfigurationChanged(Configuration newConfig) {
    try {
      super.onConfigurationChanged(newConfig);
      if (this.getResources().getConfiguration().orientation
          == Configuration.ORIENTATION_LANDSCAPE) {
      } else if (this.getResources().getConfiguration().orientation
          == Configuration.ORIENTATION_PORTRAIT) {
      }
    } catch (Exception ex) {
    }
  }
}

class H264Encoder implements Camera.PreviewCallback {
  long encoder = 0;
  RandomAccessFile raf = null;
  byte[] h264Buff = null;

  static {
    System.loadLibrary("x264");
    System.loadLibrary("h264-lib");
  }

  private H264Encoder() {
  }

  public H264Encoder(int width, int height) {
    encoder = CompressBegin(width, height);
    h264Buff = new byte[width * height * 8];
    try {
      File file = new File("/sdcard/camera.h264");
      raf = new RandomAccessFile(file, "rw");
    } catch (Exception ex) {
      Log.v("System.out", ex.toString());
    }
  }

  protected void finalize() {
    CompressEnd(encoder);
    if (null != raf) {
      try {
        raf.close();
      } catch (Exception ex) {
        Log.v("System.out", ex.toString());
      }
    }
    try {
      super.finalize();
    } catch (Throwable e) {
      // TODO Auto-generated catch block
      e.printStackTrace();
    }
  }

  private native long CompressBegin(int width, int height);

  private native int CompressBuffer(long encoder, int type, byte[] in, int insize, byte[] out);

  private native int CompressEnd(long encoder);

  @Override
  public void onPreviewFrame(byte[] data, Camera camera) {

    int result = CompressBuffer(encoder, -1, data, data.length, h264Buff);
    try {
      if (result > 0) {
        raf.write(h264Buff, 0, result);
      }
    } catch (Exception ex) {
      Log.v("System.out", ex.toString());
    }
  }
}
