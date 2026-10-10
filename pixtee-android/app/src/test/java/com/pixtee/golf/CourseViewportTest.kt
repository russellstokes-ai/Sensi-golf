package com.pixtee.golf

import org.junit.Assert.*
import org.junit.Test

class CourseViewportTest {
    @Test fun tallPhoneShowsMoreVerticalWorldInsteadOfStretchingGolfers() {
        val compact = CourseViewport(360f, 720f)
        val tall = CourseViewport(360f, 800f)
        assertEquals(compact.worldScale, tall.worldScale, 0.00001f)
        assertEquals(compact.renderedModelHeight(14f),
            tall.renderedModelHeight(14f), 0.00001f)
        assertTrue(tall.visibleWorldHeight > compact.visibleWorldHeight)
        assertEquals(360f, compact.screenX(PixteeCore.WIDTH), 0.0001f)
    }

    @Test fun worldScreenTransformIsReversibleAtTeeAndPin() {
        for (screenH in listOf(720f, 760f, 800f, 920f)) {
            val v = CourseViewport(360f, screenH)
            for ((x, y) in listOf(PixteeCore.TEE_X to PixteeCore.TEE_Y,
                PixteeCore.PIN_X to PixteeCore.PIN_Y)) {
                assertEquals(x, v.worldX(v.screenX(x)), 0.0001f)
                assertEquals(y, v.worldY(v.screenY(y)), 0.0001f)
            }
        }
    }

    @Test fun newTallPhoneDoesNotChangeHoleLengthOrCollisionCoordinates() {
        val g = PixteeCore()
        val before = g.toPin
        val compact = CourseViewport(360f, 720f)
        val tall = CourseViewport(360f, 800f)
        assertTrue(tall.visibleWorldHeight > compact.visibleWorldHeight)
        assertEquals(before, g.toPin, 0.0001f)
        assertEquals(Ground.GREEN, g.groundAt(PixteeCore.PIN_X, PixteeCore.PIN_Y))
        assertEquals(Ground.TEE, g.groundAt(PixteeCore.TEE_X, PixteeCore.TEE_Y))
    }
}
