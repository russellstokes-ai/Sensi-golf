package com.pixtee.golf

import org.junit.Assert.*
import org.junit.Test

class PixteeCoreTest {
    @Test fun thirteenStandardClubSlots() {
        assertEquals(13, PixteeCore.CLUBS.size)
        assertEquals("Putter", PixteeCore.CLUBS.last().label)
    }

    @Test fun threePressesOnlyCommitOneShot() {
        val g = PixteeCore()
        assertEquals(GameStage.POWER, g.whack())
        assertEquals(0, g.strokes)
        repeat(38) { g.tick() }
        assertEquals(GameStage.ACCURACY, g.whack())
        assertEquals(0, g.strokes)
        repeat(19) { g.tick() }
        assertEquals(GameStage.FLIGHT, g.whack())
        assertEquals(1, g.strokes)
        assertEquals(GameStage.FLIGHT, g.whack()) // cannot submit twice
        assertEquals(1, g.strokes)
    }

    @Test fun powerPeaksAtTopOfWellyArc() {
        assertEquals(0f, PixteeCore.powerForMeter(0f), 0.0001f)
        assertEquals(1f, PixteeCore.powerForMeter(0.5f), 0.0001f)
        assertEquals(0f, PixteeCore.powerForMeter(1f), 0.0001f)
        assertEquals(PixteeCore.powerForMeter(0.25f),
            PixteeCore.powerForMeter(0.75f), 0.0001f)
        val g = PixteeCore()
        g.whack()
        repeat(26) { g.tick() } // near the physical top of the arch
        assertEquals(GameStage.ACCURACY, g.whack())
        assertTrue("Top-of-arc press must produce a strong shot", g.chosenPower > 0.95f)
    }

    @Test fun aimAndClubsLockedDuringWhack() {
        val g = PixteeCore()
        g.whack()
        g.steer(20f)
        g.changeClub(1)
        assertEquals(0f, g.aimDegrees)
        assertEquals(0, g.clubIndex)
    }

    @Test fun fixedTickReproducibility() {
        fun run(): Pair<Float, Float> {
            val g = PixteeCore()
            g.whack(); repeat(40) { g.tick() }; g.whack()
            repeat(19) { g.tick() }; g.whack()
            repeat(300) { g.tick() }
            assertEquals(GameStage.READY, g.stage)
            assertEquals(1, g.shots.size)
            return g.x to g.y
        }
        assertEquals(run(), run())
    }

    @Test fun noWindDefaultAndNoAssetsNeeded() {
        val g = PixteeCore()
        assertEquals(0, g.penalties)
        assertEquals(GameStage.READY, g.stage)
        assertEquals(388f, g.toPin, 8f)
    }
    @Test fun wellyDownswingContinuesSameArcAndCanHitStraight() {
        val g = PixteeCore()
        assertEquals(GameStage.POWER, g.whack())
        repeat(38) { g.tick() }
        val power = g.meter
        assertTrue(power > 0.5f)
        assertEquals(GameStage.ACCURACY, g.whack())
        assertEquals(power, g.meter, 0.00001f) // no restart on second press
        var ticks = 0
        while (g.meter > 0.015f && ticks < 100) {
            g.tick()
            ticks++
        }
        assertEquals(GameStage.ACCURACY, g.stage)
        assertEquals(GameStage.FLIGHT, g.whack())
        assertEquals(1, g.strokes)
        assertEquals(0.5f, g.accuracy, 0.05f)
    }

    @Test fun waitingTooLongOnDownswingCannotLeaveMeterStuck() {
        val g = PixteeCore()
        g.whack()
        repeat(52) { g.tick() }
        g.whack()
        repeat(200) { g.tick() }
        assertTrue(g.stage == GameStage.FLIGHT || g.stage == GameStage.ROLL ||
            g.stage == GameStage.READY)
        assertEquals(1, g.strokes)
    }

    @Test fun putterMovesAlongGroundAndFinishes() {
        val g = PixteeCore()
        g.changeClub(12)
        assertEquals("Putter", g.club.label)
        g.whack()
        repeat(36) { g.tick() }
        g.whack()
        repeat(18) { g.tick() }
        assertEquals(GameStage.ROLL, g.whack())
        val startingY = g.y
        repeat(5) { g.tick() }
        assertTrue("Putter should actually travel on the green/ground", g.y < startingY)
        assertEquals(0f, g.height, 0.0001f)
        repeat(280) { g.tick() }
        assertTrue(g.stage == GameStage.READY || g.stage == GameStage.HOLED)
    }

    @Test fun woodsMoveDuringRollRatherThanFakeWaiting() {
        val g = PixteeCore()
        g.whack()
        repeat(37) { g.tick() }
        g.whack()
        repeat(19) { g.tick() }
        g.whack()
        for (i in 0 until 300) {
            if (g.stage == GameStage.ROLL) break
            g.tick()
        }
        assertEquals(GameStage.ROLL, g.stage)
        val landingY = g.y
        g.tick()
        assertTrue("Roll phase should continue moving", g.y < landingY)
        assertTrue("Small post-landing bounce must show a lift", g.height > 0f)
    }

}
