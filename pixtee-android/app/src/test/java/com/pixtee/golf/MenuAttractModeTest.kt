package com.pixtee.golf

import org.junit.Assert.*
import org.junit.Test

class MenuAttractModeTest {
    @Test fun autonomousPlayersRunSamePhysicsAsHumanButHaveIndependentState() {
        val game=MenuAttractMode()
        assertEquals(3,game.actors.size)
        assertEquals(3,game.actors.map { it.player }.toSet().size)
        assertEquals(3,game.actors.map { it.player.x to it.player.y }.toSet().size)
        val human=PixteeCore()
        human.startHole(PixteeCourseCatalog.hole(7,6))
        val humanInitial=human.stableBall()
        repeat(280) { game.tick() }
        assertEquals(280L,game.ticks)
        assertEquals(humanInitial,human.stableBall())
        assertEquals(0,human.strokes)
        assertTrue("At least one autonomous golfer must make a real shot",
            game.actors.any { it.player.strokes>0 })
        assertTrue("At least one real ball must separate from its starting point",
            game.actors.any { it.player.shots.isNotEmpty() ||
                it.player.x != it.player.golferWorldX ||
                it.player.y != it.player.golferWorldY })
    }

    @Test fun controlledSimulationIsRepeatableAcrossIndependentRuns() {
        val a=MenuAttractMode()
        val b=MenuAttractMode()
        repeat(450) { a.tick();b.tick() }
        assertEquals(a.ticks,b.ticks)
        for(i in 0..2) {
            assertEquals(a.actors[i].player.stage,b.actors[i].player.stage)
            assertEquals(a.actors[i].player.x,b.actors[i].player.x,0.00001f)
            assertEquals(a.actors[i].player.y,b.actors[i].player.y,0.00001f)
            assertEquals(a.actors[i].player.strokes,b.actors[i].player.strokes)
        }
    }

    @Test fun menuCannotEverIncreasePlayerRoundOrCareerStats() {
        val actualRound=PixteeRound(4,18,RoundMode.CAREER)
        val before=actualRound.results.toList()
        val demo=MenuAttractMode()
        repeat(1400){demo.tick()}
        assertEquals(before,actualRound.results)
        assertEquals(0,actualRound.totalStrokes)
        assertEquals(0,actualRound.totalPenalties)
        assertFalse(actualRound.isComplete)
        assertEquals(3,demo.actors.size)
        assertTrue(demo.actors.all { it.player.shots.size<=3 })
    }

    @Test fun menuActorsAlwaysUseTheirOwnCourseAndValidBallBounds() {
        val demo=MenuAttractMode(PixteeCourseCatalog.hole(34,18))
        repeat(1200) {
            demo.tick()
            for(actor in demo.actors) {
                val p=actor.player
                assertEquals(demo.hole,p.activeHole)
                assertTrue(p.x in 0f..PixteeCore.WIDTH)
                assertTrue(p.y in 0f..PixteeCore.HEIGHT)
            }
        }
    }
}
