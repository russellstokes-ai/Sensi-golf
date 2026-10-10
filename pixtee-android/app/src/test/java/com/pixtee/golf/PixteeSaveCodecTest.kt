package com.pixtee.golf

import org.junit.Assert.*
import org.junit.Test

class PixteeSaveCodecTest {
    @Test fun roundTripAcrossSeveralHoles() {
        val r=PixteeRound(11,18,RoundMode.CAREER)
        repeat(7) { r.record(4,0,1) }
        val position=StableBall(140f,310f,2,0,1,3,-8f)
        val save=PixteeSaveCodec.encode(r,position)
        val restored=PixteeSaveCodec.decode(save)!!
        assertEquals(7,restored.round.results.size)
        assertEquals(8,restored.round.currentHole)
        assertEquals(28,restored.round.totalStrokes)
        assertEquals(position,restored.ball)
        assertEquals(r.courseIndex,restored.round.courseIndex)
        assertEquals(r.mode,restored.round.mode)
    }
    @Test fun damagedOrTruncatedSavesAreRejected() {
        val r=PixteeRound(0,3,RoundMode.QUICK)
        val value=PixteeSaveCodec.encode(r,null)
        assertNull(PixteeSaveCodec.decode(value.dropLast(1)))
        assertNull(PixteeSaveCodec.decode(value.replace("PX2","PX1")))
        assertNull(PixteeSaveCodec.decode(null))
        assertNotNull(PixteeSaveCodec.decode(value))
    }
    @Test fun careerRewardContextSurvivesProcessRestart() {
        val event=PixteeCareer.events[11]
        val round=PixteeRound(event.course,event.holes,RoundMode.CAREER)
        round.record(4,0,2)
        val save=PixteeSaveCodec.encode(round,null,event.index)
        val restored=PixteeSaveCodec.decode(save)!!
        assertEquals(event.index,restored.careerEventIndex)
        assertEquals(event.course,restored.round.courseIndex)
        assertEquals(event.holes,restored.round.length)
        assertEquals(1,restored.round.results.size)
    }

    @Test fun futureAndMismatchedCareerEventsCannotBeCredited() {
        val ev=PixteeCareer.events[6]
        val round=PixteeRound(ev.course,ev.holes,RoundMode.CAREER)
        try {
            PixteeSaveCodec.encode(round,null,9)
            fail("Wrong tournament ID should be rejected")
        } catch (_: IllegalArgumentException) {}
        val old=PixteeSaveCodec.encode(round,null)
        assertEquals(-1,PixteeSaveCodec.decode(old)!!.careerEventIndex)
    }

    @Test fun olderPx2SavesRemainReadableWithoutGrantingUnprovenRewards() {
        val round=PixteeRound(3,3,RoundMode.QUICK)
        round.record(4,0,1)
        val v3=PixteeSaveCodec.encode(round,null)
        val raw=v3.substringBeforeLast('|')
            .replaceFirst("PX3|","PX2|").substringBeforeLast('|')
        val checksum=java.util.zip.CRC32().apply {
            update(raw.toByteArray(Charsets.UTF_8))
        }.value.toString(16)
        val old=PixteeSaveCodec.decode("$raw|$checksum")!!
        assertEquals(1,old.round.results.size)
        assertEquals(-1,old.careerEventIndex)
    }

}
