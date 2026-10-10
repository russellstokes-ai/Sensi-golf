package com.pixtee.golf

import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Canvas
import android.graphics.Paint
import android.graphics.Rect
import android.graphics.RectF
import java.io.IOException

/**
 * PRODUCTION ART ONLY. There are deliberately no code-generated characters,
 * pixel glyphs, provisional shapes or substitute sprites.
 *
 * The rejected October sprite batch is not a permitted fallback. During engine
 * development (before owner approval), these calls draw NOTHING. Release builds
 * are separately blocked until the required approved assets are present.
 */
object ProductionArtContract {
    const val APPROVAL_MARKER = "PIXTEE_PRODUCTION_ART_APPROVED_V1"
    const val MANIFEST = "art/production/APPROVED_v1.txt"

    val REQUIRED_SPRITES: List<String> = listOf(
        "golfer_idle", "golfer_takeaway", "golfer_backswing", "golfer_top",
        "golfer_downswing", "golfer_impact", "golfer_follow", "golfer_finish",
        "golfer_putt", "tree_round", "tree_pine", "spectator_idle",
        "spectator_wave", "spectator_photographer", "camera_flash",
        "bird_wings_up", "bird_wings_down", "flower_yellow", "flower_pink"
    )

    /** All poses, including a corrected club direction after impact. */
    fun golferFrame(game: PixteeCore): String =
        PixteeSwingRig.phase(game).spriteId

    /**
     * Candidate world-space size, NOT an art approval. Source PNG resolution
     * may be higher to permit detailed/cuter character art, but on-course
     * proportions remain stable and independent of that resolution.
     * Exact targets must be approved using a native phone-scale screenshot.
     */
    fun candidateWorldHeight(id: String): Float = when (id) {
        in PixteeSwingRig.FULL_SWING.map { it.spriteId }, "golfer_putt" -> 11.5f
        "tree_round" -> 37f
        "tree_pine" -> 42f
        "spectator_idle", "spectator_wave", "spectator_photographer" -> 12f
        "camera_flash" -> 5f
        "bird_wings_up", "bird_wings_down" -> 7f
        "flower_yellow", "flower_pink" -> 4f
        else -> throw IllegalArgumentException("Unknown production sprite: $id")
    }
}

class ProductionPixelArt(private val context: Context) {
    private val paint = Paint().apply {
        isAntiAlias = false
        isFilterBitmap = false
        isDither = false
    }
    private val approved: Boolean by lazy {
        try {
            context.assets.open(ProductionArtContract.MANIFEST).bufferedReader()
                .use { it.readText().trim() } == ProductionArtContract.APPROVAL_MARKER
        } catch (_: IOException) { false }
    }
    private val frames = mutableMapOf<String, Bitmap?>()

    /**
     * Draw only owner-approved files using their native pixel dimensions.
     * Never draw an empty silhouette, programmatic replacement or downloaded
     * third-party artwork when a frame is missing or rejected.
     */
    fun draw(canvas: Canvas, id: String, centreX: Float, bottomY: Float) {
        if (!approved || id !in ProductionArtContract.REQUIRED_SPRITES) return
        val bitmap = if (frames.containsKey(id)) frames[id] else {
            val loaded = try {
                context.assets.open("art/production/$id.png").use { stream ->
                    BitmapFactory.decodeStream(stream, null, BitmapFactory.Options().apply {
                        inScaled = false
                        inPreferredConfig = Bitmap.Config.ARGB_8888
                    })
                }
            } catch (_: IOException) { null }
            frames[id] = loaded
            loaded
        } ?: return
        val worldHeight=ProductionArtContract.candidateWorldHeight(id)
        val worldWidth=worldHeight*bitmap.width/bitmap.height.coerceAtLeast(1).toFloat()
        // Nearest-neighbour only. Preserve artwork aspect ratio, center on
        // ball/world anchor, and use the SAME camera zoom as other geometry.
        val dst=RectF(centreX-worldWidth/2f,bottomY-worldHeight,
            centreX+worldWidth/2f,bottomY)
        canvas.drawBitmap(bitmap,Rect(0,0,bitmap.width,bitmap.height),dst,paint)
    }
}
