package com.pixtee.golf

/**
 * Screen projection only. NEVER scales the physics world or the golfer
 * differently across X/Y. Portrait reveals extra vertical playfield.
 *
 * The baseline artwork and model sizes are independent Pixtee placeholders:
 * exact comparisons with observable Sensible Golf footage are a calibration gate.
 */
class CourseViewport(
    val logicalScreenWidth: Float,
    val logicalScreenHeight: Float,
    val worldWidth: Float = PixteeCore.WIDTH,
    val authoredHoleHeight: Float = PixteeCore.HEIGHT
) {
    init {
        require(logicalScreenWidth > 0f && logicalScreenHeight > 0f)
        require(worldWidth > 0f && authoredHoleHeight > 0f)
    }
    val worldScale: Float = logicalScreenWidth / worldWidth
    val visibleWorldHeight: Float = logicalScreenHeight / worldScale
    val topWorld: Float = (authoredHoleHeight - visibleWorldHeight) / 2f
    val bottomWorld: Float = topWorld + visibleWorldHeight

    fun screenX(worldX: Float): Float = worldX * worldScale
    fun screenY(worldY: Float): Float = (worldY - topWorld) * worldScale
    fun worldX(screenX: Float): Float = screenX / worldScale
    fun worldY(screenY: Float): Float = screenY / worldScale + topWorld
    fun renderedModelHeight(worldHeight: Float): Float = worldHeight * worldScale
}
