package com.pixtee.golf

import org.junit.Assert.*
import org.junit.Test

/**
 * Headless integration test: 18 course layouts, 12-shot maximum pickup rule,
 * all strokes using the *real* three-click mechanics and fixed-step ball movement.
 * No reflection, injected scores or "forceHoleOut" testing shortcuts.
 */
class PixteeFullRoundTest {
    private fun playHole(core: PixteeCore): Int {
        var shots=0
        while(core.stage != GameStage.HOLED && shots < 12) {
            assertEquals("Ready between strokes",GameStage.READY,core.stage)
            core.whack()
            repeat(27) { core.tick() }
            assertEquals(GameStage.ACCURACY,core.whack())
            repeat(14) { core.tick() }
            if(core.stage == GameStage.ACCURACY) core.whack()
            var iterations=0
            while(core.stage != GameStage.READY && core.stage != GameStage.HOLED &&
                  iterations < 900) {
                core.tick()
                iterations++
            }
            assertTrue("Shot must end",core.stage==GameStage.READY ||
                core.stage==GameStage.HOLED)
            shots++
        }
        assertEquals("Every hole must reach a terminal state",GameStage.HOLED,core.stage)
        assertEquals(shots,core.shots.size)
        return core.strokes
    }

    @Test fun fullEighteenHoleRoundCompletesWithActualPhysics() {
        for(courseIndex in listOf(0,11,24)) {
            val round=PixteeRound(courseIndex,18,RoundMode.QUICK)
            val core=PixteeCore()
            for(hole in 1..18) {
                val layout=round.layout()
                assertEquals(hole,layout.number)
                core.startHole(layout)
                val strokes=playHole(core)
                val score=round.record(strokes,core.penalties,core.putts)
                assertEquals(hole,score.hole)
                assertEquals(layout.par,score.par)
            }
            assertTrue(round.isComplete)
            assertEquals(18,round.results.size)
            assertTrue(round.totalStrokes>=18)
            assertEquals(round.totalStrokes-round.totalPar,round.relativeToPar)
        }
    }
}
