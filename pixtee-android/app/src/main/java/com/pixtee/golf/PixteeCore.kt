package com.pixtee.golf

import kotlin.math.PI
import kotlin.math.abs
import kotlin.math.cos
import kotlin.math.max
import kotlin.math.min
import kotlin.math.sin
import kotlin.math.sqrt

/** Independent deterministic Pixtee mechanics. Never import original commercial code/data. */
enum class GameStage { READY, POWER, ACCURACY, FLIGHT, ROLL, HOLED }
enum class Ground { TEE, FAIRWAY, ROUGH, SAND, GREEN, WATER }
data class Club(val label: String, val yards: Float, val loft: Float)
data class ShotRecord(
    val club: String,
    val power: Float,
    val accuracy: Float,
    val fromX: Float,
    val fromY: Float,
    val toX: Float,
    val toY: Float,
    val penalty: Int,
    val lie: Ground
)

/** World coordinates are abstract playable yards; painted pixels are not collision data. */
class PixteeCore {
    companion object {
        const val WIDTH = 300f
        const val HEIGHT = 510f
        const val PIN_X = 145f
        const val PIN_Y = 67f
        const val TEE_X = 151f
        const val TEE_Y = 459f
        val CLUBS = listOf(
            Club("Driver", 275f, 22f), Club("3 Wood", 235f, 24f),
            Club("5 Wood", 210f, 28f), Club("3 Iron", 185f, 30f),
            Club("4 Iron", 174f, 32f), Club("5 Iron", 164f, 33f),
            Club("6 Iron", 152f, 35f), Club("7 Iron", 140f, 37f),
            Club("8 Iron", 127f, 40f), Club("9 Iron", 111f, 42f),
            Club("Pitching Wedge", 92f, 47f), Club("Sand Wedge", 67f, 53f),
            Club("Putter", 32f, 0f)
        )
    }

    var x = TEE_X; private set
    var y = TEE_Y; private set
    var height = 0f; private set
    var stage = GameStage.READY; private set
    var meter = 0f; private set
    var chosenPower = 0f; private set
    var accuracy = 0.5f; private set
    var aimDegrees = 0f; private set
    var strokes = 0; private set
    var penalties = 0; private set
    var clubIndex = 0; private set
    var lastLie = Ground.TEE; private set
    val shots = mutableListOf<ShotRecord>()
    var practice = false
    var holeNumber = 1; private set
    var par = 4; private set
    private var meterDirection = 1f
    private var startX = TEE_X; private var startY = TEE_Y
    private var destX = TEE_X; private var destY = TEE_Y
    private var shotTime = 0f
    private var shotDuration = 1f
    private var rollTime = 0f
    private var previousX = TEE_X
    private var previousY = TEE_Y

    val toPin: Float get() = hypot(x - PIN_X, y - PIN_Y)
    val club: Club get() = CLUBS[clubIndex]
    val scoreRelative: Int get() = strokes - par

    fun restart() {
        x = TEE_X; y = TEE_Y; height = 0f; stage = GameStage.READY
        meter = 0f; chosenPower = 0f; accuracy = 0.5f
        aimDegrees = 0f; strokes = 0; penalties = 0; clubIndex = 0
        lastLie = Ground.TEE; shots.clear(); holeNumber = 1
        meterDirection = 1f
    }

    fun steer(degrees: Float) {
        if (stage == GameStage.READY) aimDegrees = (aimDegrees + degrees).coerceIn(-85f, 85f)
    }
    fun changeClub(step: Int) {
        if (stage == GameStage.READY) clubIndex = (clubIndex + step).coerceIn(0, CLUBS.lastIndex)
    }

