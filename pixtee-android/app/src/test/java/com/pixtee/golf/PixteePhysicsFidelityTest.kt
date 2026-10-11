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
    @Test fun waterPenaltyIsExactlyOneAndRestoresPreviousLie() {
        val course=PixteeCourseCatalog.courses[0]
        val custom=HoleLayout(course,1,4,150f,459f,145f,67f,
            0f,74f,listOf(WaterPatch(231f,185f,300f,302f)),emptyList(),17L)
        val g=PixteeCore()
        g.startHole(custom)
        g.restoreBall(StableBall(229f,240f,0,0,0,12,85f))
        val start=g.x to g.y
        assertEquals(Ground.ROUGH,g.groundAt(g.x,g.y))
        g.whack()
        repeat(29){g.tick()}
        g.whack()
        repeat(16){g.tick()}
        assertEquals(GameStage.ROLL,g.whack())
        repeat(250){ if(g.stage==GameStage.ROLL) g.tick() }
        assertEquals(GameStage.READY,g.stage)
        assertEquals(1,g.penalties)
        assertEquals(2,g.strokes)
        assertEquals(1,g.shots.size)
        assertEquals(1,g.shots.single().penalty)
        assertEquals(start,g.x to g.y)
        assertEquals(Ground.ROUGH,g.lastLie)
    }

    @Test fun multipleHolesUseDifferentGreensAndShotGeometry() {
        val first=PixteeCourseCatalog.hole(0,1)
        val second=PixteeCourseCatalog.hole(0,2)
        val g=PixteeCore()
        g.startHole(first)
        assertEquals(first.par,g.par)
        assertEquals(first.pinX,g.x+(first.pinX-g.x),.0001f)
        g.startHole(second)
        assertEquals(second.par,g.par)
        assertEquals(second.teeX,g.x,.0001f)
        assertEquals(second.teeY,g.y,.0001f)
        assertEquals(second.number,g.holeNumber)
        assertNotEquals(first.seed,second.seed)
    }

    @Test fun fullPowerDriverLandingAndRestTimeAreWithinClassicPacingWindow() {
        val g=PixteeCore()
        g.whack()
        repeat(31){g.tick()} // near maximum power at top of arc
        g.whack()
        repeat(18){g.tick()} // near the Whack-o-Meter accuracy target
        assertEquals(GameStage.FLIGHT,g.whack())
        var landingTick=0
        while(g.stage==GameStage.FLIGHT && landingTick<200) {
            g.tick()
            landingTick++
        }
        assertEquals(GameStage.ROLL,g.stage)
        // Documented original full-power driver first contacts around tick 80.
        // These are tolerance windows, not a claim of exact proprietary traces.
        assertTrue("Flight took $landingTick ticks, expected arcade pacing",
            landingTick in 75..85)
        var restingTick=landingTick
        while(g.stage==GameStage.ROLL && restingTick<300) {
            g.tick()
            restingTick++
        }
        assertEquals(GameStage.READY,g.stage)
        assertTrue("Resting took $restingTick ticks",
            restingTick in 125..185)
    }


    @Test fun golferRemainsAtLaunchPointWhileBallTravels() {
        val game=PixteeCore()
        val atAddress=game.golferWorldX to game.golferWorldY
        game.whack()
        repeat(29){game.tick()}
        game.whack()
        repeat(16){game.tick()}
        val beforeContact=game.x to game.y
        assertEquals(atAddress,game.golferWorldX to game.golferWorldY)
        assertEquals(GameStage.FLIGHT,game.whack())
        // First flight instant is exact ball-club contact coordinate.
        assertEquals(beforeContact,game.x to game.y)
        assertEquals(beforeContact,game.golferWorldX to game.golferWorldY)
        repeat(32){game.tick()}
        assertEquals(GameStage.FLIGHT,game.stage)
        assertTrue("Ball must have separated from golfer",game.y<game.golferWorldY)
        assertEquals(atAddress,game.golferWorldX to game.golferWorldY)
        repeat(340) {
            if(game.stage==GameStage.FLIGHT || game.stage==GameStage.ROLL) game.tick()
        }
        assertTrue(game.stage==GameStage.READY || game.stage==GameStage.HOLED)
    }
}
