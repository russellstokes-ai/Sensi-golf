package com.pixtee.golf

import org.junit.Assert.*
import org.junit.Test
import kotlin.math.sqrt

/**
 * The first original playable hole is authored, not just another seeded
 * fairway rectangle. Collision and the render mask share its river polygon.
 * These tests verify actual geometry, not yet the unapproved visuals.
 */
class PixteeSignatureHoleTest {
    private val hole = PixteeCourseCatalog.hole(0,1)

    @Test fun signatureHoleHasPlayableParFourLengthAndDistinctHazards() {
        assertEquals(4,hole.par)
        val length=sqrt(
            (hole.teeX-hole.pinX)*(hole.teeX-hole.pinX)+
            (hole.teeY-hole.pinY)*(hole.teeY-hole.pinY)
        )
        assertTrue("Signature par four must be about 376 yards",length in 375f..378f)
        assertEquals(Ground.TEE,hole.groundAt(hole.teeX,hole.teeY))
        assertEquals(Ground.GREEN,hole.groundAt(hole.pinX,hole.pinY))
        assertEquals(Ground.FAIRWAY,hole.groundAt(
            hole.fairwayCentre(245f),245f))
        assertEquals(3,hole.bunkers.size)
        assertEquals(Ground.SAND,hole.groundAt(91f,141f))
        assertEquals(Ground.SAND,hole.groundAt(200f,289f))
    }

    @Test fun waterUsesWindingPolygonRatherThanInvisibleRectangle() {
        assertEquals(1,hole.waters.size)
        val creek=hole.waters.single()
        assertTrue(creek.outline.size>=10)
        assertTrue(creek.contains(280f,300f))
        assertEquals(Ground.WATER,hole.groundAt(280f,300f))
        assertFalse("Dry rough left of the water must not be penalised",
            creek.contains(220f,300f))
        assertNotEquals(Ground.WATER,hole.groundAt(220f,300f))
        assertFalse("Same bounding box but outside winding bank",
            creek.contains(225f,165f))
    }

    @Test fun authoredHoleIsStableAndOtherHolesRemainSeeded() {
        val reread=PixteeCourseCatalog.hole(0,1)
        assertEquals(hole.seed,reread.seed)
        assertEquals(hole.waters,reread.waters)
        val another=PixteeCourseCatalog.hole(0,2)
        assertEquals(2,another.number)
        assertNotEquals(hole.seed,another.seed)
        assertTrue(PixteeCourseCatalog.allHoleCount()>=630)
    }
}
