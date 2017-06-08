package com.example.wanlinyang.navigator;

import android.app.PendingIntent;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ScrollView;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;

import com.hoho.android.usbserial.driver.CdcAcmSerialDriver;
import com.hoho.android.usbserial.driver.ProbeTable;
import com.hoho.android.usbserial.driver.UsbSerialDriver;
import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.driver.UsbSerialProber;
import com.hoho.android.usbserial.util.SerialInputOutputManager;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;


import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.WindowManager;
import android.widget.TextView;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;

import java.io.IOException;

import static android.graphics.Color.blue;
import static android.graphics.Color.green;
import static android.graphics.Color.red;
import static android.graphics.Color.rgb;


public class MainActivity extends Activity implements TextureView.SurfaceTextureListener{

    SeekBar myControl1;
    SeekBar myControl2;
    TextView myTextView;
    Button button;
    TextView myTextView2;
    ScrollView myScrollView;
    TextView myTextView3;

    private UsbManager manager;
    private UsbSerialPort sPort;
    private final ExecutorService mExecutor = Executors.newSingleThreadExecutor();
    private SerialInputOutputManager mSerialIoManager;

    private Camera mCamera;
    private TextureView mTextureView;
    private SurfaceView mSurfaceView;
    private SurfaceHolder mSurfaceHolder;
    private Bitmap bmp = Bitmap.createBitmap(640, 480, Bitmap.Config.ARGB_8888);
    private Canvas canvas = new Canvas(bmp);
    private Paint paint1 = new Paint();
    private TextView mTextView;

    int thresh;
    int[] COM = new int[10];
    int startY;
    static long prevtime = 0; // for FPS calculation

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON); // keeps the screen from turning off

        //myControl1 = (SeekBar) findViewById(R.id.seek1);
        myControl2 = (SeekBar) findViewById(R.id.seek2);

        //myTextView = (TextView) findViewById(R.id.textView01);
        //myTextView.setText("Enter whatever you Like!");

        //setMyControlListener1();
        setMyControlListener2();

        //myTextView2 = (TextView) findViewById(R.id.textView02);
        myScrollView = (ScrollView) findViewById(R.id.ScrollView01);
        myTextView3 = (TextView) findViewById(R.id.textView03);
        //button = (Button) findViewById(R.id.button1);
/*
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                myTextView2.setText("value on click is "+myControl1.getProgress());
                String sendString = String.valueOf(myControl1.getProgress()) + '\n';
                try {
                    sPort.write(sendString.getBytes(), 10); // 10 is the timeout
                } catch (IOException e) { }
            }

        });
*/
        manager = (UsbManager) getSystemService(Context.USB_SERVICE);

        if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED) {

            mSurfaceView = (SurfaceView) findViewById(R.id.surfaceview);
            mSurfaceHolder = mSurfaceView.getHolder();

            mTextureView = (TextureView) findViewById(R.id.textureview);
            mTextureView.setSurfaceTextureListener(this);

            mTextView = (TextView) findViewById(R.id.cameraStatus);
            myControl2 = (SeekBar) findViewById(R.id.seek2);

            // set the paintbrush for writing text on the image
            paint1.setColor(0xffff0000); // red
            paint1.setTextSize(24);

            setMyControlListener2();

            mTextView.setText("started camera");
        } else {
            mTextView.setText("no camera permissions");
        }


    }
