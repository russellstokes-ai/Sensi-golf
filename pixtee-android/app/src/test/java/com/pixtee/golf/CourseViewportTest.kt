package com.pixtee.golf

import org.junit.Assert.*
import org.junit.Test

class CourseViewportTest {
    @Test fun tallPhoneShowsMoreVerticalWorldInsteadOfStretchingGolfers() {
        val compact = CourseViewport(360f, 720f)
        val tall = CourseViewport(360f, 800f)
        assertEquals(compact.worldScale, tall.worldScale, 0.00001f)
        assertEquals(compact.renderedModelHeight(14f),
            tall.renderedModelHeight(14f), 0.00001f)
        assertTrue(tall.visibleWorldHeight > compact.visibleWorldHeight)
        assertEquals(360f, compact.screenX(PixteeCore.WIDTH), 0.0001f)
    }

    @Test fun worldScreenTransformIsReversibleAtTeeAndPin() {
        for (screenH in listOf(720f, 760f, 800f, 920f)) {
            val v = CourseViewport(360f, screenH)
            for ((x, y) in listOf(PixteeCore.TEE_X to PixteeCore.TEE_Y,
                PixteeCore.PIN_X to PixteeCore.PIN_Y)) {
                assertEquals(x, v.worldX(v.screenX(x)), 0.0001f)
                assertEquals(y, v.worldY(v.screenY(y)), 0.0001f)
            }
        }
    }

    @Test fun newTallPhoneDoesNotChangeHoleLengthOrCollisionCoordinates() {
        val g = PixteeCore()
        val before = g.toPin
        val compact = CourseViewport(360f, 720f)
        val tall = CourseViewport(360f, 800f)
        assertTrue(tall.visibleWorldHeight > compact.visibleWorldHeight)
        assertEquals(before, g.toPin, 0.0001f)
        assertEquals(Ground.GREEN, g.groundAt(PixteeCore.PIN_X, PixteeCore.PIN_Y))
        assertEquals(Ground.TEE, g.groundAt(PixteeCore.TEE_X, PixteeCore.TEE_Y))
    }

    @Test fun zoomedCameraShowsPartOfCourseAtLargerUniformScale() {
        val old=CourseViewport(360f,760f)
        val closer=CourseViewport(360f,760f,
            zoom=CourseViewport.REFERENCE_ZOOM_CANDIDATE,
            focusX=150f,focusY=400f)
        assertTrue(closer.worldScale>old.worldScale)
        assertTrue(closer.visibleWorldWidth<old.visibleWorldWidth)
        assertTrue(closer.visibleWorldHeight<old.visibleWorldHeight)
        assertTrue(closer.visibleWorldHeight<PixteeCore.HEIGHT)
        assertEquals(closer.worldScale/old.worldScale,
            closer.renderedModelHeight(18f)/old.renderedModelHeight(18f),.0001f)
        assertTrue(closer.containsWorld(150f,459f))
        assertFalse(closer.containsWorld(150f,60f))
    }

    @Test fun followingBallChangesFramingButNeverWorldPositionsOrDistances() {
        val tee=CourseViewport(360f,760f,zoom=1.9f,
            focusX=150f,focusY=459f-CourseViewport.LOOK_AHEAD_WORLD)
        val green=CourseViewport(360f,760f,zoom=1.9f,
            focusX=150f,focusY=90f-CourseViewport.LOOK_AHEAD_WORLD)
        assertTrue(green.topWorld<tee.topWorld)
        assertTrue(tee.containsWorld(150f,459f))
        assertTrue(green.containsWorld(150f,90f))
        for(v in listOf(tee,green)) {
            assertEquals(150f,v.worldX(v.screenX(150f)),.0001f)
            assertEquals(260f,v.worldY(v.screenY(260f)),.0001f)
            assertEquals(v.worldScale,v.renderedModelHeight(1f),.0001f)
        }
    }

    @Test fun worldCropAndAspectRemainCorrectAcrossPhoneAndFoldLikeHeights() {
        for(height in listOf(680f,720f,760f,800f,920f)) {
            val camera=CourseViewport(360f,height,
                zoom=CourseViewport.REFERENCE_ZOOM_CANDIDATE,
                focusX=20f,focusY=400f)
            assertEquals(0f,camera.leftWorld,.001f)
            assertEquals(360f,camera.screenX(camera.rightWorld),.001f)
            assertEquals(0f,camera.screenY(camera.topWorld),.001f)
            assertEquals(height,camera.screenY(camera.bottomWorld),.001f)
            assertTrue(camera.containsWorld(20f,459f))
        }
    }
}
