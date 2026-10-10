package com.pixtee.golf

import kotlin.math.PI
import kotlin.math.abs
import kotlin.math.cos
import kotlin.math.sin
import kotlin.math.sqrt
import java.util.Random

/**
 * Original, reproducible course geometry, authored specifically for Pixtee.
 * The catalog has 25 independent themes with 18 holes each. Layouts are
 * deterministic combinations of a per-course seed and individual hole design.
 * They do NOT copy course coordinates, bitmap tiles, names or hazard placements
 * from any existing golf game.
 */
data class PixteeCourse(val id: String, val title: String, val theme: Int)
data class WaterPatch(val l: Float, val t: Float, val r: Float, val b: Float) {
    fun contains(x: Float, y: Float) = x >= l && x <= r && y >= t && y <= b
}
data class SandPatch(val x: Float, val y: Float, val rx: Float, val ry: Float) {
    fun contains(px: Float, py: Float): Boolean {
        val dx = (px - x) / rx
        val dy = (py - y) / ry
        return dx * dx + dy * dy < 1f
    }
}
data class SceneryPoint(val x: Float, val y: Float, val variant: Int)

class HoleLayout(
    val course: PixteeCourse,
    val number: Int,
    val par: Int,
    val teeX: Float,
    val teeY: Float,
    val pinX: Float,
    val pinY: Float,
    val bend: Float,
    val width: Float,
    val waters: List<WaterPatch>,
    val bunkers: List<SandPatch>,
    val seed: Long
) {
    init {
        require(number in 1..18)
        require(par in 3..5)
        require(teeX in 30f..270f && pinX in 30f..270f)
    }
    val greenRadius = 30f

    fun fairwayCentre(y: Float): Float {
        val travel = ((teeY - y) / (teeY - pinY)).coerceIn(0f, 1f)
        return teeX + (pinX - teeX) * travel +
            sin((travel * PI).toFloat()) * bend
    }

    fun groundAt(x: Float, y: Float): Ground {
        if (x !in 0f..PixteeCore.WIDTH || y !in 0f..PixteeCore.HEIGHT)
            return Ground.ROUGH
        val gx = (x - pinX) / greenRadius
        val gy = (y - pinY) / (greenRadius * .82f)
        if (gx * gx + gy * gy < 1f) return Ground.GREEN
        if ((x-teeX)*(x-teeX) + (y-teeY)*(y-teeY) <= 14f*14f)
            return Ground.TEE
        if (waters.any { it.contains(x, y) }) return Ground.WATER
        if (bunkers.any { it.contains(x, y) }) return Ground.SAND
        if (y >= pinY && y <= teeY + 5f &&
            abs(x - fairwayCentre(y)) < width * .5f) return Ground.FAIRWAY
        return Ground.ROUGH
    }

    /** Stable world-space background tree marks; NEVER generated per frame. */
    val trees: List<SceneryPoint> by lazy {
        val r = Random(seed xor 0x2AFE44L)
        buildList {
            repeat(105) {
                val x = 8f + r.nextFloat() * 284f
                val y = 15f + r.nextFloat() * 479f
                if (groundAt(x,y) != Ground.ROUGH) return@repeat
                if ((x-pinX)*(x-pinX)+(y-pinY)*(y-pinY) < 1800f) return@repeat
                if ((x-teeX)*(x-teeX)+(y-teeY)*(y-teeY) < 1300f) return@repeat
                add(SceneryPoint(x,y,r.nextInt(4)))
            }
        }
    }

    val spectators: List<SceneryPoint> by lazy {
        val r = Random(seed xor 0x3ABF20L)
        buildList {
            repeat(18) { i ->
                val y = 45f + i * 24f + r.nextFloat()*7f
                val x = if(i % 2 == 0) 14f+r.nextFloat()*26f
                        else 260f+r.nextFloat()*24f
                if (groundAt(x,y)==Ground.ROUGH) add(SceneryPoint(x,y,i % 4))
            }
        }
    }
}

