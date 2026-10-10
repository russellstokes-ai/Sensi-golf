package com.pixtee.golf

import kotlin.math.PI
import kotlin.math.abs
import kotlin.math.cos
import kotlin.math.max
import kotlin.math.min
import kotlin.math.sin
import kotlin.math.pow
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
        /** Classic-reference logical cadence (~70.017Hz); approximate Pixtee implementation. */
        const val TICKS_PER_SECOND = 70f
        const val TICK_SECONDS = 1f / TICKS_PER_SECOND
        const val PIN_X = 145f
        const val PIN_Y = 67f
        const val TEE_X = 151f
        const val TEE_Y = 459f
        /** Independent swing strength: the TOP of the circular dial is 100%, not the bottom. */
        fun powerForMeter(position: Float): Float =
            sin(PI * position.coerceIn(0f, 1f).toDouble()).toFloat().coerceIn(0f, 1f)

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

    var activeHole: HoleLayout? = null; private set
    var putts = 0; private set
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
    private var flightCurveX = 0f
    private var flightCurveY = 0f
    private var rollTime = 0f
    private var rollVx = 0f
    private var rollVy = 0f
    private var previousX = TEE_X
    private var previousY = TEE_Y

    val toPin: Float get() = hypot(x - (activeHole?.pinX ?: PIN_X),
        y - (activeHole?.pinY ?: PIN_Y))
    /**
     * The golfer stands at the last address point throughout the ball's flight
     * and roll. The ball moves independently. This also supplies an exact
     * frame-zero contact anchor for the approved animation rig.
     */
    val golferWorldX: Float get() = if(stage==GameStage.FLIGHT ||
        stage==GameStage.ROLL) startX else x
    val golferWorldY: Float get() = if(stage==GameStage.FLIGHT ||
        stage==GameStage.ROLL) startY else y

    val club: Club get() = CLUBS[clubIndex]
    val scoreRelative: Int get() = strokes - par

    fun restart() {
        x = activeHole?.teeX ?: TEE_X; y = activeHole?.teeY ?: TEE_Y
        height = 0f; stage = GameStage.READY
        meter = 0f; chosenPower = 0f; accuracy = 0.5f
        aimDegrees = 0f; strokes = 0; penalties = 0; clubIndex = 0
        lastLie = Ground.TEE; shots.clear(); holeNumber = activeHole?.number ?: 1
        par = activeHole?.par ?: 4; putts = 0
        meterDirection = 1f
        rollTime = 0f; rollVx = 0f; rollVy = 0f
        flightCurveX = 0f; flightCurveY = 0f
    }

    /** Begin a distinct playable hole without losing the enclosing round. */
    fun startHole(hole: HoleLayout) {
        activeHole = hole
        restart()
    }

    fun stableBall(): StableBall? =
        if (stage != GameStage.READY) null
        else StableBall(x,y,strokes,penalties,putts,clubIndex,aimDegrees)

    fun restoreBall(state: StableBall) {
        require(activeHole != null) { "Load the authored hole before restoring a ball" }
        x=state.x; y=state.y; strokes=state.strokes
        penalties=state.penalties; putts=state.putts
        clubIndex=state.club; aimDegrees=state.aim
        lastLie=groundAt(x,y)
        stage=GameStage.READY; height=0f
        meter=0f;chosenPower=0f;accuracy=.5f
        rollTime=0f;rollVx=0f;rollVy=0f
    }

    fun steer(degrees: Float) {
        if (stage == GameStage.READY) aimDegrees = (aimDegrees + degrees).coerceIn(-85f, 85f)
    }
    fun changeClub(step: Int) {
        if (stage == GameStage.READY)
            clubIndex = ((clubIndex + step) % CLUBS.size + CLUBS.size) % CLUBS.size
    }

    /** Each touch release advances EXACTLY one stage of the classic three-click swing. */
    fun whack(): GameStage {
        when (stage) {
            GameStage.READY -> {
                meter = 0f; meterDirection = 1f
                stage = GameStage.POWER
            }
            GameStage.POWER -> {
                // A downswing returns along the SAME meter arc; do not restart at zero.
                val cursorPosition = meter.coerceIn(0f, 1f)
                chosenPower = powerForMeter(cursorPosition).coerceIn(0.05f, 1f)
                // Keep the cursor at its physical arc location for downswing.
                meter = cursorPosition
                meterDirection = -1f
                stage = GameStage.ACCURACY
            }
            GameStage.ACCURACY -> {
                // Zero is the straight-shot red zone at the bottom of the arc.
                // Pressing early/late gives the two opposite shot curves.
                accuracy = (0.5f + meter * 1.6f).coerceIn(0f, 1f)
                commitShot()
            }
            else -> Unit
        }
        return stage
    }

    /** Fixed ~70Hz simulation clock, independent of Android rendering cadence. */
    fun tick() {
        when (stage) {
        GameStage.POWER -> {
            meter += meterDirection * (1.14f * TICK_SECONDS)
            if (meter >= 1f) { meter = 1f; meterDirection = -1f }
            if (meter <= 0f) { meter = 0f; meterDirection = 1f }
        }
        GameStage.ACCURACY -> {
            // Stronger swings cross the narrow accuracy zone faster.
            meter -= (1.1f + chosenPower * 0.95f) * TICK_SECONDS
            if (meter <= -0.32f) {
                meter = -0.32f
                accuracy = 0f
                commitShot() // missed the accuracy window entirely
            }
        }
        GameStage.FLIGHT -> {
            shotTime += TICK_SECONDS
            val t = min(1f, shotTime / shotDuration)
            // Side-spin changes the trajectory during flight, not only the
            // destination. The arc closes at touchdown to prevent teleporting.
            val swing = sin(t * PI).toFloat()
            x = (startX + (destX-startX)*t + flightCurveX*swing)
                .coerceIn(5f, WIDTH-5f)
            y = (startY + (destY-startY)*t + flightCurveY*swing)
                .coerceIn(5f, HEIGHT-5f)
            height = (sin(t * PI).toFloat() * club.loft * chosenPower).coerceAtLeast(0f)
            if (t >= 1f) {
                height = 0f
                // Water at first contact terminates immediately, matching
                // the observed classic hazard behaviour (no extra roll tick).
                if (groundAt(x,y) == Ground.WATER) finishShot()
                else {
                    stage = GameStage.ROLL
                    rollTime = 0f
                }
            }
        }
        GameStage.ROLL -> {
            val dt = TICK_SECONDS
            rollTime += dt
            // Course-specific gentle green break. No wind by default.
            activeHole?.greenRollAcceleration(x,y)?.let { (ax,ay) ->
                rollVx += ax*dt
                rollVy += ay*dt
            }
            val nextX = x + rollVx * dt
            val nextY = y + rollVy * dt
            val boundary = nextX !in 5f..(WIDTH - 5f) ||
                nextY !in 5f..(HEIGHT - 5f)
            x = nextX.coerceIn(5f, WIDTH - 5f)
            y = nextY.coerceIn(5f, HEIGHT - 5f)
            val lie = groundAt(x, y)
            // Preserve approximate seconds-to-rest across the 60 -> 70Hz
            // transition. Original source coefficients are NOT imported.
            val sixtyHzDrag = when (lie) {
                Ground.GREEN -> 0.971f
                Ground.TEE -> 0.923f
                Ground.FAIRWAY -> 0.925f
                Ground.ROUGH -> 0.841f
                Ground.SAND -> 0.712f
                Ground.WATER -> 0f
            }
            val damping = sixtyHzDrag.pow(60f/TICKS_PER_SECOND)
            rollVx *= damping
            rollVy *= damping
            val speedSquared = rollVx * rollVx + rollVy * rollVy
            // One tiny post-landing hop is purely visual; position is ground-based.
            height = if (clubIndex != CLUBS.lastIndex && rollTime < 0.29f)
                sin(rollTime / 0.29f * PI).toFloat().coerceAtLeast(0f) *
                    chosenPower * 3.1f else 0f
            if (lie == Ground.WATER || boundary || speedSquared < 0.12f ||
                rollTime >= 4.5f || toPin <= 2.3f) finishShot()
        }
        else -> Unit
        }
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
        val directionX = sin(angle).toFloat()
        val directionY = -cos(angle).toFloat()
        destX = (x + directionX * intended).coerceIn(5f, WIDTH - 5f)
        destY = (y + directionY * intended).coerceIn(5f, HEIGHT - 5f)
        // Smooth early/late curve with an independently chosen arcade profile.
        val lateral = (accuracy-0.5f) * min(18f, intended*0.10f)
        flightCurveX = -directionY*lateral
        flightCurveY = directionX*lateral
        shotTime = 0f
        // Independently calibrated arcade pacing: a full-power driver now
        // lands after ~80 logical 70Hz steps (~1.14s), rather than ~170.
        // Taller lofts float a little longer. This does not copy source code.
        shotDuration = (0.44f + 0.59f*chosenPower +
            club.loft*0.005f).coerceIn(0.5f,1.45f)
        rollTime = 0f
        val speed = if (clubIndex == CLUBS.lastIndex) intended * 1.75f
            else (5f + intended * 0.08f) * (1f - miss * 0.2f)
        rollVx = directionX * speed
        rollVy = directionY * speed
        strokes++
        if (clubIndex == CLUBS.lastIndex && lastLie == Ground.GREEN) putts++
        // A putt travels along the ground. Woods and irons first fly, then roll.
        if (clubIndex == CLUBS.lastIndex) {
            destX = x; destY = y
            stage = GameStage.ROLL
        } else stage = GameStage.FLIGHT
    }

    private fun finishShot() {
        height = 0f
        rollVx = 0f; rollVy = 0f
        val landing = groundAt(x, y)
        if (landing == Ground.WATER) {
            x = previousX; y = previousY; penalties++; strokes++; lastLie = groundAt(x, y)
        } else {
            lastLie = landing
        }
        shots.add(ShotRecord(club.label, chosenPower, accuracy, startX, startY, x, y,
            if (landing == Ground.WATER) 1 else 0, lastLie))
        if (toPin < 5f || strokes >= 12) {
            if (toPin < 5f) {
                x = activeHole?.pinX ?: PIN_X
                y = activeHole?.pinY ?: PIN_Y
            }
            stage = GameStage.HOLED
        } else {
            stage = GameStage.READY
            if (toPin < 35f) clubIndex = CLUBS.lastIndex
        }
    }

    fun groundAt(tx: Float, ty: Float): Ground {
        activeHole?.let { return it.groundAt(tx,ty) }
        val green = hypot(tx - PIN_X, (ty - PIN_Y) * 1.12f)
        if (green <= 33f) return Ground.GREEN
        // Tee is a distinct collision surface nested inside fairway geometry.
        // Resolve local tee collision BEFORE the wider fairway test.
        if (hypot(tx - TEE_X, ty - TEE_Y) <= 16f) return Ground.TEE
        if (tx > 231f && ty in 185f..302f) return Ground.WATER
        if ((tx in 73f..104f && ty in 104f..158f) ||
            (tx in 208f..240f && ty in 122f..174f)) return Ground.SAND
        val centre = 148f + sin(ty / 79f) * 28f
        if (abs(tx - centre) < 43f && ty in 72f..461f) return Ground.FAIRWAY
        return Ground.ROUGH
    }

    private fun hypot(dx: Float, dy: Float): Float = sqrt(dx * dx + dy * dy)
}
