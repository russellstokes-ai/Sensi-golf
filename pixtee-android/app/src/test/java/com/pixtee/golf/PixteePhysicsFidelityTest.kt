package com.pixtee.golf

import org.junit.Assert.*
import org.junit.Test
import kotlin.math.abs

/**
 * Pixtee tuning vectors: behavioral sanity without copying proprietary code
 * or asserting unsupported bit-identical Sensible Golf parity.
 */
class PixteePhysicsFidelityTest {
    @Test fun simulationClockMatchesClassicResearchCadence() {
        assertEquals(70f,PixteeCore.TICKS_PER_SECOND,0f)
        assertEquals(1f / 70f,PixteeCore.TICK_SECONDS,1e-7f)
    }

    private fun airborneSwing(downswingTicks:Int): PixteeCore {
        val g=PixteeCore()
        g.whack()
        repeat(29){g.tick()}
        assertEquals(GameStage.ACCURACY,g.whack())
        repeat(downswingTicks){g.tick()}
        assertEquals(GameStage.FLIGHT,g.whack())
        repeat(30){g.tick()}
        assertEquals(GameStage.FLIGHT,g.stage)
        assertTrue(g.height>0f)
        return g
    }

    @Test fun earlyVersusLateStrikeProduceDifferentMidflightCurves() {
        val early=airborneSwing(10)
        val late=airborneSwing(18)
        assertTrue("Opposite accuracy timings should change flight track",
            abs(early.x-late.x)>0.1f)
        assertEquals(early.x,airborneSwing(10).x,0.0001f)
        assertEquals(late.x,airborneSwing(18).x,0.0001f)
    }

    private fun puttingHole(seed:Long): HoleLayout {
        val a=PixteeCourseCatalog.hole(0,7)
        return HoleLayout(a.course,a.number,a.par,a.teeX,a.teeY,a.pinX,a.pinY,
            a.bend,a.width,a.waters,a.bunkers,seed)
    }

    private fun puttOnGreen(seed:Long): Pair<Float,Float> {
        val h=puttingHole(seed)
        val g=PixteeCore()
        g.startHole(h)
        g.restoreBall(StableBall(h.pinX+17f,h.pinY+9f,0,0,0,12,0f))
        assertEquals(Ground.GREEN,g.groundAt(g.x,g.y))
        g.whack()
        repeat(29){g.tick()}
        g.whack()
        repeat(16){g.tick()}
        g.whack()
        assertEquals(GameStage.ROLL,g.stage)
        repeat(38) { g.tick() }
        return g.x to g.y
    }

    @Test fun authoredGreenBreakIsDeterministicAndPhysicallyApplied() {
        val a=puttOnGreen(1L)
        val b=puttOnGreen(82L)
        assertTrue("Different green slopes must change the ball track",
            abs(a.first-b.first)>0.02f)
        assertEquals(a,puttOnGreen(1L))
    }

    @Test fun gradientExistsOnlyOnGreenAndIsBounded() {
        for(c in listOf(0,12,24)) for(h in 1..18) {
            val l=PixteeCourseCatalog.hole(c,h)
            val atPin=l.greenRollAcceleration(l.pinX,l.pinY)
            assertTrue(abs(atPin.first)<=3f)
            assertTrue(abs(atPin.second)<=3f)
            assertEquals(0f to 0f,l.greenRollAcceleration(l.teeX,l.teeY))
        }
    }
}
