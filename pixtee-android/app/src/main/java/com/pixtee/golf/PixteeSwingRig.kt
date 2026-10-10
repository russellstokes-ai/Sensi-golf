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
    PUTT("golfer_putt")
}

object PixteeSwingRig {
    /** World-space horizontal ball offset from the golfer's registered feet. */
    const val BALL_OFFSET_X = 12f
    /** World-space ball Y equals ground-level golfer feet Y at address. */
    const val BALL_OFFSET_Y = 0f

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
        if(stage==GameStage.READY || stage==GameStage.HOLED) return SwingPose.ADDRESS
        if(putter) {
            return when(stage) {
                GameStage.POWER -> if(backswingTicks<10) SwingPose.ADDRESS
                    else SwingPose.TAKEAWAY
                GameStage.ACCURACY -> SwingPose.DOWNSWING
                GameStage.FLIGHT, GameStage.ROLL ->
                    if(postContactTicks==0) SwingPose.IMPACT
                    else if(postContactTicks<13) SwingPose.PUTT
                    else SwingPose.ADDRESS
                else -> SwingPose.ADDRESS
            }
        }
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
}
