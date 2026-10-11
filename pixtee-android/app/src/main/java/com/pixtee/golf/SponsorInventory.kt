package com.pixtee.golf

/** World-space sponsor signs. Never participate in ball physics or hitboxes. */
enum class SponsorZone { TEE, GREEN }
data class SponsorSlot(val id: String, val courseId: String, val hole: Int,
    val zone: SponsorZone, val x: Float, val y: Float)
data class SponsorCampaign(
    val id: String, val advertiser: String, val boardText: String,
    val slotIds: Set<String>, val startUtcSeconds: Long, val endUtcSeconds: Long,
    val approved: Boolean, val familySafe: Boolean,
    val logoFile: String? = null, val backgroundRgb: Int = 0x162F52,
    val foregroundRgb: Int = 0xFFFFFF
) {
    fun eligibleAt(nowUtcSeconds: Long): Boolean =
        approved && familySafe && advertiser.isNotBlank() &&
            boardText.length in 2..11 &&
            startUtcSeconds <= nowUtcSeconds && nowUtcSeconds < endUtcSeconds &&
            endUtcSeconds > startUtcSeconds && slotIds.isNotEmpty()
}
data class SponsorBoardView(val slot: SponsorSlot, val copy: String, val paid: Boolean,
    val logoFile: String?, val backgroundRgb: Int, val foregroundRgb: Int)

object SponsorInventory {
    const val DEFAULT_COURSE_ID = "lakewood"
    const val SLOTS_PER_HOLE = 4

    /** Four fixed slots per hole; IDs can be sold by zone, hole, course or season. */
    fun slots(courseId: String, hole: Int, teeX: Float, teeY: Float,
              pinX: Float, pinY: Float): List<SponsorSlot> {
        require(courseId.matches(Regex("[a-z0-9-]{3,50}")))
        require(hole in 1..18)
        val prefix = courseId + "-h" + hole.toString().padStart(2, '0')
        return listOf(
            SponsorSlot(prefix+"-tee-a", courseId, hole, SponsorZone.TEE, teeX-58f, teeY+10f),
            SponsorSlot(prefix+"-tee-b", courseId, hole, SponsorZone.TEE, teeX+60f, teeY+10f),
            SponsorSlot(prefix+"-green-a", courseId, hole, SponsorZone.GREEN, pinX-42f, pinY+62f),
            SponsorSlot(prefix+"-green-b", courseId, hole, SponsorZone.GREEN, pinX+66f, pinY+44f)
        )
    }

    fun show(slots: List<SponsorSlot>, campaigns: List<SponsorCampaign>,
             enabled: Boolean, nowUtcSeconds: Long): List<SponsorBoardView> {
        if (!enabled) return emptyList()
        val active = campaigns.filter { it.eligibleAt(nowUtcSeconds) }
        return slots.map { slot ->
            val ad = active.firstOrNull { slot.id in it.slotIds }
            if (ad == null) SponsorBoardView(slot, "PIXTEE", false, null, 0x34190B, 0xFFD14D)
            else SponsorBoardView(slot, ad.boardText.uppercase(), true,
                ad.logoFile, ad.backgroundRgb, ad.foregroundRgb)
        }
    }
}
