package com.pixtee.golf

import org.junit.Assert.*
import org.junit.Test

class PixteeCareerTest {
    @Test fun twentyFiveEventsAreSequentiallyUnlocked() {
        assertEquals(25,PixteeCareer.events.size)
        var mask=0L
        assertTrue(PixteeCareer.tierUnlocked(mask,0))
        assertFalse(PixteeCareer.tierUnlocked(mask,1))
        for(i in 0 until 25) {
            val ev=PixteeCareer.events[i]
            assertEquals(ev.index,PixteeCareer.nextEvent(mask,ev.tier)?.index)
            assertEquals(mask,PixteeCareer.award(mask,ev,ev.targetToPar+1))
            mask=PixteeCareer.award(mask,ev,ev.targetToPar)
            assertTrue(PixteeCareer.completed(mask,i))
        }
        assertEquals(25,PixteeCareer.titles(mask))
        assertNull(PixteeCareer.nextEvent(mask,4))
    }

    @Test fun achievementsAndLevelDoNotMaxOutInFirstFewRounds() {
        val beginner=ReaderStats(holes=3,rounds=1)
        assertEquals(1,beginner.level)
        assertTrue(PixteeRewards.achievements(beginner)[0].unlocked)
        assertFalse(PixteeRewards.achievements(beginner)[4].unlocked)
        val reader=ReaderStats(holes=250,rounds=20,birdies=14,eagles=3)
        assertTrue(reader.level>beginner.level)
        assertTrue(reader.level<10)
    }
}