object PixteeCourseCatalog {
    const val HOLES_PER_COURSE = 18
    val courses: List<PixteeCourse> = listOf(
        "Lakewood", "Riverdale", "Oak Valley", "Sunridge", "Pine Crest",
        "Coastline", "Mossy Glen", "Silver Dunes", "Redwood Park", "Emerald Bay",
        "Copper Ridge", "High Mesa", "Willow Marsh", "Blue Lagoon", "Misty Moor",
        "Desert Bloom", "Cherry Hills", "Pebble Cove", "Frostwood", "Golden Sands",
        "Thornfield", "Moonstone", "Summit Lakes", "Crimson Cliffs", "Glacier Point"
    ).mapIndexed { i, name ->
        PixteeCourse(name.lowercase().replace(" ", "-"), name, i % 5)
    }

    /** Every hole has different tee, green, path and hazard geometry. */
    fun hole(courseIndex: Int, holeNumber: Int): HoleLayout {
        require(courseIndex in courses.indices)
        require(holeNumber in 1..HOLES_PER_COURSE)
        val c = courses[courseIndex]
        val seed = 8111L + courseIndex * 104729L + holeNumber * 7919L
        val rng = Random(seed)
        val par = when ((holeNumber + courseIndex * 3) % 6) {
            0, 3 -> 3
            2 -> 5
            else -> 4
        }
        val teeX = 113f + rng.nextFloat()*76f
        val pinX = 105f + rng.nextFloat()*90f
        val teeY = when(par) {
            3 -> 245f + rng.nextFloat() * 45f
            5 -> 453f + rng.nextFloat() * 24f
            else -> 392f + rng.nextFloat() * 54f
        }
        val pinY = 53f + rng.nextFloat() * 34f
        val bend = (rng.nextFloat()-.5f) * 75f
        val fairwayWidth = 77f + rng.nextFloat()*22f
        val dangerLeft = ((courseIndex + holeNumber) % 2) == 0
        val streamY = (pinY + teeY)*.5f + (rng.nextFloat()-.5f)*45f
        val waters = if ((holeNumber+courseIndex) % 4 != 2) {
            val l = if (dangerLeft) 9f else 229f
            listOf(WaterPatch(l, streamY-43f, l+62f, streamY+43f))
        } else emptyList()
        val bunkers = listOf(
            SandPatch(pinX - 42f, pinY + 38f, 18f, 19f),
            SandPatch(pinX + 42f, pinY + 12f, 17f, 21f)
        )
        return HoleLayout(c,holeNumber,par,teeX,teeY,pinX,pinY,
            bend,fairwayWidth,waters,bunkers,seed)
    }

    fun allHoleCount(): Int = courses.size * HOLES_PER_COURSE
}

/**
 * Authoritative playable round state. Only recorded scores advance the round;
 * no fake scorecard pages and no per-hole replacement of a whole round.
 */
data class HoleScore(val hole: Int, val par: Int, val strokes: Int, val penalties: Int,
                     val putts: Int, val length: Float) {
    val relative: Int get() = strokes-par
}
enum class RoundMode { PRACTICE, QUICK, CAREER, TOURNAMENT }

class PixteeRound(val courseIndex: Int, val length: Int, val mode: RoundMode) {
    init {
        require(courseIndex in PixteeCourseCatalog.courses.indices)
        require(length in listOf(1, 3, 9, 18))
    }
    val results = mutableListOf<HoleScore>()
    val currentHole: Int get() = (results.size+1).coerceAtMost(length)
    val isComplete: Boolean get() = results.size == length
    val totalStrokes: Int get() = results.sumOf { it.strokes }
    val totalPar: Int get() = results.sumOf { it.par }
    val totalPenalties: Int get() = results.sumOf { it.penalties }
    val totalPutts: Int get() = results.sumOf { it.putts }
    val relativeToPar: Int get() = totalStrokes - totalPar
    fun layout(): HoleLayout =
        PixteeCourseCatalog.hole(courseIndex, currentHole)

    fun record(strokes: Int, penalties: Int, putts: Int): HoleScore {
        check(!isComplete) { "Cannot score beyond the final hole" }
        require(strokes >= 1 && penalties >= 0 && putts >= 0 && putts <= strokes)
        val hole = layout()
        val dx = hole.teeX-hole.pinX
        val dy = hole.teeY-hole.pinY
        val score = HoleScore(currentHole,hole.par,strokes,penalties,putts,
            sqrt(dx*dx+dy*dy))
        results.add(score)
        return score
    }
}
