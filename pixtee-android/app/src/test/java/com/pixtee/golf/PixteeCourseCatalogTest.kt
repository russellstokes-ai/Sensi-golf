package com.pixtee.golf

import org.junit.Test
import org.junit.Assert.*

class PixteeCourseCatalogTest {
    @Test fun fullRosterProvides630SeededHoles() {
        assertEquals(35, PixteeCourseCatalog.courses.size)
        assertEquals(630, PixteeCourseCatalog.allHoleCount())
        assertEquals(35, PixteeCourseCatalog.courses.map { it.id }.toSet().size)
        for (c in PixteeCourseCatalog.courses.indices) {
            val variants = (1..18).map { h ->
                val l=PixteeCourseCatalog.hole(c,h)
                assertEquals(Ground.TEE, l.groundAt(l.teeX,l.teeY))
                assertEquals(Ground.GREEN, l.groundAt(l.pinX,l.pinY))
                assertFalse(l.waters.any { it.contains(l.teeX,l.teeY) })
                assertFalse(l.waters.any { it.contains(l.pinX,l.pinY) })
                listOf(l.teeX,l.teeY,l.pinX,l.pinY,l.bend)
            }
            assertEquals(18, variants.distinct().size)
        }
    }
    @Test fun stableGeometryAndScenery() {
        val a=PixteeCourseCatalog.hole(3,7)
        val b=PixteeCourseCatalog.hole(3,7)
        assertEquals(a.seed,b.seed)
        assertEquals(a.trees,b.trees)
        assertEquals(a.spectators,b.spectators)
    }
    @Test fun roundTransitionAndScoringAcrossDifferentLengths() {
        for(length in listOf(1,3,9,18)) {
            val r=PixteeRound(0,length,RoundMode.QUICK)
            var expected=0
            repeat(length) {
                assertFalse(r.isComplete)
                val hole=r.layout()
                expected+=hole.par
                r.record(hole.par,0,1)
            }
            assertTrue(r.isComplete)
            assertEquals(length,r.results.size)
            assertEquals(expected,r.totalStrokes)
            assertEquals(0,r.relativeToPar)
            try { r.record(4,0,1); fail("must reject extra score") }
            catch (_: IllegalStateException) {}
        }
    }
}
