package com.pixtee.golf

/**
 * Provider-neutral, intrinsic (in-world) advertising contract.
 *
 * IMPORTANT: This does not serve advertisements or claim ad impressions.
 * Production programmatic demand requires an approved provider SDK with its
 * own reporting, consent, viewability and certified ad rendering.
 *
 * Ads are VISUAL decoration only; never obstacle hitboxes or game controls.
 */
enum class InCourseAdState { DISABLED, NOT_CONFIGURED, AWAITING_CONSENT,
    AWAITING_APPROVAL, READY, NO_FILL, PROVIDER_ERROR }
enum class InCourseAdProvider { ADVERTY_CUSTOM_ENGINE, OTHER_APPROVED_PROVIDER }
data class InCourseAdConfig(
    val provider: InCourseAdProvider?=null,
    val publisherConfigured: Boolean=false,
    val gameApproved: Boolean=false,
    val userConsentAvailable: Boolean=false,
    val adsEnabled: Boolean=true,
    val childAppropriateDemand: Boolean=false
)
data class AdSurface(val id: String, val course: String, val hole: Int,
    val zone: SponsorZone, val x: Float, val y: Float) {
    init { require(id.isNotBlank() && hole in 1..18) }
}
data class ProgrammaticAdView(val slotId: String, val visibleFraction: Float,
    val isScreenActive: Boolean, val occluded: Boolean) {
    fun meetsVisualThreshold(): Boolean =
        isScreenActive && !occluded && visibleFraction >= .50f
}
object InCourseAdPolicy {
    const val DEFAULT_LABEL="PIXTEE"
    /**
     * An approved SDK decides the actual ad fill and billable impression,
     * not this app. A visible Pixtee placeholder is never a paid advertisement.
     */
    fun state(config: InCourseAdConfig): InCourseAdState = when {
        !config.adsEnabled -> InCourseAdState.DISABLED
        config.provider==null || !config.publisherConfigured ->
            InCourseAdState.NOT_CONFIGURED
        !config.gameApproved -> InCourseAdState.AWAITING_APPROVAL
        !config.userConsentAvailable -> InCourseAdState.AWAITING_CONSENT
        !config.childAppropriateDemand -> InCourseAdState.NOT_CONFIGURED
        else -> InCourseAdState.READY
    }

    fun surfaces(courseId:String,hole:Int,teeX:Float,teeY:Float,
                 pinX:Float,pinY:Float): List<AdSurface> =
        SponsorInventory.slots(courseId,hole,teeX,teeY,pinX,pinY).map {
            AdSurface(it.id,it.courseId,it.hole,it.zone,it.x,it.y)
        }
}

/**
 * To be supplied by a licensed in-game advertising SDK.
 * Rendered creative, viewability evidence, campaign fill and payouts must
 * be owned by the provider; NOT by the native Canvas drawing code.
 */
interface InCourseAdNetwork {
    val provider: InCourseAdProvider
    fun initialize(approvedGameId:String, consent:Boolean)
    fun beginHole(hole:HoleLayout, surfaces:List<AdSurface>)
    fun endHole()
    fun state(): InCourseAdState
}

/** Safe fallback used until the approved provider is actually integrated. */
class OfflineInCourseAds : InCourseAdNetwork {
    override val provider=InCourseAdProvider.ADVERTY_CUSTOM_ENGINE
    override fun initialize(approvedGameId:String,consent:Boolean) {}
    override fun beginHole(hole:HoleLayout,surfaces:List<AdSurface>) {}
    override fun endHole() {}
    override fun state()=InCourseAdState.NOT_CONFIGURED
}
