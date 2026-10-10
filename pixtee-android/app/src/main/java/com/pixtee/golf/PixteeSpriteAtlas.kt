package com.pixtee.golf

/**
 * First-pass ORIGINAL pixel sprite sheet expressed as portable glyph cells.
 *
 * A sprite is a tiny fixed grid, rendered with nearest-neighbour integer-size
 * pixels. This class uses no Android graphics APIs so its authored frames,
 * dimensions, transparency and palettes can be unit-tested on the JVM.
 *
 * ALL ART IS PROVISIONAL until publisher explicitly approves the real-scale
 * course mockups. Never incorporate any original Sensible Golf sprites.
 */
class PixelSprite(
    val id: String,
    val width: Int,
    val rows: List<String>
) {
    val height: Int = rows.size
    init {
        require(id.isNotBlank() && width in 1..64 && height in 1..64)
        require(rows.all { it.length == width }) { "Pixel row size differs: $id" }
        require(rows.flatMap { it.toList() }.all { it in ".HSTPKCBAOdmlwusfx" }) {
            "Unrecognised colour token in $id"
        }
    }

    fun token(x: Int, y: Int): Char {
        require(x in 0 until width && y in 0 until height)
        return rows[y][x]
    }

    val opaqueCells: Int get() = rows.sumOf { row -> row.count { it != '.' } }
}

/**
 * Content addresses are stable, no dynamic allocations or atlas randomness
 * during a render frame; motion changes which frame is selected, not physics.
 */
object PixteeSpriteAtlas {
    private fun frame(id: String, width: Int, vararg rows: String): PixelSprite =
        PixelSprite(id, width, rows.map { line ->
            require(line.length <= width) { "Overwide pixel art row in $id" }
            line.padEnd(width, '.')
        })

    // 16x18 top-down pixel golfers. Feet stay pinned to the ball's world point.
    val golferIdle = frame("golfer-idle-v1", 16,
        "......HHHH", ".....HHHHHH", ".....OSSSSO", "......SSSS",
        ".....OTTTTO", ".....OTTTTO", "...A.OTTTTO", "...A.OTTTTO",
        "...A..OTTO...CC", "..BB..TTT...CC", "..BB..TTT...CC",
        "......PPPP", "......P..P", "......P..P", ".....PP..PP",
        ".....KK..KK", ".....KK..KK", "................")
    val golferBackswing = frame("golfer-backswing-v1", 16,
        "......HHHH", ".....HHHHHH", ".....OSSSSO", "......SSSS",
        ".....OTTTTO", ".....OTTTTOCC", "...A.OTTTCC", "...A.OTCC",
        "...A..OCC", "..BB..CTT", "..BB.CCTT",
        "......PPPP", "......P..P", "......P..P", ".....PP..PP",
        ".....KK..KK", ".....KK..KK", "................")
    val golferImpact = frame("golfer-impact-v1", 16,
        "......HHHH", ".....HHHHHH", ".....OSSSSO", "......SSSS",
        ".....OTTTTO", ".....OTTTTO", "...A.OTTTTO", "...A.OTTTTO",
        "...A..OTTOCC", "..BB..TTT.CCC", "..BB..TTTCC",
        "......PPPP", "......P..P", "......P..P", ".....PP..PP",
        ".....KK..KK", ".....KK..KK", "................")
    val golferFollow = frame("golfer-follow-v1", 16,
        ".....C.HHHH", "....CC.HHHHHH", "...CC.OSSSSO", "...C...SSSS",
        "...C.OTTTTO", ".....OTTTTO", "...A.OTTTTO", "...A.OTTTTO",
        "...A..OTTO", "..BB..TTT", "..BB..TTT",
        "......PPPP", "......P..P", "......P..P", ".....PP..PP",
        ".....KK..KK", ".....KK..KK", "................")

    val treeRound = frame("tree-round-v1", 20,
        "........dddd", "......ddmmmmdd", ".....dmmmmmmmd",
        "...ddmmmmmllmmdd", "..dmmmmmmmlllmd",
        ".dmmmmmmmmllllmd", "dmmmmmmmmlllmmmmd",
        "dmmmmmmmmmllmmmmd", ".dmmmmmmmmmmmmd",
        "..dmmmmmmmmmmmd", "....ddddmmddd",
        ".........ww", ".........ww", "........wwww",
        "........wwww")
    val treePine = frame("tree-pine-v1", 20,
        ".........d", "........dmd", ".......dmmd",
        "......dmmllmd", ".....dmmlmmmd",
        ".......dmmd", "......dmmlmd", ".....dmmllmmd",
        "....dmmmmllmmd", "......dmmmmd",
        ".....dmmmmllmd", "...dmmmmmmllmmd",
        "..dmmmmmmmllmmd", ".........ww",
        ".........ww", "........wwww", "........wwww")
    val spectatorIdle = frame("spectator-idle-v1", 10,
        "....OO", "...OOOO", "...SSSS", "....SS",
        "...uusu", "...uuuu", "...uuuu", "...uuuu",
        "...PPPP", "...P..P", "...P..P", "...K..K")
    val spectatorWave = frame("spectator-wave-v1", 10,
        "....OO..S", "...OOOO.SS", "...SSSS.S", "....SS.S",
        "...uusSS", "...uuuu", "...uuuu", "...uuuu",
        "...PPPP", "...P..P", "...P..P", "...K..K")
    val flowerA = frame("flower-yellow-v1", 7,
        "...f", "..fff", "...f", "...w", "..www")
    val flowerB = frame("flower-pink-v1", 7,
        "...x", "..xxx", "...x", "...w", "..www")

    val all: List<PixelSprite> = listOf(
        golferIdle, golferBackswing, golferImpact, golferFollow,
        treeRound, treePine, spectatorIdle, spectatorWave, flowerA, flowerB
    )

    fun golfer(stage: GameStage): PixelSprite = when(stage) {
        GameStage.POWER -> golferBackswing
        GameStage.ACCURACY -> golferImpact
        GameStage.FLIGHT -> golferFollow
        else -> golferIdle
    }
    fun tree(variant: Int): PixelSprite =
        if(variant % 3 == 0) treePine else treeRound
    fun spectator(waving: Boolean): PixelSprite =
        if(waving) spectatorWave else spectatorIdle
}
