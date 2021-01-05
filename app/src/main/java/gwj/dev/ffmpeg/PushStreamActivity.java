package gwj.dev.ffmpeg;

import android.Manifest;
import android.annotation.TargetApi;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.graphics.PixelFormat;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;


/**
 * 视频采集和推流的测试页面
 */

public class PushStreamActivity extends AppCompatActivity implements View.OnClickListener {


    // Used to load the 'native-lib' library on applica0tion startup.
    static {
        System.loadLibrary("native-lib");

        System.loadLibrary("avcodec");
        System.loadLibrary("avfilter");
        System.loadLibrary("avformat");
        System.loadLibrary("avutil");
        System.loadLibrary("swresample");
        System.loadLibrary("swscale");
    }

    private final String TAG = "PushStreamAct";

    private SurfaceView surfaceView;
    private SurfaceHolder mholder;
    private android.hardware.Camera mCamera;
    private android.hardware.Camera.PreviewCallback cameraCallback = null;
    private PushUtil pushUtil = new PushUtil();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_pusher);


        pushUtil.initStreamParams(500, 500, 480000, 25);


        surfaceView = findViewById(R.id.surface_view);
        mholder = surfaceView.getHolder();
        mholder.addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                Log.e("TAG", "surfaceCreated");


            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
            }


            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                // TODO Auto-generated method stub
                Log.i(TAG, "SurfaceHolder.Callback：Surface Destroyed");
                if (null != mCamera) {
                    mCamera.setPreviewCallback(null); //！！这个必须在前，不然退出出错
                    mCamera.stopPreview();
                    mCamera.release();
                    mCamera = null;
                }

            }
        });
        findViewById(R.id.btn_open_camera).setOnClickListener(this);
        findViewById(R.id.btn_start_push).setOnClickListener(this);
        requestPermission();

    }

    @TargetApi(Build.VERSION_CODES.M)
    private void requestPermission() {
        if (ContextCompat.checkSelfPermission(this,
                Manifest.permission.CAMERA)
                != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{Manifest.permission.CAMERA}, 1);

        }
    }


    boolean createH264File = false;

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_open_camera:
                Toast.makeText(PushStreamActivity.this, "open", Toast.LENGTH_SHORT).show();
                startPreview();

                break;
            case R.id.btn_start_push:
                createH264File = !createH264File;
                break;

        }
    }


    private void startPreview() {

        // TODO Auto-generated method stub
        mCamera = android.hardware.Camera.open();// 开启摄像头（2.3版本后支持多摄像头,需传入参数）
        try {
            Log.i(TAG, "SurfaceHolder.Callback：surface Created");

            setCameraDisplayOrientation(PushStreamActivity.this, 0, mCamera);

            setCameraParams(mCamera);
            setPreviewCallback(mCamera);

            mCamera.setPreviewDisplay(mholder);//set the surface to be used for live preview
            mCamera.startPreview();

        } catch (Exception ex) {
            if (null != mCamera) {
                mCamera.release();
                mCamera = null;
            }
            Log.i(TAG + "initCamera", ex.getMessage());
        }
    }

    /**
     * 设置 摄像头的角度
     *
     * @param activity 上下文
     * @param cameraId 摄像头ID（假如手机有N个摄像头，cameraId 的值 就是 0 ~ N-1）
     * @param camera   摄像头对象
     */
    public static void setCameraDisplayOrientation(Activity activity,
                                                   int cameraId, android.hardware.Camera camera) {

        android.hardware.Camera.CameraInfo info = new android.hardware.Camera.CameraInfo();
        //获取摄像头信息
        android.hardware.Camera.getCameraInfo(cameraId, info);
        int rotation = activity.getWindowManager().getDefaultDisplay().getRotation();
        //获取摄像头当前的角度
        int degrees = 0;
        switch (rotation) {
            case Surface.ROTATION_0:
                degrees = 0;
                break;
            case Surface.ROTATION_90:
                degrees = 90;
                break;
            case Surface.ROTATION_180:
                degrees = 180;
                break;
            case Surface.ROTATION_270:
                degrees = 270;
                break;
        }

        int result;
        if (info.facing == android.hardware.Camera.CameraInfo.CAMERA_FACING_FRONT) {
            //前置摄像头
            result = (info.orientation + degrees) % 360;
            result = (360 - result) % 360; // compensate the mirror
        } else {
            // back-facing  后置摄像头
            result = (info.orientation - degrees + 360) % 360;
        }
        camera.setDisplayOrientation(result);
    }

    /**
     * 设置相机参数
     */
    private void setCameraParams(android.hardware.Camera mCamera) {
        android.hardware.Camera.Parameters parameters = mCamera.getParameters();
        parameters.setPreviewSize(500, 500);
        parameters.setPictureSize(500, 500);
        parameters.setPictureFormat(PixelFormat.YCbCr_420_SP);
    }

    private void setPreviewCallback(android.hardware.Camera mCamera) {
        // 【获取视频预览帧的接口】
        cameraCallback = new android.hardware.Camera.PreviewCallback() {
            @Override
            public void onPreviewFrame(byte[] data, android.hardware.Camera camera) {
                //传递进来的data,默认是YUV420SP的
                Log.e(TAG, "数据的长度是 ---" + data.length);

//                byte[] yuv420 = new byte[500 * 500 *3/2];
//                YUV420SP2YUV420(data, yuv420, 500, 500);

                if (createH264File) {
                    pushUtil.encodeStream(data);
                }

            }// endonPriview
        };

        mCamera.setPreviewCallback(cameraCallback);
    }


    private OutputStream outputStream;

    private void write264file(byte[] data) {
        try {

            if (outputStream == null) {
                String inputPath = "/sdcard/" + "abc.h264";
                File f = new File(inputPath);
                if (!f.exists()) {
                    f.createNewFile();
                }
                outputStream = new FileOutputStream(f);

            }
            outputStream.write(data, 0, data.length);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }


}
