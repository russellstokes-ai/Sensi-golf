package com.pixtee.golf

import kotlin.math.abs

/**
 * Separates Canvas drawing coordinates from touch gestures.
 * A drag is never reported as a tap; y is restored to the scrolled 760-unit
 * menu canvas so small-screen/fold navigation uses the SAME hit targets.
 */
class MenuInput {
    var scrollY = 0f
        private set
    private var downX = 0f
    private var downY = 0f
    private var previousY = 0f
    private var dragging = false
    private var pressed = false

    fun down(x: Float, y: Float) {
        downX = x
        downY = y
        previousY = y
        dragging = false
        pressed = true
    }

    fun move(x: Float, y: Float, maxScroll: Float): Boolean {
        if (!pressed) return false
        if (!dragging && (abs(y - downY) > 9f || abs(x - downX) > 15f)) {
            dragging = true
        }
        if (dragging) {
            scrollY = (scrollY + previousY - y).coerceIn(0f, maxScroll.coerceAtLeast(0f))
        }
        previousY = y
        return dragging
    }

    /** Returns an unscrolled logical menu coordinate only for a true tap. */
    fun release(x: Float, y: Float): Pair<Float, Float>? {
        if (!pressed) return null
        pressed = false
        if (dragging || abs(y - downY) > 9f || abs(x - downX) > 15f) return null
        return x to (y + scrollY)
    }

    fun reset() {
        scrollY = 0f
        pressed = false
        dragging = false
    }

    fun cancel() { pressed = false; dragging = false }

    companion object {
        const val CONTENT_HEIGHT = 760f
        fun maxScroll(viewHeight: Float): Float = (CONTENT_HEIGHT - viewHeight).coerceAtLeast(0f)
        fun contains(x: Float, y: Float, l: Float, t: Float, r: Float, b: Float): Boolean =
            x >= l && x <= r && y >= t && y <= b
    }
}
