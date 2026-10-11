package com.pixtee.golf

import org.junit.Test
import org.junit.Assert.*

class PixteeWardrobeTest {
    @Test fun breadthAndUnlockability() {
        assertTrue(PixteeWardrobe.items.size>=75)
        assertEquals(PixteeWardrobe.items.size,PixteeWardrobe.items.map{it.id}.toSet().size)
        val champion=ReaderStats(holes=5000,rounds=400,birdies=200,eagles=60,
            putts=1800,cleanNine=30,careerMask=(1L shl 25)-1L)
        PixteeWardrobe.items.forEach { assertTrue(it.id,it.unlocked(champion)) }
    }
    @Test fun persistOnlyLegitimatelyEarnedEquipment() {
        val novice=ReaderStats()
        val locked=PixteeWardrobe.items.first{it.level>1}
        assertEquals(PixteeWardrobe.defaults,PixteeWardrobe.equip(
            PixteeWardrobe.defaults,locked.id,novice))
        val expert=ReaderStats(holes=5000,rounds=400,birdies=200,eagles=60,
            putts=1800,cleanNine=30,careerMask=(1L shl 25)-1L)
        val equipped=PixteeWardrobe.equip(PixteeWardrobe.defaults,locked.id,expert)
        val saved=PixteeWardrobe.encode(equipped,expert)
        assertEquals(equipped,PixteeWardrobe.decode(saved,expert))
        assertEquals(PixteeWardrobe.defaults,PixteeWardrobe.decode(saved,novice))
        assertEquals(PixteeWardrobe.defaults,
            PixteeWardrobe.decode("HAT=skin-0;CLUB=club-100;BALL=ball-7",novice))
    }
}