/*
    private void setMyControlListener1() {
        myControl1.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

            int progressChanged = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                progressChanged = progress;
                myTextView.setText("The value is: "+progress);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }

        });
    }
*/
    private void setMyControlListener2() {
        myControl2.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

            int progressChanged = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                progressChanged = progress;
                thresh = progress;
                //myTextView.setText("The value is: "+progress);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }

        });
    }

    private final SerialInputOutputManager.Listener mListener =
            new SerialInputOutputManager.Listener() {
                @Override
                public void onRunError(Exception e) {

                }

                @Override
                public void onNewData(final byte[] data) {
                    MainActivity.this.runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            MainActivity.this.updateReceivedData(data);
                        }
                    });
                }
            };

    @Override
    protected void onPause(){
        super.onPause();
        stopIoManager();
        if(sPort != null){
            try{
                sPort.close();
            } catch (IOException e){ }
            sPort = null;
        }
        finish();
    }

    @Override
    protected void onResume() {
        super.onResume();

        ProbeTable customTable = new ProbeTable();
        customTable.addProduct(0x04D8,0x000A, CdcAcmSerialDriver.class);
        UsbSerialProber prober = new UsbSerialProber(customTable);

        final List<UsbSerialDriver> availableDrivers = prober.findAllDrivers(manager);

        if(availableDrivers.isEmpty()) {
            //check
            return;
        }

        UsbSerialDriver driver = availableDrivers.get(0);
        sPort = driver.getPorts().get(0);

        if (sPort == null){
            //check
        }else{
            final UsbManager usbManager = (UsbManager) getSystemService(Context.USB_SERVICE);
            UsbDeviceConnection connection = usbManager.openDevice(driver.getDevice());
            if (connection == null){
                //check
                PendingIntent pi = PendingIntent.getBroadcast(this, 0, new Intent("com.android.example.USB_PERMISSION"), 0);
                usbManager.requestPermission(driver.getDevice(), pi);
                return;
            }

            try {
                sPort.open(connection);
                sPort.setParameters(9600, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE);

            }catch (IOException e) {
                //check
                try{
                    sPort.close();
                } catch (IOException e1) { }
                sPort = null;
                return;
            }
        }
        onDeviceStateChange();
    }

    private void stopIoManager(){
        if(mSerialIoManager != null) {
            mSerialIoManager.stop();
            mSerialIoManager = null;
        }
    }

    private void startIoManager() {
        if(sPort != null){
            mSerialIoManager = new SerialInputOutputManager(sPort, mListener);
            mExecutor.submit(mSerialIoManager);
        }
    }

    private void onDeviceStateChange(){
        stopIoManager();
        startIoManager();
    }

    private void updateReceivedData(byte[] data) {
        //do something with received data

        //for displaying:
        String rxString = null;
        try {
            rxString = new String(data, "UTF-8"); // put the data you got into a string
            myTextView3.append(rxString);
            myScrollView.fullScroll(View.FOCUS_DOWN);
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
    }

    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        mCamera = Camera.open();
        Camera.Parameters parameters = mCamera.getParameters();
        parameters.setPreviewSize(640, 480);
        parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_INFINITY); // no autofocusing
        parameters.setAutoExposureLock(true); // keep the white balance constant
        mCamera.setParameters(parameters);
        mCamera.setDisplayOrientation(90); // rotate to portrait mode

        try {
            mCamera.setPreviewTexture(surface);
            mCamera.startPreview();
        } catch (IOException ioe) {
            // Something bad happened
        }
    }

    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
        // Ignored, Camera does all the work for us
    }

    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        mCamera.stopPreview();
        mCamera.release();
        return true;
    }

    public int abs(int n){
        if (n < 0){
            int m = -n;
            return -m;
        } else {
            return n;
        }
    }

    public int min(int r, int g, int b){
        int min = r;
        if(g < min){
            min = g;
        } else if (b < min){
            min = b;
        }
        return min;
    }

    // the important function
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {
        // every time there is a new Camera preview frame
        mTextureView.getBitmap(bmp);

        final Canvas c = mSurfaceHolder.lockCanvas();
        if (c != null) {
            int[][] pixels = new int[10][bmp.getWidth()]; // pixels[][] is the RGBA data
            int gap = 5;
            startY = 100;//bmp.getHeight()+50; // which row in the bitmap to analyze to read
            //COM = 0;  // center
            for(int i=0; i<10; i++){
                COM[i] = 0;
            }

            int sum_mr = 0;  // the sum of the mass times the radius
            int sum_m = 0;  // the sum of the masses

            for(int j = 0; j < 10; j++){
                int k = startY + j*5;
                bmp.getPixels(pixels[j], 0, bmp.getWidth(), 0, k, bmp.getWidth(), 1);
                for (int i = 0; i < bmp.getWidth(); i++){
                    //int gsr = abs(green(pixels[i])-red(pixels[i]));
                    //int gsb = abs(green(pixels[i])-blue(pixels[i]));
                    //int rsb = abs(red(pixels[i])-blue(pixels[i]));

                    //int rgb_av = (red(pixels[j][i])+green(pixels[j][i])+blue(pixels[j][i]))/3;
                    //int rgb_min = min(red(pixels[j][i]), green(pixels[j][i]), blue(pixels[j][i]));
                    //double saturation = 1 - rgb_min/rgb_av;
                    //double thresh_d = (double)thresh/100;

                    //if ( gsr<thresh && gsb<thresh && rsb<thresh && green(pixels[i])>(thresh*2) ){   // gray, and brightness
                    //if (saturation < thresh_d){
                    if (-437.5371+1.67219*(float)red(pixels[j][i])+0.083888*(float)green(pixels[j][i])+0.83757*(float)blue(pixels[j][i])>0){
                        pixels[j][i] = rgb(255, 0, 0);

                        sum_m = sum_m + green(pixels[j][i])+red(pixels[j][i])+blue(pixels[j][i]);
                        sum_mr = sum_mr + (green(pixels[j][i])+red(pixels[j][i])+blue(pixels[j][i]))*i;
                    }
                }
                bmp.setPixels(pixels[j], 0, bmp.getWidth(), 0, k, bmp.getWidth(), 1);

                // only use the data if there were a few pixels identified, otherwise you might get a divide by 0 error
                if(sum_m>5){
                    COM[j] = sum_mr / sum_m;    // center of mass from 0 to 640
                }
                else{
                    COM[j] = 0;
                }
            }

        }
        int COM_sum = 0;
        for (int i=0; i<10; i++){
            COM_sum = COM_sum + COM[i];
        }
        int COM_avg = COM_sum/10;

        int COM_min = COM[0];
        int COM_max = COM[0];
        for(int i=0; i<10; i++){
            if(COM[i] < COM_min){
                COM_min = COM[i];
            }
            if(COM[i] > COM_max){
                COM_max = COM[i];
            }
        }
        if(COM_min < 50){
            COM_avg = COM_min;
        } else if(COM_max > 590){
            COM_avg = COM_max;
        }

        String sendString = String.valueOf(COM_avg) + '\n';
        try {
            sPort.write(sendString.getBytes(), 10); // 10 is the timeout
        } catch (IOException e) { }

        // draw a circle at some position
        for (int i=0; i<10; i++){
            int k = startY + i*5;
            if(COM[i] != 0){
                canvas.drawCircle(COM[i], k , 5, paint1); // x position, y position, diameter, color
            }
        }

        // write the pos as text
        canvas.drawText("COM_avg = " + COM_avg, 10, 200, paint1);
        c.drawBitmap(bmp, 0, 0, null);
        mSurfaceHolder.unlockCanvasAndPost(c);

        // calculate the FPS to see how fast the code is running
        long nowtime = System.currentTimeMillis();
        long diff = nowtime - prevtime;
        mTextView.setText("FPS " + 1000 / diff);
        prevtime = nowtime;
    }

}