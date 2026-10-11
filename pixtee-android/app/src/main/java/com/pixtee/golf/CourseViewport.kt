package com.pixtee.golf

/**
 * Orthographic top-down camera projection. World coordinates, collision,
 * golfer dimensions and gameplay physics are NOT modified by visual zoom.
 * Always uses ONE uniform X/Y scale (no stretching on taller/foldable phones).
 *
 * Reference-style gameplay framing: the main phone viewport shows a segment
 * of the course and follows the ball, instead of shrinking the entire hole
 * to fit. Actual production zoom requires owner approval against a genuine
 * original-game screenshot at matched physical pixel sizes.
 */
class CourseViewport(
    val logicalScreenWidth: Float,
    val logicalScreenHeight: Float,
    val worldWidth: Float = PixteeCore.WIDTH,
    val authoredHoleHeight: Float = PixteeCore.HEIGHT,
    val zoom: Float = 1f,
    val focusX: Float = worldWidth / 2f,
    val focusY: Float = authoredHoleHeight / 2f
) {
    companion object {
        /** Provisional closer-camera ratio for real-phone A/B review, not approved. */
        const val REFERENCE_ZOOM_CANDIDATE = 1.9f
        /** Keeps aiming context ahead of the ball toward the green (negative Y). */
        const val LOOK_AHEAD_WORLD = 52f
    }

    init {
        require(logicalScreenWidth > 0f && logicalScreenHeight > 0f)
        require(worldWidth > 0f && authoredHoleHeight > 0f)
        require(zoom in 0.5f..4f)
        require(focusX.isFinite() && focusY.isFinite())
    }

    val worldScale: Float = logicalScreenWidth / worldWidth * zoom
    val visibleWorldWidth: Float = logicalScreenWidth / worldScale
    val visibleWorldHeight: Float = logicalScreenHeight / worldScale

    private fun followAxis(focus: Float, visibleSpan: Float, courseSpan: Float): Float =
        if (visibleSpan >= courseSpan) (courseSpan - visibleSpan) / 2f
        else (focus - visibleSpan / 2f).coerceIn(0f, courseSpan-visibleSpan)

    val leftWorld: Float = followAxis(focusX,visibleWorldWidth,worldWidth)
    val rightWorld: Float = leftWorld+visibleWorldWidth
    val topWorld: Float = followAxis(focusY,visibleWorldHeight,authoredHoleHeight)
    val bottomWorld: Float = topWorld+visibleWorldHeight

    fun screenX(worldX: Float): Float = (worldX-leftWorld) * worldScale
    fun screenY(worldY: Float): Float = (worldY-topWorld) * worldScale
    fun worldX(screenX: Float): Float = screenX / worldScale + leftWorld
    fun worldY(screenY: Float): Float = screenY / worldScale + topWorld
    fun renderedModelHeight(worldHeight: Float): Float = worldHeight * worldScale

    fun containsWorld(x: Float, y: Float): Boolean =
        x in leftWorld..rightWorld && y in topWorld..bottomWorld
}
