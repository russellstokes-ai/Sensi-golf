package com.pixtee.golf

import org.junit.Assert.*
import org.junit.Test

class SponsorInventoryTest {
    private val slots = SponsorInventory.slots(
        "lakewood", 1, PixteeCore.TEE_X, PixteeCore.TEE_Y,
        PixteeCore.PIN_X, PixteeCore.PIN_Y)
    private fun campaign(start: Long=100, end: Long=200,
                         approved: Boolean=true, familySafe: Boolean=true) =
        SponsorCampaign("local-example", "Example sponsor", "EXAMPLE",
            setOf("lakewood-h01-tee-a","lakewood-h01-green-a"),
            start, end, approved, familySafe)

    @Test fun fourStableSlotsTeeAndGreenPerHole() {
        assertEquals(4, slots.size)
        assertEquals(4, slots.map { it.id }.toSet().size)
        assertEquals(2, slots.count { it.zone == SponsorZone.TEE })
        assertEquals(2, slots.count { it.zone == SponsorZone.GREEN })
        assertEquals("lakewood-h01-tee-a", slots.first().id)
        assertEquals("lakewood-h01-green-b", slots.last().id)
        assertEquals(72, (1..18).sumOf {
            SponsorInventory.slots("lakewood",it,151f,459f,145f,67f).size
        })
    }
    @Test fun disabledBoardsDoNotAffectCollision() {
        val g = PixteeCore()
        val pos = g.x to g.y
        assertTrue(SponsorInventory.show(slots,listOf(campaign()),false,150).isEmpty())
        assertEquals(pos, g.x to g.y)
        assertEquals(Ground.TEE, g.groundAt(g.x,g.y))
    }
    @Test fun houseDefaultsAndApprovedPaidCampaigns() {
        val house = SponsorInventory.show(slots,emptyList(),true,150)
        assertEquals(4, house.size)
        assertTrue(house.none { it.paid })
        val live = SponsorInventory.show(slots,listOf(campaign()),true,150)
        assertEquals(2,live.count { it.paid })
        assertEquals("EXAMPLE",live.first().copy)
        for (c in listOf(campaign(approved=false),campaign(familySafe=false),
            campaign(start=151),campaign(end=150))) {
            assertTrue(SponsorInventory.show(slots,listOf(c),true,150).none { it.paid })
        }
    }
    @Test fun expiryAndStartBoundary() {
        assertFalse(campaign().eligibleAt(200))
        assertFalse(campaign().eligibleAt(99))
        assertTrue(campaign().eligibleAt(100))
    }
    @Test fun identicalBallFlightWithAndWithoutSponsors() {
        fun shot(): Pair<Float,Float> {
            val g = PixteeCore()
            SponsorInventory.show(slots,listOf(campaign()),true,150)
            g.whack();repeat(39){g.tick()};g.whack()
            repeat(20){g.tick()};g.whack()
            repeat(300){g.tick()}
            return g.x to g.y
        }
        assertEquals(shot(),shot())
    }
}