    /** Each touch release advances EXACTLY one stage of the classic three-click swing. */
    fun whack(): GameStage {
        when (stage) {
            GameStage.READY -> {
                meter = 0f; meterDirection = 1f
                stage = GameStage.POWER
            }
            GameStage.POWER -> {
                chosenPower = meter.coerceIn(0.05f, 1f)
                meter = 0f; meterDirection = 1f
                stage = GameStage.ACCURACY
            }
            GameStage.ACCURACY -> {
                accuracy = meter.coerceIn(0f, 1f)
                commitShot()
            }
            else -> Unit
        }
        return stage
    }

    /** Fixed 60Hz gameplay clock, independent of Android render frames. */
    fun tick() = when (stage) {
        GameStage.POWER, GameStage.ACCURACY -> {
            val step = if (stage == GameStage.POWER) 1.14f / 60f else 1.55f / 60f
            meter += meterDirection * step
            if (meter >= 1f) { meter = 1f; meterDirection = -1f }
            if (meter <= 0f) { meter = 0f; meterDirection = 1f }
        }
        GameStage.FLIGHT -> {
            shotTime += 1f / 60f
            val t = min(1f, shotTime / shotDuration)
            x = startX + (destX - startX) * t
            y = startY + (destY - startY) * t
            height = (sin(t * PI).toFloat() * club.loft * chosenPower).coerceAtLeast(0f)
            if (t >= 1f) { stage = GameStage.ROLL; rollTime = 0f; height = 0f }
        }
        GameStage.ROLL -> {
            rollTime += 1f / 60f
            if (rollTime >= 0.38f) finishShot()
        }
        else -> Unit
    }

    private fun commitShot() {
        previousX = x; previousY = y
        startX = x; startY = y
        // The source of all flight parameters is this independent Pixtee tuning model.
        // No reverse-engineered copyrighted physics tables or numerical trace banks.
        val miss = abs(accuracy - 0.5f) * 2f
        val lieModifier = when (lastLie) {
            Ground.SAND -> 0.63f
            Ground.ROUGH -> 0.81f
            else -> 1f
        }
        val intended = club.yards * chosenPower * (1f - 0.23f * miss) * lieModifier
        val angle = Math.toRadians((aimDegrees + (accuracy - 0.5f) * 16f).toDouble())
        destX = (x + sin(angle).toFloat() * intended).coerceIn(5f, WIDTH - 5f)
        destY = (y - cos(angle).toFloat() * intended).coerceIn(5f, HEIGHT - 5f)
        shotTime = 0f
        shotDuration = if (club.label == "Putter") 0.6f else (0.9f + intended / 180f)
        strokes++
        stage = GameStage.FLIGHT
    }

    private fun finishShot() {
        val landing = groundAt(x, y)
        if (landing == Ground.WATER) {
            x = previousX; y = previousY; penalties++; strokes++; lastLie = groundAt(x, y)
        } else {
            lastLie = landing
        }
        shots.add(ShotRecord(club.label, chosenPower, accuracy, startX, startY, x, y,
            if (landing == Ground.WATER) 1 else 0, lastLie))
        if (toPin < 5f) {
            x = PIN_X; y = PIN_Y; stage = GameStage.HOLED
        } else {
            stage = GameStage.READY
            if (toPin < 35f) clubIndex = CLUBS.lastIndex
        }
    }

    fun groundAt(tx: Float, ty: Float): Ground {
        val green = hypot(tx - PIN_X, (ty - PIN_Y) * 1.12f)
        if (green <= 33f) return Ground.GREEN
        if (tx > 231f && ty in 185f..302f) return Ground.WATER
        if ((tx in 73f..104f && ty in 104f..158f) ||
            (tx in 208f..240f && ty in 122f..174f)) return Ground.SAND
        val centre = 148f + sin(ty / 79f) * 28f
        if (abs(tx - centre) < 43f && ty in 72f..461f) return Ground.FAIRWAY
        if (hypot(tx - TEE_X, ty - TEE_Y) <= 16f) return Ground.TEE
        return Ground.ROUGH
    }

    private fun hypot(dx: Float, dy: Float): Float = sqrt(dx * dx + dy * dy)
}
