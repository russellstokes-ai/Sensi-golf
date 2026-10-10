package com.pixtee.golf

/**
 * Offline-only career and cosmetics. No pay-to-win; every unlock is earned.
 * Five tiers each contain five score-based events for 25 distinct milestones.
 */
data class CareerEvent(val index: Int, val course: Int, val tier: Int,
                       val holes: Int, val targetToPar: Int) {
    val label: String get() = "EVENT ${index % 5 + 1} / 5"
}
object PixteeCareer {
    val tiers = listOf("AMATEUR", "REGIONAL", "NATIONAL", "PRO", "WORLD")
    val events: List<CareerEvent> = List(25) { i ->
        val tier=i / 5
        CareerEvent(i,i,tier,when {
            tier==0 -> listOf(3,3,9,9,18)[i%5]
            tier==1 -> listOf(3,9,9,18,18)[i%5]
            tier==2 -> listOf(9,9,18,18,18)[i%5]
            else -> 18
        },listOf(3,1,0,-1,-2)[tier])
    }
    fun completed(mask: Long, event: Int): Boolean =
        event in 0..24 && mask and (1L shl event) != 0L
    fun award(mask: Long, event: CareerEvent, toPar: Int): Long =
        if(toPar<=event.targetToPar) mask or (1L shl event.index) else mask

    fun tierUnlocked(mask: Long, tier: Int): Boolean {
        if(tier==0) return true
        if(tier !in 1..4) return false
        return (0 until tier*5).all { completed(mask,it) }
    }
    fun nextEvent(mask: Long, tier: Int): CareerEvent? {
        if(!tierUnlocked(mask,tier)) return null
        return events.filter { it.tier==tier }
            .firstOrNull { !completed(mask,it.index) }
    }
    fun titles(mask: Long): Int = java.lang.Long.bitCount(mask and ((1L shl 25)-1L))
}

data class ReaderStats(
    val holes: Int=0, val rounds: Int=0, val birdies: Int=0,
    val eagles: Int=0, val putts: Int=0, val shots: Int=0,
    val penalties: Int=0, val cleanNine: Int=0, val careerMask: Long=0L
) {
    val xp: Int get() = holes * 4 + rounds * 25 + birdies * 12 +
        eagles * 30 + PixteeCareer.titles(careerMask)*50
    val level: Int get() {
        val cutoffs = listOf(0,150,450,950,1650,2700,4200,6000,9000,13000)
        return (cutoffs.indexOfLast { xp>=it }+1).coerceAtLeast(1)
    }
}
data class PixteeAchievement(val id: String, val title: String, val unlocked: Boolean)
object PixteeRewards {
    fun achievements(s: ReaderStats): List<PixteeAchievement> = listOf(
        PixteeAchievement("first-hole","FIRST HOLE",s.holes>=1),
        PixteeAchievement("birdie","FIRST BIRDIE",s.birdies>=1),
        PixteeAchievement("eagle","FIRST EAGLE",s.eagles>=1),
        PixteeAchievement("clean-nine","BOGEY-FREE NINE",s.cleanNine>=1),
        PixteeAchievement("ten-rounds","TEN ROUNDS",s.rounds>=10),
        PixteeAchievement("tour-title","TOUR CHAMPION",PixteeCareer.titles(s.careerMask)>=5),
        PixteeAchievement("putting","100 PUTTS",s.putts>=100),
        PixteeAchievement("world","WORLD TOUR MASTER",PixteeCareer.titles(s.careerMask)>=25)
    )
}
