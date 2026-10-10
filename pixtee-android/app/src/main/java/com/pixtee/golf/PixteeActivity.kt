package com.pixtee.golf

import android.app.Activity
import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.RectF
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.Path
import android.graphics.Typeface
import android.os.Bundle
import android.view.MotionEvent
import android.view.View
import android.view.WindowManager
import kotlin.math.abs
import kotlin.math.cos
import kotlin.math.min
import kotlin.math.sin
import kotlin.math.sqrt
import kotlin.random.Random
import org.json.JSONObject

/** Pure original Pixtee frontend. No DOS emulator, purchased files or internet required. */
class PixteeActivity : Activity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        @Suppress("DEPRECATION")
        window.decorView.systemUiVisibility =
            View.SYSTEM_UI_FLAG_FULLSCREEN or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION or
            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN or
            View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION or View.SYSTEM_UI_FLAG_LAYOUT_STABLE
        setContentView(PixteeCanvas(this))
    }
}

private enum class Screen { MAIN, COURSES, PLAYER, TEE, PLAYING, SCORE, RESULTS, PAUSE, CAREER, STATS, TROPHIES, OPTIONS, WARDROBE }

/** Every UI element is laid out against a TALL 360x760 logical phone, then scaled edge-to-edge. */
private class PixteeCanvas(context: Context) : View(context) {
    private val p = Paint().apply { isAntiAlias = false; isFilterBitmap = false
        typeface = Typeface.create(Typeface.MONOSPACE, Typeface.BOLD) }
    private val g = PixteeCore()
    private val menuInput = MenuInput()
    private var round: PixteeRound? = null
    private var selectedCourse = 0
    private var selectedLength = 18
    private var selectedMode = RoundMode.QUICK
    private var careerTier = 0
    private var careerEventIndex = -1
    private var lastReward = ""
    private fun progress(): ReaderStats = ReaderStats(
        holes=stats.getInt("holes_played",0),
        rounds=stats.getInt("rounds",0),
        birdies=stats.getInt("birdies",0),
        eagles=stats.getInt("eagles",0),
        putts=stats.getInt("putts",0),
        shots=stats.getInt("shots",0),
        penalties=stats.getInt("penalties",0),
        cleanNine=stats.getInt("clean_nine",0),
        careerMask=stats.getLong("career_mask",0L)
    )
    private val tourNames = listOf("AMATEUR TOUR", "REGIONAL TOUR", "NATIONAL TOUR",
        "PRO TOUR", "WORLD TOUR")
    private var currentStyle = StyleSlot.HAT
    private val equipmentPrefs = context.getSharedPreferences("pixtee_equipment_v1", Context.MODE_PRIVATE)
    // Lazy loading avoids calling progress() before stats SharedPreferences is initialized.
    // Android Canvas constructors initialize fields in declaration order.
    private val equipment: MutableMap<StyleSlot,String> by lazy {
        PixteeWardrobe.decode(equipmentPrefs.getString("gear",null),progress()).toMutableMap()
    }
    private fun gear(slot:StyleSlot):StyleItem =
        PixteeWardrobe.equippedItem(slot,equipment,progress())
    private fun gearColour(slot:StyleSlot):Int =
        gear(slot).colour or 0xFF000000.toInt()
    private fun saveGear() {
        equipmentPrefs.edit().putString("gear",
            PixteeWardrobe.encode(equipment,progress())).apply()
    }
    private fun screenContentHeight() = when(screen) {
        Screen.COURSES -> 1620f
        Screen.TROPHIES -> 2400f
        Screen.MAIN -> 840f
        else -> MenuInput.CONTENT_HEIGHT
    }
    private fun startRound(mode: RoundMode, length: Int = selectedLength) {
        selectedMode = mode
        lastReward = ""
        if(mode != RoundMode.CAREER) careerEventIndex=-1
        round = PixteeRound(selectedCourse,length,mode)
        g.startHole(round!!.layout())
        g.practice = mode == RoundMode.PRACTICE
        saveSession(null)
        screen = Screen.TEE
    }
    private fun nextAfterScore() {
        val played = round ?: run { screen = Screen.MAIN; return }
        if (played.isComplete) screen = Screen.RESULTS
        else {
            g.startHole(played.layout())
            saveSession(null)
            screen = Screen.TEE
        }
    }
    private val stats = context.getSharedPreferences("pixtee_stats_v1", Context.MODE_PRIVATE)
    private val gameSave = context.getSharedPreferences("pixtee_round_save_v2",Context.MODE_PRIVATE)
    private var lastSaved = ""
    private fun savedGame(): SavedPixteeGame? =
        PixteeSaveCodec.decode(gameSave.getString("active",null))
    private fun saveSession(ball: StableBall? = null) {
        val r=round ?: return
        val value=PixteeSaveCodec.encode(r,ball,careerEventIndex)
        if(value!=lastSaved) {
            gameSave.edit().putString("active",value).apply()
            lastSaved=value
        }
    }
    private fun continueRound() {
        val restored=savedGame() ?: return
        round=restored.round
        selectedCourse=restored.round.courseIndex
        selectedLength=restored.round.length
        selectedMode=restored.round.mode
        careerEventIndex=restored.careerEventIndex
        lastSaved=""
        if(restored.round.isComplete) screen=Screen.RESULTS
        else {
            g.startHole(restored.round.layout())
            val ball=restored.ball
            if(ball!=null) {
                g.restoreBall(ball)
                screen=Screen.PLAYING
                lastNs=0L;accumulator=0f
            } else screen=Screen.TEE
        }
    }
    private var screen = Screen.MAIN
    private var lastNs = 0L
    private var accumulator = 0f
    private var downX = 0f
    private var downY = 0f
    private var selection = 0
    private var sound = true
    private val courseOptions = context.getSharedPreferences("pixtee_options_v1", Context.MODE_PRIVATE)
    private var sponsorBoards = courseOptions.getBoolean("course_boards", true)
    private var courseMotion = courseOptions.getBoolean("course_motion", true)
    // Legacy hand-sold sponsor manifests are disabled: publisher requires automatic
    // network fill only. Until an approved SDK is installed, render house signs.
    private val sponsorCampaigns = emptyList<SponsorCampaign>()
    private val networkAds: InCourseAdNetwork = OfflineInCourseAds()
    private val logoCache = mutableMapOf<String, Bitmap?>()
    private val sponsorSlots = SponsorInventory.slots(
        SponsorInventory.DEFAULT_COURSE_ID, 1,
        PixteeCore.TEE_X, PixteeCore.TEE_Y, PixteeCore.PIN_X, PixteeCore.PIN_Y
    )
    private val navy = Color.rgb(3, 8, 91)
    private val navyDark = Color.rgb(0, 3, 38)
    private val gold = Color.rgb(255, 184, 34)
    private val cream = Color.rgb(255, 243, 193)
    private val wood = Color.rgb(94, 27, 7)
    private val woodLight = Color.rgb(156, 65, 16)
    private val grass = Color.rgb(49, 176, 20)
    private val holes get() = PixteeCourseCatalog.courses
    private val trees = buildList {
        val r = Random(1021)
        repeat(120) {
            val x = 8f + r.nextFloat() * 284f
            val y = 9f + r.nextFloat() * 491f
            if (g.groundAt(x, y) == Ground.ROUGH) add(x to y)
        }
    }

