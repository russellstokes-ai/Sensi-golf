package com.pixtee.golf

import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Canvas
import android.graphics.Paint
import android.graphics.Rect
import android.graphics.RectF
import android.graphics.Shader
import android.graphics.BitmapShader
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

    val TERRAIN_TILES: List<String> = listOf(
        "terrain_rough", "terrain_fairway", "terrain_green",
        "terrain_sand", "terrain_water"
    )
    val UI_ART: List<String> = listOf(
        "ui_pixtee_logo", "ui_wood_button", "ui_hud_panel",
        "ui_sponsor_board"
    )

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
    /**
     * Unapproved review assets may be rendered ONLY by debuggable Android
     * builds and ONLY with a conspicuous review-only manifest. They never
     * become production assets and release Gradle forbids absent approvals.
     * No review images exist by default, so this cannot invent artwork.
     */
    private val reviewOnly: Boolean by lazy {
        val debuggable=(context.applicationInfo.flags and
            android.content.pm.ApplicationInfo.FLAG_DEBUGGABLE)!=0
        if(!debuggable || approved) false else try {
            context.assets.open("art/review/REVIEW_ONLY.txt").bufferedReader()
                .use { it.readText().trim() } ==
                "PIXTEE_ART_REVIEW_ONLY_NOT_APPROVED_V1"
        } catch (_: IOException) { false }
    }
    private val frames = mutableMapOf<String, Bitmap?>()
    private val textures = mutableMapOf<String, Paint>()

    private fun bitmap(id: String): Bitmap? {
        if(id !in ProductionArtContract.REQUIRED_SPRITES &&
            id !in ProductionArtContract.TERRAIN_TILES &&
            id !in ProductionArtContract.UI_ART) return null
        if(!approved && !reviewOnly) return null
        if(frames.containsKey(id)) return frames[id]
        val base=if(approved) "art/production" else "art/review"
        val loaded=try {
            context.assets.open("$base/$id.png").use { stream ->
                BitmapFactory.decodeStream(stream,null,BitmapFactory.Options().apply {
                    inScaled=false
                    inPreferredConfig=Bitmap.Config.ARGB_8888
                })
            }
        } catch (_: IOException) { null }
        frames[id]=loaded
        return loaded
    }

    /** Tile an approved artist-authored terrain bitmap in world space. */
    fun terrainRect(canvas: Canvas, id: String, l: Float, t: Float,
                    r: Float, b: Float): Boolean {
        if(id !in ProductionArtContract.TERRAIN_TILES) return false
        val bitmap=bitmap(id) ?: return false
        val tilePaint=textures.getOrPut(id) {
            Paint().apply {
                isAntiAlias=false;isFilterBitmap=false;isDither=false
                shader=BitmapShader(bitmap,Shader.TileMode.REPEAT,Shader.TileMode.REPEAT)
            }
        }
        canvas.drawRect(l,t,r,b,tilePaint)
        return true
    }

    fun terrainOval(canvas: Canvas, id: String, l: Float, t: Float,
                    r: Float, b: Float): Boolean {
        if(id !in ProductionArtContract.TERRAIN_TILES) return false
        val bitmap=bitmap(id) ?: return false
        val tilePaint=textures.getOrPut(id) {
            Paint().apply {
                isAntiAlias=false;isFilterBitmap=false;isDither=false
                shader=BitmapShader(bitmap,Shader.TileMode.REPEAT,Shader.TileMode.REPEAT)
            }
        }
        canvas.drawOval(l,t,r,b,tilePaint)
        return true
    }

    /** Approved logos/panels render independently of the world camera. */
    fun ui(canvas: Canvas, id: String, l: Float, t: Float,
           r: Float, b: Float): Boolean {
        if(id !in ProductionArtContract.UI_ART) return false
        val bitmap=bitmap(id) ?: return false
        canvas.drawBitmap(bitmap,Rect(0,0,bitmap.width,bitmap.height),
            RectF(l,t,r,b),paint)
        return true
    }

    /**
     * Draw only owner-approved files using their native pixel dimensions.
     * Never draw an empty silhouette, programmatic replacement or downloaded
     * third-party artwork when a frame is missing or rejected.
     */
    fun draw(canvas: Canvas, id: String, centreX: Float, bottomY: Float) {
        if ((!approved && !reviewOnly) || id !in ProductionArtContract.REQUIRED_SPRITES) return
        val bitmap=bitmap(id) ?: return
        val dst=if(id.startsWith("golfer_")) {
            // The golfer frame uses the SAME pixel-to-world ratio across all
            // poses, even when the club rotates above the head. Image bounds
            // are never scaled relative to opaque contents or cropped.
            if(!PixteeSwingRig.isValidFrameSize(bitmap.width,bitmap.height))
                return
            val r=PixteeSwingRig.artFrameRect(centreX,bottomY)
            RectF(r[0],r[1],r[2],r[3])
        } else {
            val worldHeight=ProductionArtContract.candidateWorldHeight(id)
            val worldWidth=worldHeight*bitmap.width/
                bitmap.height.coerceAtLeast(1).toFloat()
            RectF(centreX-worldWidth/2f,bottomY-worldHeight,
                centreX+worldWidth/2f,bottomY)
        }
        canvas.drawBitmap(bitmap,Rect(0,0,bitmap.width,bitmap.height),dst,paint)
    }
}
