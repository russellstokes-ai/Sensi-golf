package com.pixtee.golf

data class BirdFlyover(val x: Float, val y: Float,
                       val movingRight: Boolean, val wingsUp: Boolean)

/**
 * Environment animation timings are deterministic and independent of frame
 * rate. Absent approved sprites render NOTHING; there are no default glyphs.
 */
object CourseAmbient {
    const val FRAMES = 8
    fun frame(elapsedMs: Long, enabled: Boolean): Int =
        if (enabled) ((elapsedMs.coerceAtLeast(0L) / 160L) % FRAMES).toInt() else 0
    fun grassSway(seed: Int, frame: Int): Int =
        when ((seed + frame) % FRAMES) { 1, 2 -> 1; 5, 6 -> -1; else -> 0 }
    fun waterRipple(row: Int, frame: Int): Int = (row * 5 + frame * 2) % 16
    fun spectatorWave(seed: Int, frame: Int): Boolean =
        ((seed * 3 + frame) % FRAMES) in 2..3

    /** Approx 100ms burst once per 18s, for exactly one crowd photographer. */
    fun photographerFlash(elapsedMs: Long, seed: Int, enabled: Boolean): Boolean {
        if (!enabled || elapsedMs < 0L) return false
        val interval=18_000L
        val offset=((seed.toLong()*991L) % interval+interval)%interval
        return (elapsedMs+offset)%interval < 120L
    }

    /** Occasional small flyover, not a bird spawned every render frame. */
    fun birdFlyover(elapsedMs: Long, seed: Long,
                    enabled: Boolean): BirdFlyover? {
        if(!enabled || elapsedMs < 0L) return null
        val interval=23_000L
        val offset=((seed%interval)+interval)%interval
        val cycle=(elapsedMs+offset)/interval
        val local=(elapsedMs+offset)%interval
        if(local>=4_500L) return null
        val travel=local/4_500f
        val right=(cycle%2L)==0L
        val x=if(right) -22f+344f*travel else 322f-344f*travel
        val y=74f+((seed xor (cycle*37L)).ushr(1)%90L).toFloat()+
            (travel-.5f)*14f
        return BirdFlyover(x,y,right,((local/140L)%2L)==0L)
    }
}
