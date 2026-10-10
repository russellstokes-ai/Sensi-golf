package com.russellstokes.sensigolf;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Insets;
import android.graphics.Paint;
import android.graphics.RectF;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.SystemClock;
import android.provider.DocumentsContract;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowInsets;
import android.widget.Toast;
import org.json.JSONObject;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;

/**
 * Real native-engine test shell, no included copyrighted original assets.
 * Android UI reads the recovered C++ simulation, never calculates ball physics.
 */
public final class MainActivity extends Activity {
    private static final int REQUEST_ORIGINAL_FOLDER = 117;
    private static final String PREFS = "sensigolf.preview.preferences";
    private GameView view;
    private SharedPreferences prefs;
    private volatile boolean importing = false;
    private boolean paused = false;
    private boolean enhanced = false;
    private String notification = "Import your legitimately owned, extracted original course folder.";

    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        getWindow().addFlags(android.view.WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        prefs = getSharedPreferences(PREFS, MODE_PRIVATE);
        enhanced = prefs.getBoolean("enhancedPixels", false);
        view = new GameView();
        setContentView(view);
        File files = new File(getFilesDir(), "original-epf");
        if (new File(files, "MAPM42.MAP").exists()
                && new File(files, "MAPI01.RAW").exists()) {
            try {
                NativeBridge.nativeLoad(files.getAbsolutePath());
                view.clearMap();
                notification = NativeBridge.nativeStatus();
            } catch (RuntimeException e) {
                notification = e.getMessage();
            }
        }
    }

    @Override public void onConfigurationChanged(Configuration configuration) {
        super.onConfigurationChanged(configuration);
        // One Activity / one C++ session survives Fold open/close and rotation.
        view.requestLayout();
        view.invalidate();
    }

    private void safeNative(Runnable task) {
        try { task.run(); notification = NativeBridge.nativeStatus(); }
        catch (RuntimeException e) {
            notification = e.getMessage() == null ? "Original engine rejected action" : e.getMessage();
            Toast.makeText(this,notification,Toast.LENGTH_SHORT).show();
        }
        view.invalidate();
    }

