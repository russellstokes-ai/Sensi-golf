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

private enum class Screen { MAIN, COURSES, PLAYER, TEE, PLAYING, SCORE, CAREER, STATS, TROPHIES, OPTIONS }

/** Every UI element is laid out against a TALL 360x760 logical phone, then scaled edge-to-edge. */
private class PixteeCanvas(context: Context) : View(context) {
    private val p = Paint().apply { isAntiAlias = false; isFilterBitmap = false
        typeface = Typeface.create(Typeface.MONOSPACE, Typeface.BOLD) }
    private val g = PixteeCore()
    private val menuInput = MenuInput()
    private val stats = context.getSharedPreferences("pixtee_stats_v1", Context.MODE_PRIVATE)
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
    private val sponsorCampaigns = readSponsorManifest(context)
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
    private val holes = listOf("LAKEWOOD", "RIVERDALE", "OAK VALLEY", "SUNRIDGE",
        "PINE CREST", "COASTLINE")
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
            while (accumulator >= 1f / 60f && steps < 4) {
                g.tick()
                accumulator -= 1f / 60f
                steps++
            }
        }
        lastNs = now
        canvas.save()
        // One X/Y scale: a tall handset reveals MORE world vertically;
        // neither golfers nor collision geometry get stretched.
        val pixelScale = width / 360f
        canvas.scale(pixelScale, pixelScale)
        if (screen != Screen.PLAYING) canvas.translate(0f, -menuInput.scrollY)
        when (screen) {
            Screen.MAIN -> drawMain(canvas)
            Screen.COURSES -> drawCourses(canvas)
            Screen.PLAYER -> drawPlayer(canvas)
            Screen.TEE -> drawTee(canvas)
            Screen.PLAYING -> drawPlaying(canvas)
            Screen.SCORE -> drawScore(canvas)
            Screen.CAREER -> drawCareer(canvas)
            Screen.STATS -> drawStats(canvas)
            Screen.TROPHIES -> drawTrophies(canvas)
            Screen.OPTIONS -> drawOptions(canvas)
        }
        canvas.restore()
        val state = "Pixtee screen: ${screen.name}" +
            if (screen == Screen.PLAYING) "; stage: ${g.stage.name}; strokes: ${g.strokes}" else ""
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
        for (y in 0..760 step 14) for (x in 0..360 step 23) {
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
        val buttons = listOf("PLAY ROUND", "CAREER", "PRACTICE HOLE",
            "STATISTICS", "TROPHIES", "OPTIONS")
        for ((i, label) in buttons.withIndex())
            woodButton(c, label, 44f, 279f + i * 66f, 272f, 49f)
        text(c, "CLASSIC 2D ARCADE GOLF", 180f, 732f, 11f, cream, true)
    }
    private fun drawCourses(c: Canvas) {
        pageHeader(c, "SELECT GOLF COURSE")
        text(c, "ORIGINAL PIXTEE COURSE", 180f, 105f, 13f, gold, true)
        for ((i, name) in holes.withIndex()) {
            woodButton(c, name, 37f, 132f + i * 78f, 286f, 51f, i == 0)
            if (i != 0) text(c, "COMING SOON", 268f, 190f + i * 78f, 10f)
        }
        woodButton(c, "BACK", 75f, 677f, 210f, 46f)
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
        woodButton(c, "OKAY", 76f, 527f, 208f, 51f)
        woodButton(c, "EXIT", 76f, 600f, 208f, 51f)
    }
    private fun drawTee(c: Canvas) {
        pageHeader(c, "NEXT TO THE TEE")
        woodButton(c, "LAKEWOOD", 40f, 128f, 280f, 54f)
        woodButton(c, "HOLE 1 (PAR 4)", 40f, 203f, 280f, 51f)
        text(c, "NAME", 81f, 321f, 23f, gold)
        text(c, "SCORE", 231f, 321f, 23f, gold)
        text(c, "HUMAN 1", 50f, 381f, 22f)
        text(c, "PAR", 244f, 381f, 22f, gold)
        woodButton(c, "TEE OFF", 63f, 611f, 234f, 60f)
    }

    /** First original Pixtee hole: newly-authored terrain data and renderer. */
    private fun course(c: Canvas) {
        fill(c, Color.rgb(37, 121, 19))
        // No fake stretched 4:3 bitmap: the portrait viewport exposes
        // extra world above and below the original-sized hole.
        val viewport = CourseViewport(360f, logicalScreenHeight())
        c.save()
        c.scale(viewport.worldScale, viewport.worldScale)
        c.translate(0f, -viewport.topWorld)
        rect(c, 0f, viewport.topWorld, 300f, viewport.bottomWorld, Color.rgb(48, 131, 25))
        // Alternating rough checkerboard/pixel dithering from new procedural art.
        for (y in (viewport.topWorld.toInt() - 10)..(viewport.bottomWorld.toInt() + 10) step 10) for (x in 0..300 step 11)
            if ((x * 17 + y * 13) % 7 < 3)
                rect(c, x.toFloat(), y.toFloat(), x + 3f, y + 3f, Color.rgb(53, 139, 26))
        for (y in 72..462 step 3) {
            val cx = 148f + sin(y / 79f) * 28f
            rect(c, cx - 51f, y.toFloat(), cx + 51f, y + 3f, Color.rgb(52, 153, 23))
            rect(c, cx - 43f, y.toFloat(), cx + 43f, y + 3f,
                if ((y / 21) % 2 == 0) Color.rgb(72, 185, 33) else Color.rgb(62, 173, 25))
        }
        val ambientFrame = CourseAmbient.frame(android.os.SystemClock.uptimeMillis(), courseMotion)
        // 8-frame Zelda-era pixel ripple: replaces static blue diagonal scribbles.
        // The underlying water hazard in PixteeCore.groundAt() is never modified.
        rect(c, 231f, 185f, 300f, 302f, Color.rgb(8, 79, 174))
        for (row in 0..13) {
            val yy = 189f + row * 8f
            val offset = CourseAmbient.waterRipple(row, ambientFrame)
            for (column in 0..3) {
                val xx = 234f + column * 16f + offset
                val endX = min(298f, xx + 7f)
                if (endX > xx) {
                    rect(c, xx, yy, endX, yy + 2f,
                        if ((row + column) % 3 == 0) Color.rgb(86, 165, 241)
                        else Color.rgb(29, 118, 211))
                }
            }
        }
        ellipse(c, 73f, 104f, 104f, 158f, Color.rgb(225, 200, 110))
        ellipse(c, 208f, 122f, 240f, 174f, Color.rgb(232, 210, 137))
        circle(c, PixteeCore.PIN_X, PixteeCore.PIN_Y, 35f, Color.rgb(82, 185, 41))
        circle(c, PixteeCore.PIN_X, PixteeCore.PIN_Y, 28f, Color.rgb(111, 207, 47))
        for (i in 0 until 13) {
            val gy = 43f + i * 4f
            line(c, 114f, gy, 174f, gy, Color.rgb(107, 197, 46))
        }
        // Low-amplitude grass, flowers and idle spectators: purely visual sprites.
        // Coarse frame stepping prevents distracting 60fps shimmering.
        for ((i, patch) in grassSpots.withIndex()) {
            val (gx, gy) = patch
            val offset = CourseAmbient.grassSway(i, ambientFrame).toFloat()
            line(c, gx, gy + 2f, gx + offset, gy - 1f, Color.rgb(81, 174, 46))
            rect(c, gx + offset, gy - 2f, gx + offset + 1f, gy - 1f,
                Color.rgb(116, 197, 51))
        }
        for ((i, spot) in flowers.withIndex()) {
            val (fx, fy) = spot
            val offset = CourseAmbient.grassSway(i + 3, ambientFrame).toFloat()
            line(c, fx, fy + 2f, fx + offset, fy - 1f, Color.rgb(23, 100, 23))
            rect(c, fx + offset - 1f, fy - 3f, fx + offset + 2f, fy - 1f,
                if (i % 3 == 0) Color.rgb(255, 233, 108)
                else Color.rgb(248, 214, 229))
            rect(c, fx + offset, fy - 2.5f, fx + offset + 1f, fy - 1.5f,
                Color.rgb(246, 170, 38))
        }
        // Trees are small sprites and NEVER obscure the ball/tee by design.
        for ((tx, ty) in trees) tree(c, tx, ty)
        for ((i, spot) in spectators.withIndex()) {
            spectator(c, spot.first, spot.second, i, ambientFrame)
        }
        rect(c, 130f, 448f, 173f, 468f, Color.rgb(83, 185, 40))
        circle(c, 137f, 464f, 2.2f, Color.WHITE)
        circle(c, 165f, 464f, 2.2f, Color.WHITE)
        flag(c, PixteeCore.PIN_X, PixteeCore.PIN_Y)
        // Signs are world-space decoration, not collision objects or HUD ads.
        // Draw AFTER terrain/trees, BEFORE phone HUD and swing controls.
        if (sponsorBoards) {
            val shown = SponsorInventory.show(
                sponsorSlots, sponsorCampaigns, true,
                System.currentTimeMillis() / 1000L
            )
            for (board in shown) drawSponsorBoard(c, board)
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
        rect(c, x - 2f, y - 14f, x + 3f, y - 11f, Color.rgb(245, 210, 69))
        rect(c, x - 3f, y - 12f, x + 4f, y - 10f, Color.rgb(227, 178, 42))
        rect(c, x - 3f, y - 10f, x + 3f, y - 4f, Color.rgb(42, 73, 200))
        rect(c, x - 3f, y - 4f, x - 1f, y + 1f, Color.rgb(18, 29, 41))
        rect(c, x + 1f, y - 4f, x + 3f, y + 1f, Color.rgb(18, 29, 41))
        line(c, x + 3f, y - 7f, x + 8f, y - 2f, Color.WHITE)
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
        if (g.height > 1f) circle(c, bx, by, 3f, Color.rgb(255, 244, 174))
        else circle(c, bx, by, 2.5f, Color.WHITE)
        drawHUD(c)
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
            text(c, "HOLED OUT!", 183f, 365f, 29f, gold, true)
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
        text(c, "LAKEWOOD", 8f, 61f, 15f, gold)
        text(c, "HOLE 1 PAR 4", 8f, 81f, 11f)
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
    private fun drawMeter(c: Canvas) {
        rect(c, 91f, 324f, 353f, 464f, Color.BLACK)
        rect(c, 95f, 328f, 349f, 460f, navyDark)
        woodButton(c, "WHACK-O-METER", 104f, 335f, 236f, 32f)
        text(c, if (g.stage == GameStage.POWER) "1. SET POWER" else "2. SET ACCURACY",
            222f, 389f, 16f, gold, true)
        for (i in 0 until 30) {
            val color = when {
                i in 11..18 -> Color.rgb(72, 218, 35)
                i in 6..10 || i in 19..23 -> Color.rgb(245, 216, 36)
                else -> Color.rgb(242, 75, 30)
            }
            rect(c, 107f + i * 7.6f, 399f, 114f + i * 7.6f, 420f, color)
        }
        val markerX = 107f + g.meter * 228f
        rect(c, markerX - 2f, 394f, markerX + 2f, 428f, cream)
        text(c, "TAP WHACK TO STOP", 223f, 446f, 13f, cream, true)
    }
    private fun drawScore(c: Canvas) {
        pageHeader(c, "SCORECARD")
        woodButton(c, "LAKEWOOD - HOLE 1", 37f, 125f, 286f, 49f)
        text(c, "PAR", 83f, 232f, 22f, gold)
        text(c, "4", 269f, 232f, 22f)
        text(c, "STROKES", 52f, 291f, 20f, gold)
        text(c, g.strokes.toString(), 267f, 291f, 24f)
        text(c, "PENALTIES", 52f, 350f, 19f, gold)
        text(c, g.penalties.toString(), 268f, 350f, 24f)
        text(c, "RELATIVE TO PAR", 47f, 420f, 16f, gold)
        text(c, (if (g.scoreRelative > 0) "+" else "") + g.scoreRelative, 275f, 420f, 23f)
        woodButton(c, "REPLAY HOLE", 50f, 545f, 260f, 52f)
        woodButton(c, "MAIN MENU", 50f, 625f, 260f, 52f)
    }
    private fun drawCareer(c: Canvas) {
        pageHeader(c, "CAREER")
        woodButton(c, "SEASON 1 - ROOKIE", 27f, 135f, 306f, 52f)
        text(c, "TOUR STRUCTURE", 180f, 251f, 20f, gold, true)
        listOf("AMATEUR TOUR", "REGIONAL TOUR", "NATIONAL TOUR", "PRO TOUR",
            "WORLD TOUR").forEachIndexed { i, label ->
            woodButton(c, label, 33f, 282f + i * 60f, 294f, 43f, i == 0)
        }
        text(c, "CAREER EVENTS IN DEVELOPMENT", 180f, 632f, 12f, cream, true)
        woodButton(c, "PRACTICE FIRST HOLE", 37f, 652f, 286f, 45f)
    }
    private fun drawStats(c: Canvas) {
        pageHeader(c, "STATISTICS")
        val labels = listOf("ROUNDS FINISHED", "SHOTS PLAYED", "HOLES UNDER PAR",
            "PENALTY STROKES", "BEST HOLE")
        val vals = listOf(stats.getInt("rounds", 0).toString(), stats.getInt("shots", 0).toString(),
            stats.getInt("under_par", 0).toString(), stats.getInt("penalties", 0).toString(),
            if (stats.contains("best")) stats.getInt("best", 0).toString() else "-")
        labels.forEachIndexed { i, label ->
            woodButton(c, "", 20f, 160f + i * 85f, 320f, 62f)
            text(c, label, 33f, 196f + i * 85f, 15f, cream)
            text(c, vals[i], 318f, 196f + i * 85f, 22f, gold, true)
        }
        woodButton(c, "BACK", 74f, 674f, 212f, 46f)
    }
    private fun drawTrophies(c: Canvas) {
        pageHeader(c, "TROPHY CABINET")
        text(c, "CLASSIC ARCADE ACHIEVEMENTS", 180f, 130f, 15f, gold, true)
        listOf("FIRST HOLE FINISHED", "FIRST BIRDIE", "FIRST EAGLE", "BOGEY-FREE ROUND",
            "TEN ROUNDS", "TOUR CHAMPION").forEachIndexed { i, a ->
            woodButton(c, (if (i == 0 && stats.getInt("rounds", 0) > 0) "*" else "-") +
                "  " + a, 17f, 163f + i * 77f, 326f, 53f,
                i == 0 && stats.getInt("rounds", 0) > 0)
        }
        woodButton(c, "BACK", 74f, 667f, 212f, 47f)
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
        val rounds = stats.getInt("rounds", 0) + 1
        val shots = stats.getInt("shots", 0) + g.strokes
        val e = stats.edit().putInt("rounds", rounds).putInt("shots", shots)
            .putInt("penalties", stats.getInt("penalties", 0) + g.penalties)
        if (g.strokes < g.par) e.putInt("under_par", stats.getInt("under_par", 0) + 1)
        if (!stats.contains("best") || g.strokes < stats.getInt("best", 999)) e.putInt("best", g.strokes)
        e.apply()
        screen = Screen.SCORE
    }

    /** Native touch and scroll handling against the same fixed logical drawing coordinates. */
    override fun onTouchEvent(event: MotionEvent): Boolean {
        val x = event.x * 360f / width.coerceAtLeast(1)
        val y = event.y * 360f / width.coerceAtLeast(1)
        when (event.actionMasked) {
            MotionEvent.ACTION_DOWN -> {
                downX = x; downY = y
                if (screen != Screen.PLAYING) menuInput.down(x, y)
                return true
            }
            MotionEvent.ACTION_MOVE -> {
                if (screen != Screen.PLAYING) {
                    menuInput.move(x, y, MenuInput.maxScroll(logicalScreenHeight()))
                    invalidate()
                }
                return true
            }
            MotionEvent.ACTION_CANCEL -> {
                menuInput.cancel()
                return true
            }
            MotionEvent.ACTION_UP -> {
                performClick()
                val previousScreen = screen
                if (screen == Screen.PLAYING) {
                    tapPlaying(x, y)
                } else {
                    val location = menuInput.release(x, y) ?: return true
                    tapMenu(location.first, location.second)
                }
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
        when (screen) {
            Screen.MAIN -> {
                for (i in 0..5) {
                    if (!hit(44f, 279f + i * 66f, 272f, 49f)) continue
                    when (i) {
                        0 -> screen = Screen.COURSES
                        1 -> screen = Screen.CAREER
                        2 -> { g.restart(); g.practice = true; screen = Screen.TEE }
                        3 -> screen = Screen.STATS
                        4 -> screen = Screen.TROPHIES
                        5 -> screen = Screen.OPTIONS
                    }
                    return
                }
            }
            Screen.COURSES -> when {
                hit(6f, 24f, 348f, 47f) || hit(75f, 677f, 210f, 46f) -> screen = Screen.MAIN
                hit(37f, 132f, 286f, 51f) -> screen = Screen.PLAYER
                // Unbuilt course entries deliberately cannot launch a placeholder hole.
            }
            Screen.PLAYER -> when {
                hit(76f, 527f, 208f, 51f) -> screen = Screen.TEE
                hit(6f, 24f, 348f, 47f) || hit(76f, 600f, 208f, 51f) ->
                    screen = Screen.COURSES
            }
            Screen.TEE -> when {
                hit(63f, 611f, 234f, 60f) -> {
                    g.restart(); screen = Screen.PLAYING; lastNs = 0L; accumulator = 0f
                }
                hit(6f, 24f, 348f, 47f) -> screen = Screen.PLAYER
            }
            Screen.SCORE -> when {
                hit(50f, 545f, 260f, 52f) -> { g.restart(); screen = Screen.TEE }
                hit(50f, 625f, 260f, 52f) || hit(6f, 24f, 348f, 47f) ->
                    screen = Screen.MAIN
            }
            Screen.CAREER -> when {
                hit(6f, 24f, 348f, 47f) -> screen = Screen.MAIN
                hit(37f, 652f, 286f, 45f) -> {
                    g.restart(); g.practice = true; screen = Screen.TEE
                }
            }
            Screen.STATS, Screen.TROPHIES -> {
                if (hit(6f, 24f, 348f, 47f) || hit(74f, 667f, 212f, 53f)) {
                    screen = Screen.MAIN
                }
            }
            Screen.OPTIONS -> when {
                hit(6f, 24f, 348f, 47f) || hit(74f, 670f, 212f, 47f) ->
                    screen = Screen.MAIN
                hit(30f, 417f, 300f, 53f) -> sound = !sound
                hit(30f, 502f, 300f, 53f) -> {
                    sponsorBoards = !sponsorBoards
                    courseOptions.edit().putBoolean("course_boards", sponsorBoards).apply()
                }
                hit(30f, 585f, 300f, 49f) -> {
                    courseMotion = !courseMotion
                    courseOptions.edit().putBoolean("course_motion", courseMotion).apply()
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
            // Keep the upper strip as an explicit exit gesture until pause is implemented.
            y < 38f -> screen = Screen.MAIN
        }
    }
}
