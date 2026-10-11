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

    @Test fun photographerFlashesAreRareBriefAndMotionAware() {
        for(seed in listOf(3,12,21,30)) {
            val frequency=(0L..180_000L step 20L).count {
                CourseAmbient.photographerFlash(it,seed,true)
            }
            // 10 cycles, 120ms each = 1.2s over three minutes.
            assertTrue("Photographer flashing excessively: $frequency",
                frequency in 44..75)
            assertFalse(CourseAmbient.photographerFlash(20000L,seed,false))
            assertEquals(
                CourseAmbient.photographerFlash(200L,seed,true),
                CourseAmbient.photographerFlash(18_200L,seed,true))
        }
    }

    @Test fun birdsCrossTheCourseOnlyOccasionallyAndNeverInReducedMotion() {
        val seed=7919L
        var flying=0
        for(t in 0L..92_000L step 100L) {
            val bird=CourseAmbient.birdFlyover(t,seed,true)
            if(bird!=null) {
                flying++
                assertTrue(bird.x in -22f..322f)
                assertTrue(bird.y in 45f..200f)
            }
            assertNull(CourseAmbient.birdFlyover(t,seed,false))
        }
        assertTrue("Birds must not fill the course constantly",
            flying in 140..230)
        assertEquals(CourseAmbient.birdFlyover(2_000L,seed,true),
            CourseAmbient.birdFlyover(2_000L,seed,true))
    }
}