    private void chooseOriginalFolder() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION |
                Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION);
        startActivityForResult(intent, REQUEST_ORIGINAL_FOLDER);
    }

    @Override protected void onActivityResult(int requestCode,int resultCode,Intent data) {
        super.onActivityResult(requestCode,resultCode,data);
        if (requestCode != REQUEST_ORIGINAL_FOLDER || resultCode != RESULT_OK
                || data == null || data.getData() == null || importing) return;
        Uri selected = data.getData();
        try {
            getContentResolver().takePersistableUriPermission(selected,
                    Intent.FLAG_GRANT_READ_URI_PERMISSION);
        } catch (SecurityException ignored) {
            // One-time document tree grant still allows a local copy.
        }
        importing = true;
        notification = "Importing original course resources...";
        view.invalidate();
        new Thread(() -> {
            try {
                File dir = new File(getFilesDir(),"original-epf");
                if (!dir.exists() && !dir.mkdirs()) throw new IllegalStateException("Cannot create local course folder");
                int[] progress = {0};
                String rootId=DocumentsContract.getTreeDocumentId(selected);
                importDirectory(selected,rootId,dir,0,progress);
                if (!new File(dir,"MAPI01.RAW").exists() ||
                        !new File(dir,"MAPI02.RAW").exists() ||
                        !new File(dir,"MAPM42.MAP").exists() ||
                        !new File(dir,"MAPM42.SPT").exists() ||
                        !new File(dir,"MAPS42.MAP").exists())
                    throw new IllegalStateException(
                        "Selected folder is missing required v1.014 MAPI or hole 42 files. Select the extracted EPF resource folder.");
                runOnUiThread(() -> {
                    importing=false;
                    try {
                        NativeBridge.nativeLoad(dir.getAbsolutePath());
                        view.clearMap();
                        notification="Original C++ engine ready — "+progress[0]+" source files imported";
                    } catch(RuntimeException e) { notification=e.getMessage(); }
                    view.invalidate();
                });
            } catch (Exception e) {
                runOnUiThread(() -> {
                    importing=false;
                    notification="Import failed: "+e.getMessage();
                    Toast.makeText(this,notification,Toast.LENGTH_LONG).show();
                    view.invalidate();
                });
            }
        },"OriginalCourseImport").start();
    }

    private void importDirectory(Uri tree,String parentId,File destination,int depth,int[] count)
            throws Exception {
        if (depth>5) return;
        Uri children=DocumentsContract.buildChildDocumentsUriUsingTree(tree,parentId);
        String[] projection={DocumentsContract.Document.COLUMN_DOCUMENT_ID,
                DocumentsContract.Document.COLUMN_DISPLAY_NAME,
                DocumentsContract.Document.COLUMN_MIME_TYPE,
                DocumentsContract.Document.COLUMN_SIZE};
        try (Cursor cursor=getContentResolver().query(children,projection,null,null,null)) {
            if(cursor==null) throw new IllegalStateException("Cannot enumerate selected document tree");
            while(cursor.moveToNext()) {
                String id=cursor.getString(0);
                String name=cursor.getString(1);
                String mime=cursor.getString(2);
                if (DocumentsContract.Document.MIME_TYPE_DIR.equals(mime)) {
                    importDirectory(tree,id,destination,depth+1,count);
                    continue;
                }
                if(name==null) continue;
                String upper=name.toUpperCase(java.util.Locale.ROOT);
                if(!upper.matches("[A-Z0-9_-]+\\.(MAP|SPT|RAW)")) continue;
                if(count[0]>=800) throw new IllegalStateException("Original folder has too many matching resources");
                Uri fileUri=DocumentsContract.buildDocumentUriUsingTree(tree,id);
                File target=new File(destination,upper);
                File tmp=new File(destination,upper+".partial");
                long total=0;
                try(InputStream in=getContentResolver().openInputStream(fileUri);
                    FileOutputStream out=new FileOutputStream(tmp)) {
                    if(in==null) throw new IllegalStateException("Cannot open "+upper);
                    byte[] buffer=new byte[32*1024];
                    int got;
                    while((got=in.read(buffer))!=-1) {
                        total+=got;
                        if(total>16L*1024*1024) throw new IllegalStateException("Oversized "+upper);
                        out.write(buffer,0,got);
                    }
                }
                if(!tmp.renameTo(target)) {
                    // Fall back to replacement on providers/filesystems that retain old targets.
                    if(!target.delete() || !tmp.renameTo(target))
                        throw new IllegalStateException("Could not store "+upper);
                }
                count[0]++;
            }
        }
    }

    private void showMenu() {
        String[] items={
                enhanced ? "Graphics: Enhanced 2× ✓" : "Graphics: Classic Pixels ✓",
                "Switch to Classic Pixels",
                "Switch to Enhanced 2×",
                paused ? "Resume game" : "Pause game",
                "Import original course folder",
                "Restart round",
                "Help / known limitations"
        };
        new AlertDialog.Builder(this).setTitle("Sensi Golf • Engine Preview")
                .setItems(items,(dialog,which)->{
                    if(which==1 || which==2) {
                        enhanced=which==2;
                        prefs.edit().putBoolean("enhancedPixels",enhanced).apply();
                        view.invalidate();
                    } else if(which==3) {
                        paused=!paused;
                        view.resetClock();
                    } else if(which==4) chooseOriginalFolder();
                    else if(which==5)
                        new AlertDialog.Builder(this).setTitle("Restart round?")
                            .setMessage("All strokes in this testing session will be reset.")
                            .setPositiveButton("Restart",(d,w)->safeNative(NativeBridge::nativeReset))
                            .setNegativeButton("Cancel",null).show();
                    else if(which==6) new AlertDialog.Builder(this)
                        .setTitle("Android original-engine test")
                        .setMessage("Import your own extracted Windows v1.014 EPF resources. Aim on the course, choose one of 13 clubs, then tap SWING three times (start / power / accuracy). The meter's safe accuracy preview is not yet the original full timing algorithm. The course is coloured from collision data, not original copyrighted artwork. Special-green event 11 remains unsupported. Menu has Classic Pixels and Enhanced 2× render modes. Fold open/close and tablets use adaptive controls.")
                        .setPositiveButton("OK",null).show();
                }).show();
    }

    @Override public boolean onKeyDown(int keyCode,KeyEvent event) {
        if(event.getRepeatCount()>0) return true;
        switch(keyCode) {
            case KeyEvent.KEYCODE_DPAD_LEFT: safeNative(()->NativeBridge.nativeAim(-16)); return true;
            case KeyEvent.KEYCODE_DPAD_RIGHT: safeNative(()->NativeBridge.nativeAim(16)); return true;
            case KeyEvent.KEYCODE_BUTTON_L1: safeNative(()->NativeBridge.nativeClub(-1)); return true;
            case KeyEvent.KEYCODE_BUTTON_R1: safeNative(()->NativeBridge.nativeClub(1)); return true;
            case KeyEvent.KEYCODE_BUTTON_B: safeNative(NativeBridge::nativeCancel); return true;
            case KeyEvent.KEYCODE_BUTTON_A:
            case KeyEvent.KEYCODE_DPAD_CENTER: view.swing(); return true;
            case KeyEvent.KEYCODE_BUTTON_START: showMenu(); return true;
            default: return super.onKeyDown(keyCode,event);
        }
    }

    private final class GameView extends View {
        private final Paint p=new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint type=new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Map<String,RectF> buttons=new HashMap<>();
        private final RectF courseRect=new RectF();
        private Bitmap courseBitmap,enhancedBitmap;
        private int lastCourse=-999;
        private float zoom=1.0f;
        private long lastFrame=0L, stageTime=0L;
        private double tickDebt=0d;
        private float touchX,touchY;
        private float displayedBallX = Float.NaN, displayedBallY = Float.NaN;
        private boolean pinch=false;
        private float twoFingerDistance=0f;
        private int safeL=0,safeT=0,safeR=0,safeB=0;
        private JSONObject lastSnapshot=new JSONObject();

        GameView() {
            super(MainActivity.this);
            setFocusable(true);
            setFocusableInTouchMode(true);
            type.setTypeface(android.graphics.Typeface.create("monospace",
                    android.graphics.Typeface.BOLD));
            if(Build.VERSION.SDK_INT>=30) setOnApplyWindowInsetsListener((v,insets)->{
                Insets s=insets.getInsets(WindowInsets.Type.systemBars() |
                        WindowInsets.Type.displayCutout());
                safeL=s.left;safeT=s.top;safeR=s.right;safeB=s.bottom;
                invalidate();return insets;
            });
        }
        void clearMap() {
            if(courseBitmap!=null) courseBitmap.recycle();
            if(enhancedBitmap!=null) enhancedBitmap.recycle();
            courseBitmap=null;enhancedBitmap=null;lastCourse=-999;
            resetClock();
        }
        void resetClock(){ lastFrame=0;tickDebt=0; }
        private float dp(float n) { return n*getResources().getDisplayMetrics().density; }
        private int readMeter(int stage) {
            long elapsed=Math.max(0,SystemClock.elapsedRealtimeNanos()-stageTime);
            int t=(int)Math.min(1000000L,elapsed/14282227L);
            if(stage==1) {
                int swing=t%210;
                return swing<=105?swing:210-swing;
            }
            if(stage==2) {
                // Safe preview around recovered accuracy centre. Wider original
                // miss profile is not yet parity verified; do not fake it.
                int cycle=t%12;
                return 63+(cycle<=6?cycle-3:9-cycle)/2;
            }
            return 0;
        }
        void swing() {
            int stage=lastSnapshot.optInt("meterStage",0);
            if(!lastSnapshot.optBoolean("ready",false) || paused) return;
            int reading=readMeter(stage);
            safeNative(()->NativeBridge.nativeClick(reading));
            stageTime=SystemClock.elapsedRealtimeNanos();
            invalidate();
        }
        private void fill(Canvas c,int color) { c.drawColor(color); }
        private void label(Canvas c,String value,float x,float y,float size,int color) {
            type.setTextAlign(Paint.Align.LEFT);
            type.setTextSize(dp(size));type.setColor(color);
            c.drawText(value,x,y,type);
        }
        private void centre(Canvas c,String value,float x,float y,float size,int color) {
            type.setTextAlign(Paint.Align.CENTER);
            type.setTextSize(dp(size));type.setColor(color);
            c.drawText(value,x,y,type);
        }
        private void button(Canvas c,String key,String text,float left,float top,
                            float width,float height,int colour) {
            RectF r=new RectF(left,top,left+width,top+height);
            buttons.put(key,r);
            p.setColor(colour);
            c.drawRoundRect(r,dp(11),dp(11),p);
            centre(c,text,r.centerX(),r.centerY()+dp(5),13,Color.WHITE);
        }
        private void syncBitmap(JSONObject snapshot) {
            int id=snapshot.optInt("courseId",-1);
            if(id==lastCourse || !snapshot.optBoolean("loaded",false)) return;
            clearMap();
            lastCourse=id;
            int w=snapshot.optInt("mapW",0),h=snapshot.optInt("mapH",0);
            int[] pixels=NativeBridge.nativePixels();
            if(w>0 && h>0 && pixels.length==w*h) {
                courseBitmap=Bitmap.createBitmap(pixels,w,h,Bitmap.Config.ARGB_8888);
            }
        }
        @Override protected void onDraw(Canvas canvas) {
            super.onDraw(canvas);
            fill(canvas,Color.rgb(12,30,21));
            buttons.clear();
            int w=getWidth(),h=getHeight();
            if(w<dp(180) || h<dp(210)) return;
            long now=SystemClock.elapsedRealtimeNanos();
            if(lastFrame==0) lastFrame=now;
            if(!paused && !importing) {
                tickDebt+=Math.min(0.08d,(now-lastFrame)/1e9d)*(65536d/936d);
                int steps=(int)Math.min(8d,Math.floor(tickDebt));
                if(steps>0) {
                    try { NativeBridge.nativeStep(steps); }
                    catch(RuntimeException e) { notification=e.getMessage(); paused=true; }
                    tickDebt-=steps;
                }
            }
            lastFrame=now;
            try { lastSnapshot=new JSONObject(NativeBridge.nativeSnapshot()); }
            catch(Exception e) { notification="Engine unavailable: "+e.getMessage(); }
            JSONObject s=lastSnapshot;
            try { syncBitmap(s); } catch(RuntimeException e){notification=e.getMessage();}

            float l=safeL+dp(8),t=safeT+dp(8),r=w-safeR-dp(8),b=h-safeB-dp(8);
            if(r-l<dp(150) || b-t<dp(190)) { postInvalidateDelayed(80);return; }
            float ww=(r-l)/getResources().getDisplayMetrics().density;
            float hh=(b-t)/getResources().getDisplayMetrics().density;
            GolfLayout.Mode mode=GolfLayout.mode(ww,hh);
            final float header=dp(54);
            p.setColor(Color.rgb(22,53,34));
            canvas.drawRoundRect(new RectF(l,t,r,t+header),dp(9),dp(9),p);
            label(canvas,"SENSI GOLF",l+dp(13),t+dp(22),16,0xfff8e8b4);
            if(s.optBoolean("loaded",false)) {
                label(canvas,"HOLE "+(s.optInt("roundIndex",0)+1)+"/18    "
                        +"STROKES "+s.optInt("strokes",0)+"    "
                        +"DIST "+s.optInt("distance",0),l+dp(13),t+dp(42),
                        mode==GolfLayout.Mode.PHONE_LANDSCAPE?8:10,0xffcce2ba);
            } else {
                label(canvas,"ORIGINAL ENGINE • ANDROID TEST",l+dp(13),t+dp(42),9,0xffbbd2b3);
            }
            button(canvas,"MENU","MENU",r-dp(74),t+dp(3),dp(71),dp(48),0xff37583b);

            float top=t+header+dp(6);
            float side=dp(GolfLayout.sidebarDp(ww,hh));
            float dock=dp(GolfLayout.bottomDockDp(ww,hh));
            if(mode==GolfLayout.Mode.PHONE_PORTRAIT)
                courseRect.set(l,top,r,b-dock-dp(7));
            else
                courseRect.set(l,top,r-side-dp(12),b);
            p.setColor(0xff263e28);
            canvas.drawRoundRect(courseRect,dp(9),dp(9),p);
            canvas.save();
            canvas.clipRect(courseRect);
            if(courseBitmap!=null && s.optBoolean("loaded",false)) {
                drawCourse(canvas,s);
            } else {
                centre(canvas,"LOAD ORIGINAL COURSES",courseRect.centerX(),
                        courseRect.centerY()-dp(25),15,0xffe9dca4);
                centre(canvas,"Requires your own v1.014 EPF extraction",
                        courseRect.centerX(),courseRect.centerY(),10,0xffd8e5d0);
                button(canvas,"IMPORT","IMPORT FOLDER",
                        courseRect.centerX()-dp(100),courseRect.centerY()+dp(22),
                        dp(200),dp(54),0xff467e46);
            }
            canvas.restore();
            if(s.optBoolean("loaded",false)) {
                if(mode==GolfLayout.Mode.PHONE_PORTRAIT) {
                    controls(canvas,l,b-dock+dp(5),r-l,dock-dp(6),s,mode);
                } else {
                    controls(canvas,courseRect.right+dp(10),top,
                            r-courseRect.right-dp(10),b-top,s,mode);
                }
            }
            if(paused) centre(canvas,"PAUSED",courseRect.centerX(),
                    courseRect.centerY(),22,0xfff2e3b1);
            if(notification!=null && (!s.optBoolean("loaded",false) ||
                    notification.contains("unsupported") || notification.contains("not recovered"))) {
                p.setColor(0xbd091d13);
                RectF msg=new RectF(courseRect.left+dp(7),courseRect.bottom-dp(40),
                        courseRect.right-dp(7),courseRect.bottom-dp(5));
                canvas.drawRoundRect(msg,dp(8),dp(8),p);
                String shortStatus=notification.length()>60?notification.substring(0,57)+"...":notification;
                centre(canvas,shortStatus,msg.centerX(),msg.centerY()+dp(3),9,0xfff3e5bc);
            }
            postInvalidateDelayed(16);
        }
        private void drawCourse(Canvas canvas,JSONObject s) {
            if(courseBitmap==null)return;
            Bitmap texture=courseBitmap;
            if(enhanced) {
                if(enhancedBitmap==null)
                    enhancedBitmap=Bitmap.createScaledBitmap(courseBitmap,
                        courseBitmap.getWidth()*2,courseBitmap.getHeight()*2,true);
                texture=enhancedBitmap;
            }
            final float sourceW=courseBitmap.getWidth(),sourceH=courseBitmap.getHeight();
            final float base=Math.min(courseRect.width()/sourceW,courseRect.height()/sourceH);
            float scale=Math.max(dp(1f),base*1.65f)*zoom;
            float bx=(float)s.optDouble("x",0)/4f;
            float by=(float)s.optDouble("y",0)/4f;
            float mapLeft=courseRect.centerX()-bx*scale;
            float mapTop=courseRect.centerY()-by*scale;
            if(sourceW*scale<courseRect.width())mapLeft=courseRect.centerX()-sourceW*scale/2f;
            else mapLeft=Math.min(courseRect.left,Math.max(courseRect.right-sourceW*scale,mapLeft));
            if(sourceH*scale<courseRect.height())mapTop=courseRect.centerY()-sourceH*scale/2f;
            else mapTop=Math.min(courseRect.top,Math.max(courseRect.bottom-sourceH*scale,mapTop));
            RectF mapRect=new RectF(mapLeft,mapTop,mapLeft+sourceW*scale,mapTop+sourceH*scale);
            p.setColor(Color.WHITE);
            p.setFilterBitmap(enhanced);
            canvas.drawBitmap(texture,null,mapRect,p);
            float cupX=mapLeft+((float)s.optDouble("cupX",0)/4f)*scale;
            float cupY=mapTop+((float)s.optDouble("cupY",0)/4f)*scale;
            p.setColor(Color.BLACK);
            canvas.drawCircle(cupX,cupY,dp(6),p);
            p.setColor(0xfff4d979);
            canvas.drawCircle(cupX,cupY,dp(4),p);
            p.setStrokeWidth(dp(2));
            canvas.drawLine(cupX,cupY,cupX,cupY-dp(24),p);
            p.setColor(0xffef5752);
            canvas.drawRect(cupX,cupY-dp(24),cupX+dp(13),cupY-dp(16),p);
            float x=mapLeft+bx*scale,y=mapTop+by*scale;
            displayedBallX=x;
            displayedBallY=y;
            int aim=s.optInt("aim",0);
            if(s.optBoolean("ready",false)) {
                double rad=aim*(Math.PI*2/4096d);
                p.setColor(0xffe9eed7);
                p.setStrokeWidth(dp(1.5f));
                canvas.drawLine(x,y,
                    x+(float)Math.sin(rad)*dp(59),y+(float)Math.cos(rad)*dp(59),p);
            }
            p.setColor(0x99000000);
            canvas.drawOval(new RectF(x-dp(6),y-dp(2),x+dp(6),y+dp(3)),p);
            float elevation=Math.min(dp(28),(float)s.optDouble("height",0)*dp(3));
            p.setColor(Color.WHITE);
            canvas.drawCircle(x,y-elevation,dp(5),p);
            p.setStyle(Paint.Style.STROKE);
            p.setColor(0xff213422);p.setStrokeWidth(dp(1));
            canvas.drawCircle(x,y-elevation,dp(5),p);
            p.setStyle(Paint.Style.FILL);
            // Camera-only zoom: these buttons never touch recovered ball physics.
            button(canvas,"ZOOM-","−",courseRect.left+dp(5),courseRect.top+dp(5),
                    dp(48),dp(48),0xba23412d);
            button(canvas,"ZOOM+","+ ",courseRect.left+dp(58),courseRect.top+dp(5),
                    dp(48),dp(48),0xba23412d);
        }
        private void controls(Canvas c,float left,float top,float width,float height,
                              JSONObject s,GolfLayout.Mode mode) {
            float gap=dp(6);
            float minimum=dp(48);
            float rowH=Math.max(minimum,Math.min(dp(56),height/4.2f));
            float buttonW=Math.max(minimum,Math.min(dp(58),width*0.25f));
            float clubY=top;
            button(c,"CLUB-","−",left,clubY,buttonW,rowH,0xff38553b);
            button(c,"CLUB+","+ ",left+width-buttonW,clubY,buttonW,rowH,0xff38553b);
            centre(c,"CLUB "+s.optInt("club",0),left+width/2f,clubY+rowH/2f+dp(5),
                    mode==GolfLayout.Mode.PHONE_PORTRAIT?12:11,0xffd9e5cb);
            float aimY=clubY+rowH+gap;
            button(c,"AIM-","◀",left,aimY,buttonW,rowH,0xff38553b);
            button(c,"AIM+","▶",left+width-buttonW,aimY,buttonW,rowH,0xff38553b);
            centre(c,"AIM "+s.optInt("aim",0),left+width/2f,aimY+rowH/2f+dp(5),
                    11,0xffd9e5cb);
            int stage=s.optInt("meterStage",0);
            String stageLabel=stage==0?"SWING • 1/3":stage==1?"LOCK POWER • 2/3":
                    stage==2?"LOCK ACCURACY • 3/3":"SHOT QUEUED";
            float swingTop=aimY+rowH+gap;
            float swingH=Math.max(dp(58),Math.min(dp(79),height-(swingTop-top)-dp(57)));
            float cancelW=mode==GolfLayout.Mode.PHONE_PORTRAIT?dp(83):0;
            button(c,"SWING",stageLabel,left,swingTop,
                    width-cancelW-(cancelW>0?gap:0),swingH,0xff6a8550);
            if(cancelW>0)
                button(c,"CANCEL","CANCEL",left+width-cancelW,
                    swingTop,cancelW,swingH,0xff76543c);
            float rest=swingTop+swingH+gap;
            if(mode!=GolfLayout.Mode.PHONE_PORTRAIT && rest+dp(48)<top+height)
                button(c,"CANCEL","CANCEL",left,rest,width,dp(48),0xff71563a);
            float reading=readMeter(stage);
            if(stage==1 || stage==2) {
                float barLeft=left+dp(2),barRight=left+width-dp(2);
                float barTop=Math.min(top+height-dp(12),rest+dp(6));
                p.setColor(0xff233b30);
                c.drawRoundRect(new RectF(barLeft,barTop,barRight,barTop+dp(8)),dp(3),dp(3),p);
                p.setColor(0xffedd39b);
                float percentage=reading/105f;
                c.drawRoundRect(new RectF(barLeft,barTop,barLeft+
                    Math.max(dp(2),(barRight-barLeft)*percentage),barTop+dp(8)),dp(3),dp(3),p);
            }
        }
        private float distance(MotionEvent e) {
            if(e.getPointerCount()<2)return 0f;
            return (float)Math.hypot(e.getX(0)-e.getX(1),e.getY(0)-e.getY(1));
        }
        @Override public boolean onTouchEvent(MotionEvent e) {
            switch(e.getActionMasked()) {
                case MotionEvent.ACTION_DOWN:
                    touchX=e.getX();touchY=e.getY();pinch=false;
                    return true;
                case MotionEvent.ACTION_POINTER_DOWN:
                    if(e.getPointerCount()>=2) {
                        pinch=true;twoFingerDistance=distance(e);
                    }
                    return true;
                case MotionEvent.ACTION_MOVE:
                    if(e.getPointerCount()>=2 && pinch) {
                        float d=distance(e);
                        if(twoFingerDistance>0f && d>0f) zoom=Math.max(0.7f,
                                Math.min(4f,zoom*d/twoFingerDistance));
                        twoFingerDistance=d;
                        invalidate();
                    }
                    return true;
                case MotionEvent.ACTION_UP:
                    if(pinch) { pinch=false;return true; }
                    String hit=null;
                    for(Map.Entry<String,RectF> entry:buttons.entrySet()) {
                        if(entry.getValue().contains(e.getX(),e.getY()) &&
                           entry.getValue().contains(touchX,touchY)) {
                            hit=entry.getKey();break;
                        }
                    }
                    if(hit!=null) {
                        action(hit);
                    } else if(courseRect.contains(e.getX(),e.getY()) &&
                            lastSnapshot.optBoolean("ready",false)) {
                        // Touch direction relative to the ball marker at viewport
                        // centre; small aim corrections via arrow buttons.
                        float originX=Float.isNaN(displayedBallX)
                                ? courseRect.centerX():displayedBallX;
                        float originY=Float.isNaN(displayedBallY)
                                ? courseRect.centerY():displayedBallY;
                        float dx=e.getX()-originX;
                        float dy=e.getY()-originY;
                        if(Math.hypot(dx,dy)<dp(12)) return true;
                        int target=(int)Math.round((Math.atan2(dx,dy)/
                                (Math.PI*2d))*4096d)&4095;
                        int current=lastSnapshot.optInt("aim",0);
                        int delta=((target-current+6144)&4095)-2048;
                        safeNative(()->NativeBridge.nativeAim(delta));
                    }
                    invalidate();return true;
                case MotionEvent.ACTION_CANCEL:
                    pinch=false;return true;
                default:return true;
            }
        }
        private void action(String key) {
            switch(key) {
                case "MENU":showMenu();break;
                case "IMPORT":chooseOriginalFolder();break;
                case "SWING":swing();break;
                case "CANCEL":safeNative(NativeBridge::nativeCancel);break;
                case "CLUB-":safeNative(()->NativeBridge.nativeClub(-1));break;
                case "CLUB+":safeNative(()->NativeBridge.nativeClub(1));break;
                case "AIM-":safeNative(()->NativeBridge.nativeAim(-12));break;
                case "AIM+":safeNative(()->NativeBridge.nativeAim(12));break;
                case "ZOOM-":zoom=Math.max(0.7f,zoom/1.2f);break;
                case "ZOOM+":zoom=Math.min(4f,zoom*1.2f);break;
                default:break;
            }
        }
    }
}
