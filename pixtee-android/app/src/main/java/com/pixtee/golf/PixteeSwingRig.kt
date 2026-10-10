package com.pixtee.golf

/**
 * Eight distinct swing drawings and a synchronized club/ball-contact
 * contract. Does NOT generate sprites, reuse original-game images, or alter
 * shot physics. The exact artwork for these frame IDs requires owner approval.
 */
enum class SwingPose(val spriteId: String) {
    ADDRESS("golfer_idle"),
    TAKEAWAY("golfer_takeaway"),
    BACKSWING("golfer_backswing"),
    TOP("golfer_top"),
    DOWNSWING("golfer_downswing"),
    IMPACT("golfer_impact"),
    FOLLOW_THROUGH("golfer_follow"),
    FINISH("golfer_finish"),
    PUTT_READY("golfer_putt_ready"),
    PUTT_BACK("golfer_putt_back"),
    PUTT_IMPACT("golfer_putt_impact"),
    PUTT_FINISH("golfer_putt_finish")
}

object PixteeSwingRig {
    /** World-space horizontal ball offset from the golfer's registered feet. */
    const val BALL_OFFSET_X = 3.5f
    /** World-space ball Y equals ground-level golfer feet Y at address. */
    const val BALL_OFFSET_Y = 0f

    /**
     * Mandatory matching transparent canvas for all production golfer poses.
     * Registration is measured from PIXEL ANCHORS, not bitmap centering.
     *
     * Pivot of both shoes = (80, 116) and contact clubface pixel = (108, 116).
     * 28 source pixels represent 3.5 world units, so the clubface meets
     * the physical ball at shot launch without stretching frames.
     *
     * These are proposed final export anchors from the production brief.
     * Original-game parity and on-device contact still require visual review.
     */
    const val FRAME_PX_W = 256
    const val FRAME_PX_H = 128
    const val FEET_PX_X = 80f
    const val FEET_PX_Y = 116f
    const val IMPACT_CLUB_PX_X = 108f
    const val IMPACT_CLUB_PX_Y = 116f
    const val SOURCE_PIXELS_PER_WORLD =
        (IMPACT_CLUB_PX_X-FEET_PX_X)/BALL_OFFSET_X

    fun isValidFrameSize(width: Int, height: Int): Boolean =
        width==FRAME_PX_W && height==FRAME_PX_H

    /** Temporary debug-only compatibility; callers must enforce the debug gate. */
    fun isLegacyFrameSize(width: Int, height: Int): Boolean =
        width == 128 && height == 64

    fun artFrameRect(feetWorldX: Float, feetWorldY: Float): FloatArray {
        val left=feetWorldX-FEET_PX_X/SOURCE_PIXELS_PER_WORLD
        val top=feetWorldY-FEET_PX_Y/SOURCE_PIXELS_PER_WORLD
        return floatArrayOf(
            left,top,
            left+FRAME_PX_W/SOURCE_PIXELS_PER_WORLD,
            top+FRAME_PX_H/SOURCE_PIXELS_PER_WORLD
        )
    }

    fun contactFromFrameRect(rect: FloatArray): Pair<Float,Float> =
        (rect[0]+IMPACT_CLUB_PX_X/SOURCE_PIXELS_PER_WORLD) to
            (rect[1]+IMPACT_CLUB_PX_Y/SOURCE_PIXELS_PER_WORLD)

    /**
     * Impact target in common sprite-layout coordinates (not native pixels).
     * The golfer frame is drawn at the same registered feet pivot every time.
     *
     * Production artwork must place the clubface over these registered
     * coordinates at IMPACT. That requirement is visually verified after
     * artist-supplied PNGs arrive; this code cannot certify unseen art.
     */
    fun expectedBallPosition(feetX: Float, feetY: Float): Pair<Float, Float> =
        (feetX+BALL_OFFSET_X) to (feetY+BALL_OFFSET_Y)

    fun phase(stage: GameStage, backswingTicks: Int,
              downswingTicks: Int, postContactTicks: Int,
              putter: Boolean = false): SwingPose {
        // Putting is a separate four-frame animation family. It must never
        // display a full-swing impact or driver follow-through sprite.
        if(putter) {
            return when(stage) {
                GameStage.READY, GameStage.HOLED -> SwingPose.PUTT_READY
                GameStage.POWER -> if(backswingTicks<10) SwingPose.PUTT_READY
                    else SwingPose.PUTT_BACK
                GameStage.ACCURACY -> SwingPose.PUTT_BACK
                GameStage.FLIGHT, GameStage.ROLL -> when {
                    postContactTicks==0 -> SwingPose.PUTT_IMPACT
                    postContactTicks<13 -> SwingPose.PUTT_FINISH
                    else -> SwingPose.PUTT_READY
                }
            }
        }
        if(stage==GameStage.READY || stage==GameStage.HOLED) return SwingPose.ADDRESS
        return when(stage) {
            GameStage.POWER -> when {
                backswingTicks<7 -> SwingPose.ADDRESS
                backswingTicks<16 -> SwingPose.TAKEAWAY
                backswingTicks<25 -> SwingPose.BACKSWING
                else -> SwingPose.TOP
            }
            GameStage.ACCURACY -> when {
                downswingTicks<5 -> SwingPose.TOP
                else -> SwingPose.DOWNSWING
            }
            GameStage.FLIGHT, GameStage.ROLL -> when {
                postContactTicks==0 -> SwingPose.IMPACT
                postContactTicks<7 -> SwingPose.FOLLOW_THROUGH
                postContactTicks<58 -> SwingPose.FINISH
                else -> SwingPose.ADDRESS
            }
            else -> SwingPose.ADDRESS
        }
    }

    fun phase(core: PixteeCore): SwingPose = phase(core.stage,
        core.backswingTicks,core.downswingTicks,core.postContactTicks,
        core.clubIndex==PixteeCore.CLUBS.lastIndex
    )

    /** This is the only accepted full-swing order for art review. */
    val FULL_SWING = listOf(
        SwingPose.ADDRESS, SwingPose.TAKEAWAY, SwingPose.BACKSWING,
        SwingPose.TOP, SwingPose.DOWNSWING, SwingPose.IMPACT,
        SwingPose.FOLLOW_THROUGH, SwingPose.FINISH
    )

    /** Approved 12-pose source sheet supplies four distinct putter frames. */
    val PUTTING = listOf(
        SwingPose.PUTT_READY, SwingPose.PUTT_BACK,
        SwingPose.PUTT_IMPACT, SwingPose.PUTT_FINISH
    )
}
