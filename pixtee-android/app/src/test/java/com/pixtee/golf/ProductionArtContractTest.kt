package com.pixtee.golf

import org.junit.Assert.*
import org.junit.Test

/** Production sprite mapping is explicit; rejected glyph art is not a fallback. */
class ProductionArtContractTest {
    @Test fun expectedProductionSpriteNamesAreStableAndUnique() {
        val ids = ProductionArtContract.REQUIRED_SPRITES
        assertEquals(19,ids.size)
        assertEquals(5,ProductionArtContract.TERRAIN_TILES.size)
        assertEquals(4,ProductionArtContract.UI_ART.size)
        assertEquals(28,(
            ids+ProductionArtContract.TERRAIN_TILES+ProductionArtContract.UI_ART
        ).distinct().size)
        assertEquals(ids.size,ids.distinct().size)
        assertTrue(ids.all { it.matches(Regex("[a-z][a-z_]+")) })
        assertEquals("art/production/APPROVED_v1.txt",ProductionArtContract.MANIFEST)
        assertEquals("PIXTEE_PRODUCTION_ART_APPROVED_V1",
            ProductionArtContract.APPROVAL_MARKER)
    }

    @Test fun allSwingFramesHaveExplicitProductionSlots() {
        val ids=ProductionArtContract.REQUIRED_SPRITES
        assertTrue(PixteeSwingRig.FULL_SWING.all { it.spriteId in ids })
        assertTrue(SwingPose.PUTT.spriteId in ids)
        assertTrue("spectator_photographer" in ids)
        assertTrue("camera_flash" in ids)
        assertTrue("bird_wings_up" in ids)
        assertTrue("bird_wings_down" in ids)
        val core=PixteeCore()
        assertEquals("golfer_idle",ProductionArtContract.golferFrame(core))
    }

    @Test fun artPixelResolutionCannotChangeWorldSpaceGolferScale() {
        val zoomed=CourseViewport(360f,760f,zoom=1.9f,
            focusX=150f,focusY=400f)
        val worldHeight=ProductionArtContract.candidateWorldHeight("golfer_idle")
        val rendered=zoomed.renderedModelHeight(worldHeight)
        assertTrue("Classic-size candidate should be legible",rendered>=22f)
        assertTrue("Character must not dominate the course",rendered<=29f)
        assertEquals(worldHeight,
            ProductionArtContract.candidateWorldHeight("golfer_follow"),0f)
        assertEquals(worldHeight,
            ProductionArtContract.candidateWorldHeight("golfer_impact"),0f)
        assertEquals(12f,
            ProductionArtContract.candidateWorldHeight("spectator_wave"),0f)
        assertTrue(ProductionArtContract.candidateWorldHeight("tree_round") >
            ProductionArtContract.candidateWorldHeight("golfer_idle"))
    }

    @Test fun everyApprovedSpriteSlotHasAWorldSize() {
        ProductionArtContract.REQUIRED_SPRITES.forEach { id ->
            assertTrue("$id has no playable scale",
                ProductionArtContract.candidateWorldHeight(id) in 1f..60f)
        }
        try {
            ProductionArtContract.candidateWorldHeight("unreviewed_placeholder")
            fail("Unapproved sprite name was accepted")
        } catch (_: IllegalArgumentException) { }
    }
}
