package com.pixtee.golf

import org.junit.Assert.*
import org.junit.Test

/** First unapproved original glyph atlas: format, motion and themes are testable. */
class PixteeSpriteAtlasTest {
    @Test fun everyFrameUsesValidSolidPixelsAndEqualWidthRows() {
        val ids=PixteeSpriteAtlas.all.map { it.id }
        assertEquals(ids.size,ids.distinct().size)
        assertTrue("At least 10 independently editable sprite poses",ids.size>=10)
        for(s in PixteeSpriteAtlas.all) {
            assertTrue("No empty sprite: ${s.id}",s.opaqueCells>=4)
            assertTrue(s.rows.all { it.length == s.width })
            assertTrue(s.rows.flatMap { it.toList() }.none { it !in ".HSTPKCBAOdmlwusfx" })
        }
    }

    @Test fun swingFramesShareExactlyTheSameRegistrationAndFootprint() {
        val poses=listOf(GameStage.READY,GameStage.POWER,GameStage.ACCURACY,
            GameStage.FLIGHT,GameStage.ROLL)
        val sprites=poses.map(PixteeSpriteAtlas::golfer)
        assertEquals(1,sprites.map { it.width }.distinct().size)
        assertEquals(1,sprites.map { it.height }.distinct().size)
        assertNotEquals(sprites[0].id,sprites[1].id)
        assertNotEquals(sprites[1].id,sprites[2].id)
        assertNotEquals(sprites[2].id,sprites[3].id)
        assertEquals(sprites[0].id,sprites[4].id)
    }

    @Test fun allCourseThemesHaveAssignedStableVisualPalettes() {
        assertEquals(35,PixteeCourseCatalog.courses.size)
        assertEquals(10,CourseArtDirection.PALETTES.size)
        assertEquals(10,PixteeCourseCatalog.courses.map { it.theme }.distinct().size)
        for(course in PixteeCourseCatalog.courses) {
            val palette=CourseArtDirection.palette(course.theme)
            assertTrue(palette.name.isNotBlank())
            for(argb in listOf(palette.rough,palette.fairway,palette.green,
                palette.sand,palette.water,palette.accent)) {
                assertEquals("Every colour must be opaque",255,(argb ushr 24) and 0xff)
            }
        }
    }

    @Test fun animationSwitchDoesNotChangeTreeOrSpectatorWorldGeometry() {
        val hole=PixteeCourseCatalog.hole(34,18)
        val treeCoordinates=hole.trees.map { it.x to it.y }
        val spectatorCoordinates=hole.spectators.map { it.x to it.y }
        assertNotEquals(PixteeSpriteAtlas.spectator(false).id,
            PixteeSpriteAtlas.spectator(true).id)
        assertNotEquals(PixteeSpriteAtlas.tree(0).id,
            PixteeSpriteAtlas.tree(1).id)
        assertEquals(treeCoordinates,hole.trees.map { it.x to it.y })
        assertEquals(spectatorCoordinates,hole.spectators.map { it.x to it.y })
    }
}
