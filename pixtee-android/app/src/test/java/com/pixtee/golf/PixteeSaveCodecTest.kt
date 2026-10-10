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
}
