package com.pixtee.golf

import org.junit.Assert.*
import org.junit.Test

class CourseAmbientTest {
    @Test fun optionalMotionCanBeFrozen() {
        for (t in listOf(0L, 160L, 1600L, 987654L)) {
            assertEquals(0, CourseAmbient.frame(t, false))
        }
    }

    @Test fun loopsSeamlesslyAndDoesNotDependOnRenderFrequency() {
        for (t in listOf(0L, 200L, 818L, 2044L, 98333L)) {
            assertEquals(CourseAmbient.frame(t, true),
                CourseAmbient.frame(t + 160L * CourseAmbient.FRAMES, true))
        }
    }

    @Test fun windAndWaveAreBoundedAndDeterministic() {
        for (frame in 0 until 24) {
            assertTrue(CourseAmbient.grassSway(17, frame) in -1..1)
            assertTrue(CourseAmbient.waterRipple(4, frame) in 0..15)
            assertEquals(CourseAmbient.waterRipple(4, frame),
                CourseAmbient.waterRipple(4, frame + 8))
        }
    }
}
