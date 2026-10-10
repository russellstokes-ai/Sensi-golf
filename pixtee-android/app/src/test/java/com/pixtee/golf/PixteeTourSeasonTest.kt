package com.pixtee.golf

import org.junit.Assert.*
import org.junit.Test

class PixteeTourSeasonTest {
    @Test fun tourRotatesAcrossAll35CoursesWithoutReplacingExistingCareer() {
        val firstThree=(1..3).flatMap { s ->
            (0 until PixteeSeasons.EVENTS_PER_SEASON).map { event ->
                PixteeSeasons.event(s,event).courseIndex
            }
        }
        assertEquals(35,firstThree.toSet().size)
        assertEquals((0 until 35).toList(),firstThree.take(35))
        assertEquals(25,PixteeCareer.events.size)
        assertEquals(5,PixteeCareer.tiers.size)
    }

    @Test fun entireSeasonScoresRivalsAndCarriesTitlesIntoNextSeason() {
        var season=TourSeason()
        assertEquals(1,season.number)
        assertFalse(season.finished)
        repeat(PixteeSeasons.EVENTS_PER_SEASON) {
            val event=season.next!!
            assertEquals(it,event.index)
            val playerToPar=event.rivals.minOf { it.toPar }-1
            assertEquals(1,event.playerPosition(playerToPar))
            season=season.record(playerToPar)
        }
        assertTrue(season.finished)
        assertEquals(12,season.results.size)
        assertEquals(12*25,season.points)
        assertNull(season.next)
        try { season.record(0); fail("Must not exceed 12 events") }
        catch (_: IllegalStateException) {}
        val next=season.advance()
        assertEquals(2,next.number)
        assertEquals(1,next.championships)
        assertEquals(0,next.results.size)
        assertEquals(12,PixteeSeasons.EVENTS_PER_SEASON)
    }

    @Test fun saveCodecRejectsForgedPointsAndRestoresProgress() {
        val state=TourSeason().record(4).record(2)
        val encoded=PixteeSeasonCodec.encode(state)
        assertEquals(state,PixteeSeasonCodec.decode(encoded))
        assertNull(PixteeSeasonCodec.decode(encoded.replaceFirst("|", "")))
        assertNull(PixteeSeasonCodec.decode("PXTOUR1|1|0|0,999,1"))
        assertNull(PixteeSeasonCodec.decode("PXTOUR1|1|0|0,25,10"))
        assertNull(PixteeSeasonCodec.decode("PXTOUR1|0|0|"))
    }

    @Test fun seasonsUsePredictableScoresIndependentOfClockAndNetwork() {
        val event=PixteeSeasons.event(37,5)
        assertEquals(event,PixteeSeasons.event(37,5))
        assertEquals(4,event.rivals.size)
        assertEquals(4,event.rivals.map { it.name }.toSet().size)
        assertTrue(PixteeSeasons.rivalSeasonTotals(37).all { it>0 })
        assertTrue(event.playerPosition(-99) in 1..5)
        assertTrue(event.playerPosition(99) in 1..5)
    }
}
