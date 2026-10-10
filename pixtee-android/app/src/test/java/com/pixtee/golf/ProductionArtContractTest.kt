package com.pixtee.golf

import org.junit.Assert.*
import org.junit.Test

/** Production sprite mapping is explicit; rejected glyph art is not a fallback. */
class ProductionArtContractTest {
    @Test fun expectedProductionSpriteNamesAreStableAndUnique() {
        val ids = ProductionArtContract.REQUIRED_SPRITES
        assertEquals(10,ids.size)
        assertEquals(ids.size,ids.distinct().size)
        assertTrue(ids.all { it.matches(Regex("[a-z][a-z_]+")) })
        assertEquals("art/production/APPROVED_v1.txt",ProductionArtContract.MANIFEST)
        assertEquals("PIXTEE_PRODUCTION_ART_APPROVED_V1",
            ProductionArtContract.APPROVAL_MARKER)
    }

    @Test fun allSwingStagesReferenceApprovedAssetIdsOnly() {
        val ids=ProductionArtContract.REQUIRED_SPRITES
        GameStage.values().forEach { stage ->
            assertTrue("No production artwork for $stage",
                ProductionArtContract.golferFrame(stage) in ids)
        }
        assertEquals("golfer_idle",ProductionArtContract.golferFrame(GameStage.READY))
        assertEquals("golfer_backswing",ProductionArtContract.golferFrame(GameStage.POWER))
        assertEquals("golfer_impact",ProductionArtContract.golferFrame(GameStage.ACCURACY))
        assertEquals("golfer_follow",ProductionArtContract.golferFrame(GameStage.FLIGHT))
        assertEquals("golfer_idle",ProductionArtContract.golferFrame(GameStage.ROLL))
    }
}