    // Decorative coordinates are deterministic and never part of groundAt().
    private val grassSpots = buildList {
        val rng = Random(1249)
        repeat(135) {
            val x = 11f + rng.nextFloat() * 276f
            val y = 15f + rng.nextFloat() * 479f
            if (g.groundAt(x, y) == Ground.ROUGH) add(x to y)
        }
    }
    private val flowers = buildList {
        val rng = Random(9281)
        repeat(65) {
            val x = 12f + rng.nextFloat() * 274f
            val y = 15f + rng.nextFloat() * 476f
            if (g.groundAt(x, y) == Ground.ROUGH) add(x to y)
        }
    }
    private val spectators = listOf(
        29f to 420f, 37f to 447f, 52f to 483f, 269f to 422f, 281f to 458f,
        23f to 141f, 32f to 109f, 50f to 82f, 264f to 72f,
        281f to 104f, 279f to 141f, 31f to 305f
    )

    private fun logicalScreenHeight(): Float =
        height.coerceAtLeast(1) * 360f / width.coerceAtLeast(1)

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        val now = System.nanoTime()
        if (lastNs != 0L && screen == Screen.PLAYING) {
            accumulator += ((now - lastNs) / 1_000_000_000f).coerceIn(0f, 0.05f)
            var steps = 0
            while (accumulator >= PixteeCore.TICK_SECONDS && steps < 4) {
                g.tick()
                accumulator -= PixteeCore.TICK_SECONDS
                steps++
            }
        }
        lastNs = now
        // Persist only settled ball coordinates. Pending swings resume from
        // the last fully completed shot, not a partially simulated frame.
        if(screen==Screen.PLAYING && g.stage==GameStage.READY)
            saveSession(g.stableBall())
        canvas.save()
        // One X/Y scale: a tall handset reveals MORE world vertically;
        // neither golfers nor collision geometry get stretched.
        val pixelScale = width / 360f
        canvas.scale(pixelScale, pixelScale)
        if (screen != Screen.PLAYING && screen != Screen.PAUSE)
            canvas.translate(0f, -menuInput.scrollY)
        when (screen) {
            Screen.MAIN -> drawMain(canvas)
            Screen.COURSES -> drawCourses(canvas)
            Screen.PLAYER -> drawPlayer(canvas)
            Screen.TEE -> drawTee(canvas)
            Screen.PLAYING -> drawPlaying(canvas)
            Screen.SCORE -> drawScore(canvas)
            Screen.RESULTS -> drawResults(canvas)
            Screen.PAUSE -> drawPause(canvas)
            Screen.CAREER -> drawCareer(canvas)
            Screen.STATS -> drawStats(canvas)
            Screen.TROPHIES -> drawTrophies(canvas)
            Screen.OPTIONS -> drawOptions(canvas)
            Screen.WARDROBE -> drawWardrobe(canvas)
        }
        canvas.restore()
        val state = "Pixtee screen: ${screen.name}" +
            (if (screen == Screen.PLAYING) "; stage: ${g.stage.name}; strokes: ${g.strokes}" else "") +
            (if (g.activeHole != null && screen in listOf(Screen.TEE, Screen.PLAYING, Screen.PAUSE, Screen.SCORE))
                "; course: ${g.activeHole?.course?.id}; hole: ${g.holeNumber}" else "")
        if (contentDescription != state) contentDescription = state
        if (screen == Screen.PLAYING) postInvalidateOnAnimation()
    }

    private fun fill(c: Canvas, color: Int) { c.drawColor(color) }
    private fun rect(c: Canvas, l: Float, t: Float, r: Float, b: Float, color: Int) {
        p.color = color; p.style = Paint.Style.FILL; c.drawRect(l, t, r, b, p)
    }
    private fun line(c: Canvas, x1: Float, y1: Float, x2: Float, y2: Float, color: Int, sw: Float = 1f) {
        p.color = color; p.strokeWidth = sw; p.style = Paint.Style.STROKE
        c.drawLine(x1, y1, x2, y2, p); p.style = Paint.Style.FILL
    }
    private fun circle(c: Canvas, x: Float, y: Float, r: Float, color: Int) {
        p.color = color; p.style = Paint.Style.FILL; c.drawCircle(x, y, r, p)
    }
    private fun ellipse(c: Canvas, x1: Float, y1: Float, x2: Float, y2: Float, color: Int) {
        p.color = color; c.drawOval(x1, y1, x2, y2, p)
    }
    private fun text(c: Canvas, str: String, x: Float, y: Float, size: Float = 15f,
                     color: Int = cream, centered: Boolean = false) {
        p.color = Color.BLACK; p.textSize = size
        p.typeface = Typeface.create(Typeface.MONOSPACE, Typeface.BOLD)
        p.style = Paint.Style.FILL
        p.textAlign = if (centered) Paint.Align.CENTER else Paint.Align.LEFT
        c.drawText(str, x + 1.25f, y + 1.4f, p)
        p.color = color; c.drawText(str, x, y, p)
    }
    private fun navyBackdrop(c: Canvas) {
        fill(c, navy)
        for (y in 0..screenContentHeight().toInt() step 14) for (x in 0..360 step 23) {
            line(c, x.toFloat(), y.toFloat(), x + 4f, y - 7f, Color.rgb(5, 12, 106))
        }
    }
    private fun woodButton(c: Canvas, label: String, x: Float, y: Float,
                           w: Float, h: Float, enabled: Boolean = true) {
        rect(c, x - 2, y - 2, x + w + 2, y + h + 2, Color.BLACK)
        rect(c, x, y, x + w, y + h, Color.rgb(199, 106, 20))
        rect(c, x + 2, y + 2, x + w - 2, y + h - 2, wood)
        for (i in 0..4) {
            line(c, x + 6f, y + 5f + i * 7, x + w - 7f,
                y + 4f + i * 7, Color.rgb(111, 38, 10))
        }
        text(c, label, x + w / 2f, y + h * 0.67f, min(19f, w / (label.length * 0.67f)),
            if (enabled) gold else Color.rgb(160, 126, 87), true)
    }
    private fun pageHeader(c: Canvas, title: String) {
        navyBackdrop(c)
        woodButton(c, title, 6f, 24f, 348f, 47f)
        text(c, "<", 20f, 55f, 23f, gold)
    }
    private fun drawMain(c: Canvas) {
        navyBackdrop(c)
        text(c, "PIXTEE", 180f, 164f, 64f, Color.rgb(255, 193, 31), true)
        text(c, "GOLF", 180f, 228f, 56f, Color.rgb(63, 212, 53), true)
        val buttons = listOf("PLAY ROUND", "CAREER",
            if(savedGame()!=null) "RESUME ROUND" else "PRACTICE HOLE",
            "STATISTICS", "TROPHIES", "OPTIONS", "CUSTOMIZE GOLFER")
        for ((i, label) in buttons.withIndex())
            woodButton(c, label, 44f, 279f + i * 66f, 272f, 49f)
        text(c, "CLASSIC 2D ARCADE GOLF", 180f, 790f, 11f, cream, true)
    }
    private fun drawCourses(c: Canvas) {
        pageHeader(c, "SELECT GOLF COURSE")
        text(c, "25 PIXTEE COURSES", 180f, 108f, 13f, gold, true)
        holes.forEachIndexed { i, course ->
            val y = 132f + i * 56f
            woodButton(c, course.title.uppercase(), 37f, y, 286f, 45f)
            text(c, "18 HOLES", 294f, y + 29f, 9f, cream, true)
        }
        woodButton(c, "BACK", 75f, 1550f, 210f, 46f)
    }
    private fun drawPlayer(c: Canvas) {
        pageHeader(c, "PLAYER SELECT")
        text(c, "NAME", 74f, 149f, 18f, gold)
        text(c, "TYPE", 229f, 149f, 18f, gold)
        val names = listOf("HUMAN 1", "-", "-", "-")
        for (i in 0..3) {
            woodButton(c, names[i], 21f, 165f + i * 68f, 172f, 48f)
            woodButton(c, if (i == 0) "TOUCH" else "OFF",
                200f, 165f + i * 68f, 140f, 48f)
        }
        text(c, "ROUND LENGTH", 180f, 435f, 15f, gold, true)
        listOf(1,3,9,18).forEachIndexed { i,n ->
            woodButton(c, "$n", 21f + i*81f, 448f, 73f, 48f, selectedLength == n)
        }
        woodButton(c, "OKAY", 76f, 527f, 208f, 51f)
        woodButton(c, "EXIT", 76f, 600f, 208f, 51f)
    }
    private fun drawTee(c: Canvas) {
        pageHeader(c, "NEXT TO THE TEE")
        val hole = g.activeHole
        woodButton(c, (hole?.course?.title ?: "PIXTEE").uppercase(), 40f, 128f, 280f, 54f)
        woodButton(c, "HOLE ${g.holeNumber} (PAR ${g.par})", 40f, 203f, 280f, 51f)
        text(c, "NAME", 81f, 321f, 23f, gold)
        text(c, "SCORE", 231f, 321f, 23f, gold)
        text(c, "HUMAN 1", 50f, 381f, 22f)
        text(c, if (round?.results?.isEmpty() != false) "PAR"
            else "${round?.relativeToPar ?: 0}", 244f, 381f, 22f, gold)
        text(c, "${round?.results?.size ?: 0} / ${round?.length ?: 1} HOLES COMPLETE",
            180f, 477f, 14f, cream, true)
        woodButton(c, "TEE OFF", 63f, 611f, 234f, 60f)
    }

    /** Each hole renders from the exact same authored geometry used for collisions. */
    private fun course(c: Canvas) {
        val layout = g.activeHole ?: return
        fill(c, Color.rgb(37, 121, 19))
        val viewport = CourseViewport(360f, logicalScreenHeight())
        c.save()
        c.scale(viewport.worldScale, viewport.worldScale)
        c.translate(0f, -viewport.topWorld)
        val dark = when(layout.course.theme) {
            1 -> Color.rgb(39, 113, 30)
            2 -> Color.rgb(77, 128, 38)
            3 -> Color.rgb(43, 130, 25)
            4 -> Color.rgb(61, 131, 22)
            else -> Color.rgb(48, 131, 25)
        }
        rect(c, 0f, viewport.topWorld, 300f, viewport.bottomWorld, dark)
        for (y in (viewport.topWorld.toInt()-10)..(viewport.bottomWorld.toInt()+10) step 10)
            for (x in 0..300 step 11)
                if ((x*17+y*13+layout.seed.toInt())%7 < 3)
                    rect(c,x.toFloat(),y.toFloat(),x+3f,y+3f,
                        Color.rgb(56,146,31))
        for (y in layout.pinY.toInt()..layout.teeY.toInt() step 3) {
            val centre = layout.fairwayCentre(y.toFloat())
            val half=layout.width*.5f
            rect(c,centre-half-7f,y.toFloat(),centre+half+7f,y+3f,
                Color.rgb(52,153,23))
            rect(c,centre-half,y.toFloat(),centre+half,y+3f,
                if ((y/21)%2==0) Color.rgb(72,185,33) else Color.rgb(62,173,25))
        }
        val frame = CourseAmbient.frame(android.os.SystemClock.uptimeMillis(),courseMotion)
        layout.waters.forEach { patch ->
            rect(c,patch.l,patch.t,patch.r,patch.b,Color.rgb(8,79,174))
            var row=0
            var yy=patch.t+5f
            while(yy<patch.b-4f) {
                val shift=CourseAmbient.waterRipple(row,frame).toFloat()
                var xx=patch.l+4f+shift
                while(xx<patch.r-7f) {
                    rect(c,xx,yy,xx+6f,yy+2f,
                        if(row%3==0) Color.rgb(86,165,241) else Color.rgb(29,118,211))
                    xx+=17f
                }
                yy+=9f;row++
            }
        }
        layout.bunkers.forEach {
            ellipse(c,it.x-it.rx,it.y-it.ry,it.x+it.rx,it.y+it.ry,
                Color.rgb(225,200,110))
        }
        ellipse(c, layout.pinX-30f,layout.pinY-24f,
            layout.pinX+30f,layout.pinY+24f,Color.rgb(82,185,41))
        ellipse(c, layout.pinX-24f,layout.pinY-20f,
            layout.pinX+24f,layout.pinY+20f,Color.rgb(111,207,47))
        for((i,spot) in grassSpots.withIndex()) {
            val (gx,gy)=spot
            if (layout.groundAt(gx,gy)!=Ground.ROUGH) continue
            val sway=CourseAmbient.grassSway(i,frame).toFloat()
            line(c,gx,gy+2f,gx+sway,gy-1f,Color.rgb(81,174,46))
        }
        for((i,spot) in flowers.withIndex()) {
            val (fx,fy)=spot
            if(layout.groundAt(fx,fy)!=Ground.ROUGH) continue
            val sway=CourseAmbient.grassSway(i+3,frame).toFloat()
            line(c,fx,fy+2f,fx+sway,fy-1f,Color.rgb(23,100,23))
            rect(c,fx+sway-1f,fy-3f,fx+sway+2f,fy-1f,
                if(i%3==0) Color.rgb(255,233,108) else Color.rgb(248,214,229))
        }
        layout.trees.forEach { tree(c,it.x,it.y) }
        layout.spectators.forEachIndexed { i,spot ->
            spectator(c,spot.x,spot.y,i,frame)
        }
        rect(c, layout.teeX-21f,layout.teeY-10f,layout.teeX+21f,layout.teeY+9f,
            Color.rgb(83,185,40))
        circle(c,layout.teeX-14f,layout.teeY+5f,2.2f,Color.WHITE)
        circle(c,layout.teeX+14f,layout.teeY+5f,2.2f,Color.WHITE)
        flag(c,layout.pinX,layout.pinY)
        if(sponsorBoards) {
            val slots=SponsorInventory.slots(layout.course.id,layout.number,
                layout.teeX,layout.teeY,layout.pinX,layout.pinY)
            val shown=SponsorInventory.show(slots,sponsorCampaigns,true,
                System.currentTimeMillis()/1000L)
            shown.forEach { drawSponsorBoard(c,it) }
        }
        c.restore()
    }
    private fun tree(c: Canvas, x: Float, y: Float) {
        rect(c, x - 2f, y + 2f, x + 2f, y + 7f, Color.rgb(91, 53, 16))
        circle(c, x + 1f, y - 1f, 8f, Color.rgb(14, 72, 13))
        circle(c, x - 3f, y - 4f, 7f, Color.rgb(22, 98, 19))
        circle(c, x + 3f, y - 6f, 5f, Color.rgb(32, 129, 24))
        rect(c, x - 4f, y - 9f, x - 1f, y - 7f, Color.rgb(56, 154, 31))
    }
    /** Tiny two-pose spectators: occasional arm wave; no physics objects. */
    private fun spectator(c: Canvas, x: Float, y: Float, index: Int, frame: Int) {
        val shirt = when (index % 4) {
            0 -> Color.rgb(248, 201, 53)
            1 -> Color.rgb(44, 68, 200)
            2 -> Color.rgb(219, 70, 69)
            else -> Color.rgb(242, 237, 210)
        }
        val waving = courseMotion && CourseAmbient.spectatorWave(index, frame)
        circle(c, x, y - 8f, 2f, Color.rgb(234, 190, 129))
        rect(c, x - 3f, y - 11f, x + 3f, y - 9f, Color.rgb(39, 39, 51))
        rect(c, x - 2f, y - 6f, x + 2f, y - 1f, shirt)
        rect(c, x - 2f, y - 1f, x - 0.5f, y + 3f, Color.rgb(45, 45, 58))
        rect(c, x + 0.5f, y - 1f, x + 2f, y + 3f, Color.rgb(45, 45, 58))
        line(c, x + 2f, y - 5f, x + 4f, if (waving) y - 10f else y - 2f,
            Color.rgb(239, 191, 127), 1.3f)
    }

    private fun flag(c: Canvas, x: Float, y: Float) {
        line(c, x, y, x, y - 15f, Color.rgb(237, 237, 224), 1.4f)
        rect(c, x, y - 15f, x + 10f, y - 9f, Color.rgb(236, 37, 31))
        circle(c, x, y, 1.5f, Color.BLACK)
    }
    private fun golfer(c: Canvas, x: Float, y: Float) {
        // Purpose-made 10x14 pixel figure at the same tiny world scale as classic overhead golf.
        rect(c,x-2f,y-14f,x+3f,y-11f,gearColour(StyleSlot.HAT))
        rect(c,x-3f,y-12f,x+4f,y-10f,gearColour(StyleSlot.SKIN))
        rect(c,x-3f,y-10f,x+3f,y-4f,gearColour(StyleSlot.TOP))
        rect(c,x-3f,y-4f,x-1f,y+1f,gearColour(StyleSlot.TROUSERS))
        rect(c,x+1f,y-4f,x+3f,y+1f,gearColour(StyleSlot.TROUSERS))
        line(c,x+3f,y-7f,x+8f,y-2f,gearColour(StyleSlot.CLUB))
        // Identical tiny bag and wristband models across all equipment colours.
        rect(c,x-7f,y-6f,x-4f,y+1f,gearColour(StyleSlot.BAG))
        rect(c,x-6f,y-8f,x-5f,y-6f,gearColour(StyleSlot.ACCESSORY))
    }


    /**
     * Sponsorship creatives are locally bundled and explicitly approved.
     * No ad SDK, tracking, advertising ID, external image URLs, WebView or
     * gameplay physics changes. Future publisher tooling creates this JSON.
     */
    private fun readSponsorManifest(context: Context): List<SponsorCampaign> {
        return try {
            val content = context.assets.open("sponsors/placements.v1.json")
                .bufferedReader().use { it.readText() }
            val rows = JSONObject(content).getJSONArray("campaigns")
            val result = mutableListOf<SponsorCampaign>()
            for (i in 0 until rows.length()) {
                val item = rows.getJSONObject(i)
                val positions = item.getJSONArray("slotIds")
                val assigned = mutableSetOf<String>()
                for (j in 0 until positions.length()) assigned.add(positions.getString(j))
                val logo = item.optString("logoFile", "").takeIf {
                    it.matches(Regex("[a-z0-9][a-z0-9_-]{0,49}[.]png"))
                }
                result.add(SponsorCampaign(
                    id=item.getString("id"),
                    advertiser=item.getString("advertiser"),
                    boardText=item.getString("boardText"),
                    slotIds=assigned,
                    startUtcSeconds=item.getLong("startUtcSeconds"),
                    endUtcSeconds=item.getLong("endUtcSeconds"),
                    approved=item.optBoolean("approved", false),
                    familySafe=item.optBoolean("familySafe", false),
                    logoFile=logo,
                    backgroundRgb=Color.parseColor(item.optString("backgroundColor", "#162F52")),
                    foregroundRgb=Color.parseColor(item.optString("foregroundColor", "#FFFFFF"))
                ))
            }
            result
        } catch (_: Exception) {
            // Broken or absent creative catalog must NEVER break game launch.
            emptyList()
        }
    }

    private fun boardLogo(name: String): Bitmap? {
        if (!name.matches(Regex("[a-z0-9][a-z0-9_-]{0,49}[.]png"))) return null
        if (!logoCache.containsKey(name)) {
            logoCache[name] = try {
                context.assets.open("sponsors/logos/" + name)
                    .use { BitmapFactory.decodeStream(it) }
            } catch (_: Exception) { null }
        }
        return logoCache[name]
    }

    /**
     * Small, stationary, physical golf-course signs with two wooden posts;
     * approximately 36 game-world units wide, rendered through the SAME
     * uniform world transform as golfer/ball. Non-clickable.
     */
    private fun drawSponsorBoard(c: Canvas, sign: SponsorBoardView) {
        val x = sign.slot.x
        val y = sign.slot.y
        val w = 35f
        val h = 13f
        val left = x - w / 2f
        rect(c, left + 3f, y + 2f, left + 5f, y + 8f, Color.rgb(89, 52, 23))
        rect(c, left + w - 5f, y + 2f, left + w - 3f, y + 8f, Color.rgb(89, 52, 23))
        rect(c, left - 1f, y - h - 1f, left + w + 1f, y + 3f, Color.BLACK)
        rect(c, left, y - h, left + w, y + 1f, Color.rgb(151, 85, 34))
        rect(c, left + 2f, y - h + 2f, left + w - 2f, y - 1f,
            Color.rgb((sign.backgroundRgb shr 16) and 255,
                (sign.backgroundRgb shr 8) and 255, sign.backgroundRgb and 255))
        val logo = sign.logoFile?.let { boardLogo(it) }
        if (logo != null) {
            p.color = Color.WHITE
            p.isFilterBitmap = false
            c.drawBitmap(logo, null, RectF(left + 2f, y - h + 2f,
                left + w - 2f, y - 1f), p)
        } else {
            val colour = Color.rgb((sign.foregroundRgb shr 16) and 255,
                (sign.foregroundRgb shr 8) and 255, sign.foregroundRgb and 255)
            text(c, sign.copy.take(11), x, y - 4.2f,
                if (sign.copy.length > 7) 4.3f else 5.5f, colour, true)
        }
        if (sign.paid) {
            // Embedded disclosure within the sign; no separate UI banner.
            rect(c, left + w - 5f, y - h, left + w + 1f, y - h + 5f,
                Color.rgb(255, 203, 52))
            text(c, "AD", left + w - 2f, y - h + 3.7f, 3.2f, Color.BLACK, true)
        }
    }

    private fun drawPlaying(c: Canvas) {
        course(c)
        val viewport = CourseViewport(360f, logicalScreenHeight())
        val bx = viewport.screenX(g.x)
        val by = viewport.screenY(g.y)
        // Underlying tiny golfer drawn a small distance beside the ball.
        golfer(c, bx - 12f, by)
        circle(c, bx, by + 3f, 3f, Color.rgb(27, 70, 20))
        val lift = min(32f, g.height * 0.55f)
        if (g.height > 1f) circle(c, bx, by - lift, 3f, Color.rgb(255, 244, 174))
        else circle(c, bx, by, 2.5f, gearColour(StyleSlot.BALL))
        drawHUD(c)
        woodButton(c, "II", 305f, 3f, 49f, 30f)
        if (g.stage == GameStage.READY) {
            // Aim marker stays in course world coordinates; classic full top-down view.
            val angle = Math.toRadians(g.aimDegrees.toDouble())
            val px = bx + sin(angle).toFloat() * 65f
            val py = by - cos(angle).toFloat() * 65f
            line(c, bx, by - 5f, px, py, Color.WHITE, 1.7f)
            circle(c, px, py, 3f, gold)
        }
        if (g.stage == GameStage.POWER || g.stage == GameStage.ACCURACY) drawMeter(c)
        if (g.stage == GameStage.HOLED) {
            rect(c, 52f, 320f, 314f, 446f, navyDark)
            text(c, if (g.toPin < 5f) "HOLED OUT!" else "MAX STROKES",
                183f, 365f, 26f, gold, true)
            text(c, "TAP FOR SCORECARD", 183f, 407f, 15f, cream, true)
        }
        // Unobtrusive bottom overlay; course still bleeds fully to every screen edge.
        val bottomControlY = logicalScreenHeight() - 57f
        woodButton(c, "< AIM", 3f, bottomControlY, 82f, 50f)
        woodButton(c, "CLUB", 92f, bottomControlY, 78f, 50f)
        woodButton(c, "AIM >", 177f, bottomControlY, 82f, 50f)
        woodButton(c, "WHACK", 266f, bottomControlY, 91f, 50f)
    }
    private fun drawHUD(c: Canvas) {
        rect(c, 2f, 38f, 110f, 279f, Color.BLACK)
        rect(c, 4f, 40f, 108f, 277f, Color.rgb(34, 8, 5))
        for (y in 40..276 step 48) line(c, 4f, y.toFloat(), 108f, y.toFloat(), woodLight, 2f)
        text(c, (g.activeHole?.course?.title ?: "PIXTEE").uppercase().take(10),
            8f, 61f, 12f, gold)
        text(c, "HOLE ${g.holeNumber} PAR ${g.par}", 8f, 81f, 11f)
        text(c, "HUMAN 1", 8f, 112f, 15f, gold)
        text(c, "SHOT " + (g.strokes + 1), 8f, 131f, 13f)
        text(c, g.club.label.uppercase().take(10), 8f, 162f, 13f)
        text(c, g.club.yards.toInt().toString() + " YDS", 8f, 181f, 12f, gold)
        text(c, "TO PIN", 8f, 209f, 13f)
        text(c, g.toPin.toInt().toString() + " YDS", 8f, 228f, 14f, gold)
        text(c, "WIND OFF", 8f, 260f, 12f)
        // Tiny top right hole overview; do not allow a tall full-screen map.
        rect(c, 319f, 38f, 354f, 151f, Color.BLACK)
        rect(c, 321f, 40f, 352f, 149f, Color.rgb(13, 86, 16))
        rect(c, 331f, 56f, 343f, 130f, Color.rgb(73, 181, 30))
        circle(c, 336f, 55f, 8f, Color.rgb(106, 212, 59))
        circle(c, 337f, 137f, 3f, gold)
        flag(c, 336f, 53f)
    }
    /**
     * Welly-inspired semicircular arc, not the earlier horizontal prototype bar.
     * The cursor ascends for power and returns down the SAME arc for accuracy.
     * Exact timing/colours remain a reference comparison gate.
     */
    private fun drawMeter(c: Canvas) {
        rect(c, 91f, 319f, 354f, 469f, Color.BLACK)
        rect(c, 94f, 322f, 351f, 466f, navyDark)
        woodButton(c, "WHACK-O-METER", 103f, 331f, 239f, 28f)
        text(c, if (g.stage == GameStage.POWER) "BACKSWING: POWER"
            else "DOWNSWING: ACCURACY", 224f, 382f, 11.5f, gold, true)

        val cx = 223f
        val cy = 450f
        // Narrow straight-hit zone becomes harder from rough or sand.
        val safeTicks = when (g.lastLie) {
            Ground.SAND -> 2
            Ground.ROUGH -> 3
            else -> 5
        }
        // Small extension left of the bottom red zone shows a LATE strike.
        // The previous design clamped the cursor to zero and hid this error.
        for (i in -8..-1) {
            val px = cx - 100f + i * 3f
            val colour = if (i >= -safeTicks) Color.rgb(242, 52, 35)
                else Color.rgb(237, 205, 61)
            rect(c, px - 1f, cy - 3f, px + 2f, cy + 4f, colour)
        }
        // Original-era solid pixel steps create a crisp arch at mobile scale.
        for (i in 0..64) {
            val radians = Math.PI * (1.0 - i / 64.0)
            val px = cx + cos(radians).toFloat() * 100f
            val py = cy - sin(radians).toFloat() * 78f
            val colour = when {
                i <= safeTicks -> Color.rgb(242, 52, 35) // lie-sensitive red zone
                i in 25..39 -> Color.rgb(246, 183, 29) // strong-power zone
                i in 19..24 || i in 40..45 -> Color.rgb(103, 204, 42)
                else -> Color.rgb(237, 231, 186)
            }
            rect(c, px - 3f, py - 3f, px + 4f, py + 4f, Color.BLACK)
            rect(c, px - 2f, py - 2f, px + 3f, py + 3f, colour)
        }
        val t = g.meter
        val cursorX: Float
        val cursorY: Float
        if (t < 0f) {
            cursorX = cx - 100f + t.coerceAtLeast(-0.32f) * 75f
            cursorY = cy
        } else {
            val radians = Math.PI * (1.0 - t.coerceAtMost(1f).toDouble())
            cursorX = cx + cos(radians).toFloat() * 100f
            cursorY = cy - sin(radians).toFloat() * 78f
        }
        circle(c, cursorX, cursorY, 5.2f, Color.BLACK)
        circle(c, cursorX, cursorY, 3.7f, Color.WHITE)
        text(c, "MAX", cx, 369f, 10f, cream, true)
        text(c, if (g.stage == GameStage.POWER) "TAP TO SET POWER" else "TAP AT RED TO HIT",
            cx, 439f, 11f, cream, true)
    }
    private fun drawScore(c: Canvas) {
        pageHeader(c, "SCORECARD")
        val result = round?.results?.lastOrNull()
        woodButton(c, "${g.activeHole?.course?.title ?: "PIXTEE"} - HOLE ${g.holeNumber}",
            30f,125f,300f,49f)
        text(c, "PAR", 83f,232f,22f,gold)
        text(c, (result?.par ?: g.par).toString(), 269f,232f,22f)
        text(c, "STROKES",52f,291f,20f,gold)
        text(c, (result?.strokes ?: g.strokes).toString(),267f,291f,24f)
        text(c, "PENALTIES",52f,350f,19f,gold)
        text(c,(result?.penalties ?: g.penalties).toString(),268f,350f,24f)
        text(c,"ROUND TO PAR",47f,420f,18f,gold)
        val relative=round?.relativeToPar ?: g.scoreRelative
        text(c,(if(relative>0) "+" else "")+relative,275f,420f,23f)
        text(c, "${round?.results?.size ?: 1} OF ${round?.length ?: 1} HOLES",
            180f,488f,14f,cream,true)
        woodButton(c, if(round?.isComplete==true) "ROUND RESULTS" else "NEXT HOLE",
            50f,545f,260f,52f)
        woodButton(c,"MAIN MENU",50f,625f,260f,52f)
    }

    private fun drawResults(c: Canvas) {
        pageHeader(c, "ROUND RESULTS")
        val r=round ?: return
        text(c,r.layout().course.title.uppercase(),180f,122f,19f,gold,true)
        text(c,"${r.length} HOLES PLAYED",180f,163f,16f,cream,true)
        woodButton(c,"TOTAL ${r.totalStrokes} / PAR ${r.totalPar}",25f,210f,310f,62f)
        val rel=r.relativeToPar
        text(c,(if(rel>0) "+" else "")+rel,180f,345f,74f,gold,true)
        text(c,"PENALTIES ${r.totalPenalties}   PUTTS ${r.totalPutts}",
            180f,391f,14f,cream,true)
        text(c,if(lastReward.isNotEmpty()) lastReward else "ROUND COMPLETE",
            180f,477f,15f,gold,true)
        woodButton(c,"PLAY AGAIN",52f,544f,256f,52f)
        woodButton(c,"MAIN MENU",52f,628f,256f,52f)
    }

    private fun drawPause(c: Canvas) {
        navyBackdrop(c)
        text(c,"PAUSED",180f,245f,47f,gold,true)
        text(c,"${g.activeHole?.course?.title ?: "PIXTEE"} - HOLE ${g.holeNumber}",
            180f,295f,18f,cream,true)
        woodButton(c,"RESUME",50f,350f,260f,58f)
        woodButton(c,"EXIT ROUND",50f,440f,260f,58f)
    }
    private fun drawCareer(c: Canvas) {
        pageHeader(c, "PIXTEE CAREER")
        val mask=stats.getLong("career_mask",0L)
        val wins=PixteeCareer.titles(mask)
        woodButton(c,"SEASON 1 - ${wins} / 25 EVENTS",27f,135f,306f,52f)
        text(c,"TOUR PROGRESSION",180f,246f,20f,gold,true)
        tourNames.forEachIndexed { i,label ->
            val unlocked=PixteeCareer.tierUnlocked(mask,i)
            val earned=(i*5 until i*5+5).count{ PixteeCareer.completed(mask,it) }
            woodButton(c,label,33f,282f+i*60f,294f,43f,unlocked)
            text(c,if(unlocked) "$earned/5" else "LOCKED",
                300f,310f+i*60f,11f,gold,true)
        }
        val p=progress()
        text(c,"LEVEL ${p.level}   ${p.xp} XP",180f,619f,14f,gold,true)
        woodButton(c,"PRACTICE HOLE",37f,652f,286f,45f)
    }
    private fun drawStats(c: Canvas) {
        pageHeader(c, "STATISTICS")
        val p=progress()
        val labels=listOf("ROUNDS COMPLETED","HOLES PLAYED","SHOTS PLAYED",
            "HOLES UNDER PAR","TOTAL PUTTS","PENALTIES")
        val vals=listOf(p.rounds,p.holes,p.shots,stats.getInt("under_par",0),
            p.putts,p.penalties)
        labels.forEachIndexed { i,label ->
            val y=121f+i*82f
            woodButton(c,"",20f,y,320f,61f)
            text(c,label,32f,y+38f,13f,cream)
            text(c,vals[i].toString(),318f,y+38f,22f,gold,true)
        }
        text(c,"PIXTEE LEVEL ${p.level}   -   ${p.xp} XP",180f,645f,12f,gold,true)
        woodButton(c,"BACK",74f,674f,212f,46f)
    }
    private fun drawTrophies(c: Canvas) {
        pageHeader(c, "TROPHY CABINET")
        text(c,"EARNED THROUGH PLAY",180f,116f,15f,gold,true)
        val rewards=PixteeRewards.achievements(progress())
        rewards.forEachIndexed { i,a ->
            val y=130f+i*64f
            woodButton(c,(if(a.unlocked) "*  " else "-  ")+a.title,
                17f,y,326f,50f,a.unlocked)
        }
        text(c,"${rewards.count{it.unlocked}} / ${rewards.size} AWARDS",
            180f,2305f,13f,gold,true)
        woodButton(c,"BACK",74f,2340f,212f,47f)
    }
    private fun drawWardrobe(c: Canvas) {
        pageHeader(c, "CUSTOMIZE GOLFER")
        text(c,"EARN COSMETICS BY PLAYING",180f,105f,12f,gold,true)
        // Four-times enlarged preview; course art and physics remain unchanged.
        c.save()
        c.translate(180f,170f)
        c.scale(4.1f,4.1f)
        golfer(c,0f,0f)
        c.restore()
        text(c,"LEVEL ${progress().level}  -  ${progress().xp} XP",
            180f,194f,13f,gold,true)
        StyleSlot.values().forEachIndexed { i,slot ->
            val y=209f+i*48f
            woodButton(c,"${slot.name}: ${gear(slot).name}".take(29),
                26f,y,308f,40f,slot==currentStyle)
        }
        val unlocked=PixteeWardrobe.available(currentStyle,progress())
        text(c,"${unlocked.size} OF ${PixteeWardrobe.items.count{it.slot==currentStyle}} UNLOCKED",
            180f,610f,12f,gold,true)
        text(c,"EQUIPPED: ${gear(currentStyle).name}",
            180f,634f,12f,cream,true)
        woodButton(c,"< PREV",19f,649f,155f,46f)
        woodButton(c,"NEXT >",186f,649f,155f,46f)
        woodButton(c,"BACK",74f,706f,212f,43f)
    }

    private fun drawOptions(c: Canvas) {
        pageHeader(c, "OPTIONS")
        woodButton(c, "PORTRAIT FULL SCREEN", 30f, 162f, 300f, 53f)
        woodButton(c, "PIXEL GRAPHICS", 30f, 247f, 300f, 53f)
        woodButton(c, "WIND: OFF", 30f, 332f, 300f, 53f)
        woodButton(c, if (sound) "SOUND: ON" else "SOUND: OFF", 30f, 417f, 300f, 53f)
        woodButton(c, if (sponsorBoards) "COURSE BOARDS: ON" else "COURSE BOARDS: OFF",
            30f, 502f, 300f, 53f)
        woodButton(c, if (courseMotion) "SCENERY MOTION: ON" else "SCENERY MOTION: OFF",
            30f, 585f, 300f, 49f)
        text(c, "PIXEL WATER / GRASS / CROWD", 180f, 652f, 11f, cream, true)
        woodButton(c, "BACK", 74f, 670f, 212f, 47f)
    }

    private fun finishHole() {
        val current=round ?: run { screen=Screen.MAIN; return }
        if(current.isComplete) { screen=Screen.SCORE; return }
        val scored=current.record(g.strokes,g.penalties,g.putts)
        val e=stats.edit()
        e.putInt("holes_played",stats.getInt("holes_played",0)+1)
        e.putInt("shots",stats.getInt("shots",0)+scored.strokes)
        e.putInt("penalties",stats.getInt("penalties",0)+scored.penalties)
        e.putInt("putts",stats.getInt("putts",0)+scored.putts)
        if(scored.relative<0)
            e.putInt("under_par",stats.getInt("under_par",0)+1)
        if(scored.relative<=-2)
            e.putInt("eagles",stats.getInt("eagles",0)+1)
        if(scored.relative==-1)
            e.putInt("birdies",stats.getInt("birdies",0)+1)
        if(!stats.contains("best") || scored.strokes<stats.getInt("best",999))
            e.putInt("best",scored.strokes)
        if(current.isComplete) {
            e.putInt("rounds",stats.getInt("rounds",0)+1)
            lastReward=""
            if(current.length>=9 && current.results.take(9).all{it.strokes<=it.par})
                e.putInt("clean_nine",stats.getInt("clean_nine",0)+1)
            if(current.mode==RoundMode.CAREER && careerEventIndex in 0..24) {
                val mask=stats.getLong("career_mask",0L)
                val event=PixteeCareer.events[careerEventIndex]
                val newMask=PixteeCareer.award(mask,event,current.relativeToPar)
                if(newMask!=mask) {
                    e.putLong("career_mask",newMask)
                    lastReward="TOUR EVENT WON - TROPHY EARNED"
                } else lastReward="EVENT ENDED - RETRY TO WIN"
            }
        }
        e.apply()
        saveSession(null)
        screen=Screen.SCORE
    }

    /** Native touch and scroll handling against the same fixed logical drawing coordinates. */
    override fun onTouchEvent(event: MotionEvent): Boolean {
        val x = event.x * 360f / width.coerceAtLeast(1)
        val y = event.y * 360f / width.coerceAtLeast(1)
        when (event.actionMasked) {
            MotionEvent.ACTION_DOWN -> {
                android.util.Log.i("PixteeTouch", "DOWN screen=$screen x=$x y=$y width=$width height=$height")
                downX = x; downY = y
                if (screen != Screen.PLAYING) menuInput.down(x, y)
                return true
            }
            MotionEvent.ACTION_MOVE -> {
                if (screen != Screen.PLAYING) {
                    menuInput.move(x, y, MenuInput.maxScroll(logicalScreenHeight(),screenContentHeight()))
                    invalidate()
                }
                return true
            }
            MotionEvent.ACTION_CANCEL -> {
                menuInput.cancel()
                return true
            }
            MotionEvent.ACTION_UP -> {
                android.util.Log.i("PixteeTouch", "UP screen=$screen x=$x y=$y scroll=${menuInput.scrollY}")
                performClick()
                val previousScreen = screen
                if (screen == Screen.PLAYING) {
                    tapPlaying(x, y)
                } else {
                    val location = menuInput.release(x, y) ?: return true
                    tapMenu(location.first, location.second)
                }
                android.util.Log.i("PixteeTouch", "RESULT prev=$previousScreen screen=$screen stage=${g.stage}")
                if (screen != previousScreen) menuInput.reset()
                invalidate()
                return true
            }
        }
        return true
    }

    override fun performClick(): Boolean {
        super.performClick()
        return true
    }

    private fun tapMenu(x: Float, y: Float) {
        fun hit(l: Float, t: Float, w: Float, h: Float): Boolean =
            MenuInput.contains(x, y, l, t, l + w, t + h)
        when(screen) {
            Screen.MAIN -> {
                for(i in 0..6) {
                    if(!hit(44f,279f+i*66f,272f,49f))continue
                    when(i) {
                        0 -> screen=Screen.COURSES
                        1 -> screen=Screen.CAREER
                        2 -> if(savedGame()!=null) continueRound()
                             else startRound(RoundMode.PRACTICE,1)
                        3 -> screen=Screen.STATS
                        4 -> screen=Screen.TROPHIES
                        5 -> screen=Screen.OPTIONS
                        6 -> screen=Screen.WARDROBE
                    }
                    return
                }
            }
            Screen.COURSES -> {
                if(hit(6f,24f,348f,47f) || hit(75f,1550f,210f,46f)) {
                    screen=Screen.MAIN;return
                }
                PixteeCourseCatalog.courses.forEachIndexed { i,_ ->
                    if(hit(37f,132f+i*56f,286f,45f)) {
                        selectedCourse=i
                        screen=Screen.PLAYER
                        return
                    }
                }
            }
            Screen.PLAYER -> when {
                hit(6f,24f,348f,47f) || hit(76f,600f,208f,51f) ->
                    screen=Screen.COURSES
                hit(76f,527f,208f,51f) -> startRound(RoundMode.QUICK)
                y in 448f..496f -> {
                    listOf(1,3,9,18).forEachIndexed { i,n ->
                        if(hit(21f+i*81f,448f,73f,48f)) selectedLength=n
                    }
                }
            }
            Screen.TEE -> when {
                hit(63f,611f,234f,60f) -> {
                    screen=Screen.PLAYING;lastNs=0L;accumulator=0f
                }
                hit(6f,24f,348f,47f) -> screen=Screen.MAIN
            }
            Screen.SCORE -> when {
                hit(50f,545f,260f,52f) -> nextAfterScore()
                hit(50f,625f,260f,52f) || hit(6f,24f,348f,47f) ->
                    screen=Screen.MAIN
            }
            Screen.RESULTS -> when {
                hit(52f,544f,256f,52f) -> startRound(selectedMode,
                    round?.length ?: selectedLength)
                hit(52f,628f,256f,52f) || hit(6f,24f,348f,47f) ->
                    screen=Screen.MAIN
            }
            Screen.PAUSE -> when {
                hit(50f,350f,260f,58f) -> {
                    screen=Screen.PLAYING;lastNs=0L;accumulator=0f
                }
                hit(50f,440f,260f,58f) -> screen=Screen.MAIN
            }
            Screen.CAREER -> when {
                hit(6f,24f,348f,47f) -> screen=Screen.MAIN
                hit(37f,652f,286f,45f) -> startRound(RoundMode.PRACTICE,1)
                else -> {
                    val mask=stats.getLong("career_mask",0L)
                    tourNames.forEachIndexed { i,_ ->
                        if(hit(33f,282f+i*60f,294f,43f)) {
                            val ev=PixteeCareer.nextEvent(mask,i)
                                ?: PixteeCareer.events[i*5+4].takeIf {
                                    PixteeCareer.tierUnlocked(mask,i)
                                }
                            if(ev!=null) {
                                careerTier=i
                                careerEventIndex=ev.index
                                selectedCourse=ev.course
                                startRound(RoundMode.CAREER,ev.holes)
                            }
                            return
                        }
                    }
                }
            }
            Screen.STATS -> if(hit(6f,24f,348f,47f) ||
                hit(74f,674f,212f,46f)) screen=Screen.MAIN
            Screen.TROPHIES -> if(hit(6f,24f,348f,47f) ||
                hit(74f,2340f,212f,47f)) screen=Screen.MAIN
            Screen.WARDROBE -> when {
                hit(6f,24f,348f,47f) || hit(74f,706f,212f,43f) ->
                    screen=Screen.MAIN
                hit(19f,649f,322f,46f) -> {
                    val items=PixteeWardrobe.available(currentStyle,progress())
                    if(items.isNotEmpty()) {
                        val current=items.indexOfFirst{it.id==gear(currentStyle).id}
                        val delta=if(x<178f)-1 else 1
                        val next=(current+delta+items.size)%items.size
                        val nextEquipment=PixteeWardrobe.equip(equipment,items[next].id,progress())
                        equipment.clear()
                        equipment.putAll(nextEquipment)
                        saveGear()
                    }
                }
                else -> StyleSlot.values().forEachIndexed { i,slot ->
                    if(hit(26f,209f+i*48f,308f,40f)) {
                        currentStyle=slot
                        return
                    }
                }
            }
            Screen.OPTIONS -> when {
                hit(6f,24f,348f,47f) || hit(74f,670f,212f,47f) ->
                    screen=Screen.MAIN
                hit(30f,417f,300f,53f) -> sound=!sound
                hit(30f,502f,300f,53f) -> {
                    sponsorBoards=!sponsorBoards
                    courseOptions.edit().putBoolean("course_boards",sponsorBoards).apply()
                }
                hit(30f,585f,300f,49f) -> {
                    courseMotion=!courseMotion
                    courseOptions.edit().putBoolean("course_motion",courseMotion).apply()
                }
            }
            Screen.PLAYING -> Unit
        }
    }

    private fun tapPlaying(x: Float, y: Float) {
        when {
            g.stage == GameStage.HOLED -> finishHole()
            y >= logicalScreenHeight() - 57f -> when {
                x < 87f -> g.steer(-3.5f)
                x < 172f -> g.changeClub(1)
                x < 263f -> g.steer(3.5f)
                else -> g.whack()
            }
            g.stage == GameStage.READY && abs(x - downX) > 20f ->
                g.steer((x - downX) / 5f)
            g.stage == GameStage.POWER || g.stage == GameStage.ACCURACY -> g.whack()
            // Visible top-right pause control, never silently discards a round.
            y < 38f && x > 286f -> screen = Screen.PAUSE
        }
    }
}
