package com.pixtee.golf

/**
 * Full-hole schematic projection derived from the same immutable layout
 * that powers the game's collision logic. No illustrated course, fake
 * hazards, placeholder sprites, or imported original-game graphics.
 */
class CourseMiniMap(
    val left: Float = 321f,
    val top: Float = 40f,
    val width: Float = 31f,
    val height: Float = 109f
) {
    init { require(width>0f && height>0f) }

    fun sx(x: Float): Float = left+x/PixteeCore.WIDTH*width
    fun sy(y: Float): Float = top+y/PixteeCore.HEIGHT*height
    fun originalX(screenX: Float): Float = (screenX-left)*PixteeCore.WIDTH/width
    fun originalY(screenY: Float): Float = (screenY-top)*PixteeCore.HEIGHT/height

    fun green(layout: HoleLayout): Pair<Float,Float> =
        sx(layout.pinX) to sy(layout.pinY)

    fun tee(layout: HoleLayout): Pair<Float,Float> =
        sx(layout.teeX) to sy(layout.teeY)

    fun centreline(layout: HoleLayout, samples: Int = 36):
        List<Pair<Float,Float>> {
        require(samples in 2..128)
        return (0 until samples).map { n ->
            val y=layout.pinY+(layout.teeY-layout.pinY)*n/(samples-1f)
            sx(layout.fairwayCentre(y)) to sy(y)
        }
    }

    fun waterPatches(layout: HoleLayout):
        List<List<Pair<Float,Float>>> = layout.waters.map { patch ->
        listOf(sx(patch.l) to sy(patch.t),sx(patch.r) to sy(patch.b))
    }
}
