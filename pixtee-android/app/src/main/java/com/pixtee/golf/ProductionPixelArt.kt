package com.pixtee.golf

import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Canvas
import android.graphics.Paint
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
        "golfer_idle", "golfer_backswing", "golfer_impact", "golfer_follow",
        "tree_round", "tree_pine", "spectator_idle", "spectator_wave",
        "flower_yellow", "flower_pink"
    )

    fun golferFrame(stage: GameStage): String = when (stage) {
        GameStage.POWER -> "golfer_backswing"
        GameStage.ACCURACY -> "golfer_impact"
        GameStage.FLIGHT -> "golfer_follow"
        else -> "golfer_idle"
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
        canvas.drawBitmap(bitmap, centreX-bitmap.width/2f,
            bottomY-bitmap.height.toFloat(),paint)
    }
}
