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

}
