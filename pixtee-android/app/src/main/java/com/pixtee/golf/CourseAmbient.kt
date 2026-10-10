package com.pixtee.golf

object CourseAmbient {
    const val FRAMES = 8
    fun frame(elapsedMs: Long, enabled: Boolean): Int =
        if (enabled) ((elapsedMs.coerceAtLeast(0L) / 160L) % FRAMES).toInt() else 0
    fun grassSway(seed: Int, frame: Int): Int =
        when ((seed + frame) % FRAMES) { 1, 2 -> 1; 5, 6 -> -1; else -> 0 }
    fun waterRipple(row: Int, frame: Int): Int = (row * 5 + frame * 2) % 16
    fun spectatorWave(seed: Int, frame: Int): Boolean =
        ((seed * 3 + frame) % FRAMES) in 2..3
}
