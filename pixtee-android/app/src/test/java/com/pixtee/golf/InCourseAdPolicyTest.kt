package com.pixtee.golf

import org.junit.Assert.*
import org.junit.Test

class InCourseAdPolicyTest {
    @Test fun noFalseRevenueBeforeApprovalAndConsent() {
        val c=InCourseAdConfig(provider=InCourseAdProvider.ADVERTY_CUSTOM_ENGINE,
            publisherConfigured=true,gameApproved=true,
            userConsentAvailable=true,childAppropriateDemand=true)
        assertEquals(InCourseAdState.READY,InCourseAdPolicy.state(c))
        assertEquals(InCourseAdState.AWAITING_CONSENT,
            InCourseAdPolicy.state(c.copy(userConsentAvailable=false)))
        assertEquals(InCourseAdState.AWAITING_APPROVAL,
            InCourseAdPolicy.state(c.copy(gameApproved=false)))
        assertEquals(InCourseAdState.NOT_CONFIGURED,InCourseAdPolicy.state(InCourseAdConfig()))
        assertEquals(InCourseAdState.DISABLED,InCourseAdPolicy.state(c.copy(adsEnabled=false)))
        assertEquals(InCourseAdState.NOT_CONFIGURED,
            InCourseAdPolicy.state(c.copy(childAppropriateDemand=false)))
    }
    @Test fun onlyVisibleUnoccludedPlacementIsEligibleForProviderMeasurement() {
        assertFalse(ProgrammaticAdView("a",.49f,true,false).meetsVisualThreshold())
        assertFalse(ProgrammaticAdView("a",1f,false,false).meetsVisualThreshold())
        assertFalse(ProgrammaticAdView("a",1f,true,true).meetsVisualThreshold())
        assertTrue(ProgrammaticAdView("a",.6f,true,false).meetsVisualThreshold())
    }
    @Test fun everyCourseProvidesFourNonPhysicalAdSurfacesPerHole() {
        val layout=PixteeCourseCatalog.hole(24,18)
        val surfaces=InCourseAdPolicy.surfaces(layout.course.id,layout.number,
            layout.teeX,layout.teeY,layout.pinX,layout.pinY)
        assertEquals(4,surfaces.size)
        assertEquals(4,surfaces.map{it.id}.toSet().size)
        assertEquals(2,surfaces.count{it.zone==SponsorZone.TEE})
        assertEquals(2,surfaces.count{it.zone==SponsorZone.GREEN})
        val terrain=layout.groundAt(layout.teeX,layout.teeY)
        OfflineInCourseAds().beginHole(layout,surfaces)
        assertEquals(terrain,layout.groundAt(layout.teeX,layout.teeY))
    }
}
