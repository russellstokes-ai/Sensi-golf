package com.pixtee.golf

import org.junit.Assert.*
import org.junit.Test

class PixteeCourseMasteryTest {
    private fun finished(
        course: Int, length: Int = 18, mode: RoundMode = RoundMode.QUICK,
        strokesAbovePar: Int = 0
    ): PixteeRound {
        val round=PixteeRound(course,length,mode)
        repeat(length) { i ->
            val par=round.layout().par
            // Total round score is exactly target; no impossible zero strokes.
            round.record(if(i==0) (par+strokesAbovePar).coerceAtLeast(1) else par,
                0,1)
        }
        return round
    }

    @Test fun unfinishedAndPracticeRoundsNeverEarnCourseMastery() {
        val base=PixteeCourseMastery()
        assertEquals(base,base.record(PixteeRound(0,18,RoundMode.QUICK)))
        assertEquals(base,base.record(finished(0,3)))
        assertEquals(base,base.record(finished(0,9)))
        assertEquals(base,base.record(finished(0,18,RoundMode.PRACTICE)))
        assertEquals(0,base.coursesPlayed)
    }

    @Test fun bronzeSilverGoldAndPlatinumAreScoreBasedAndMonotone() {
        var m=PixteeCourseMastery()
        m=m.record(finished(8,strokesAbovePar=20))
        assertEquals(MasteryMedal.BRONZE,m.medal(8))
        m=m.record(finished(8,strokesAbovePar=8))
        assertEquals(MasteryMedal.SILVER,m.medal(8))
        m=m.record(finished(8))
        assertEquals(MasteryMedal.GOLD,m.medal(8))
        // For -9, distribute nine under-par strokes over distinct holes.
        val eagleRound=PixteeRound(8,18,RoundMode.TOUR)
        repeat(18) {
            val par=eagleRound.layout().par
            eagleRound.record(par-if(it<9) 1 else 0,0,1)
        }
        m=m.record(eagleRound)
        assertEquals(MasteryMedal.PLATINUM,m.medal(8))
        assertEquals(4,m.records[8]!!.completedRounds)
        assertEquals(-9,m.records[8]!!.bestRelativeToPar)
        m=m.record(finished(8,strokesAbovePar=100))
        assertEquals(MasteryMedal.PLATINUM,m.medal(8))
    }

    @Test fun completingOneFullRoundOnEveryCoursePersistsAll35Medals() {
        var mastery=PixteeCourseMastery()
        for(i in PixteeCourseCatalog.courses.indices) {
            mastery=mastery.record(finished(i))
        }
        assertEquals(35,mastery.coursesPlayed)
        assertEquals(35,mastery.totalCompletedRounds)
        assertTrue(PixteeCourseCatalog.courses.indices.all {
            mastery.medal(it)==MasteryMedal.GOLD
        })
        assertEquals(mastery,PixteeCourseMasteryCodec.decode(
            PixteeCourseMasteryCodec.encode(mastery)))
    }

    @Test fun invalidSavesAndDuplicateRecordsCannotUnlockMedals() {
        assertEquals(PixteeCourseMastery(),
            PixteeCourseMasteryCodec.decode("PXMASTERY1|"))
        assertNull(PixteeCourseMasteryCodec.decode(null))
        assertNull(PixteeCourseMasteryCodec.decode("PXMASTERY1|0,1,-20,1,0;0,1,-20,1,0"))
        assertNull(PixteeCourseMasteryCodec.decode("PXMASTERY1|35,1,0,1,0"))
        assertNull(PixteeCourseMasteryCodec.decode("PXMASTERY1|0,-1,0,1,0"))
        assertNull(PixteeCourseMasteryCodec.decode("PXMASTERY1|0,1,-999,1,0"))
        assertNull(PixteeCourseMasteryCodec.decode("PXMASTERY1|0,1,0,1,0|extra"))
    }
}
