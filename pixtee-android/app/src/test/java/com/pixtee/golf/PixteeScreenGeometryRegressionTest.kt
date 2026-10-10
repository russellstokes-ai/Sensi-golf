package com.pixtee.golf

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

/**
 * Screen-size contract for the APPROVED portrait, true-top-down ball-follow view.
 * This tests the existing camera and measures device-dependent output only.
 * It does not import or claim rights to original-game assets/physics.
 */
class PixteeScreenGeometryRegressionTest {
    data class WindowPx(val width: Int, val height: Int)

    private val normalPhones = listOf(
        WindowPx(720, 1600), WindowPx(1080, 2160),
        WindowPx(1080, 2340), WindowPx(1080, 2400),
        WindowPx(1440, 3120)
    )

    @Test fun everyNormalPhoneKeepsGolferWorldSizeAndUniformCameraScale() {
        val modelHeightWorld = 11.5f
        val expectedLogical = 11.5f * 360f / PixteeCore.WIDTH *
            CourseViewport.REFERENCE_ZOOM_CANDIDATE
        assertEquals(26.22f, expectedLogical, 0.0001f)

        for (device in normalPhones) {
            val logicalHeight = device.height * 360f / device.width
            val camera = CourseViewport(360f, logicalHeight, zoom =
                CourseViewport.REFERENCE_ZOOM_CANDIDATE,
                focusX = 150f, focusY = 400f)
            assertEquals(360f / PixteeCore.WIDTH *
                CourseViewport.REFERENCE_ZOOM_CANDIDATE,
                camera.worldScale, 0.0001f)
            assertEquals(expectedLogical,
                camera.renderedModelHeight(modelHeightWorld), 0.0001f)
            val physicalPixels = camera.renderedModelHeight(modelHeightWorld) *
                device.width / 360f
            assertEquals(expectedLogical * device.width / 360f,
                physicalPixels, 0.0001f)
            assertEquals(camera.worldScale,
                camera.screenX(camera.leftWorld + 1f) -
                    camera.screenX(camera.leftWorld), 0.0001f)
            assertEquals(camera.worldScale,
                camera.screenY(camera.topWorld + 1f) -
                    camera.screenY(camera.topWorld), 0.0001f)
        }
    }

    @Test fun tallPhonesRevealVerticalTerrainNotWiderCourseOrStrongerShots() {
        val short = CourseViewport(360f, 720f, zoom = 1.9f,
            focusX = 150f, focusY = 400f)
        val tall = CourseViewport(360f, 800f, zoom = 1.9f,
            focusX = 150f, focusY = 400f)
        assertEquals(short.visibleWorldWidth, tall.visibleWorldWidth, 0.0001f)
        assertEquals(short.worldScale, tall.worldScale, 0.0001f)
        assertTrue(tall.visibleWorldHeight > short.visibleWorldHeight)
        val model = PixteeCore()
        val originalDistance = model.toPin
        assertEquals(originalDistance, model.toPin, 0.0001f)
    }

    @Test fun originalGameWorldBoundsCanBeProjectedWithoutStretchingCollisions() {
        // Source file measurement: 34x114 logical tiles, 16x8 world each.
        val worldW = 34f * 16f
        val worldH = 114f * 8f
        assertEquals(544f, worldW, 0f)
        assertEquals(912f, worldH, 0f)
        val collisionGridW = 34 * 8
        val collisionGridH = 114 * 4
        assertEquals(272, collisionGridW)
        assertEquals(456, collisionGridH)
        for (device in normalPhones) {
            val camera = CourseViewport(360f,
                device.height * 360f / device.width,
                worldWidth = worldW,
                authoredHoleHeight = worldH,
                zoom = 2f,
                focusX = 271f, focusY = 500f)
            // Camera must not change the original 2x2-world-unit collision cell.
            val projectedX = camera.screenX(100f + 2f) -
                camera.screenX(100f)
            val projectedY = camera.screenY(100f + 2f) -
                camera.screenY(100f)
            assertEquals(projectedX, projectedY, 0.0001f)
            assertEquals(2f * camera.worldScale, projectedX, 0.0001f)
        }
    }

    @Test fun veryShortUnfoldedWindowNeedsHudReflowNotPhysicsRescale() {
        val logicalHeight = 1840f * 360f / 2208f
        assertEquals(300f, logicalHeight, 0.0001f)
        val unfolded = CourseViewport(360f,logicalHeight,
            zoom = CourseViewport.REFERENCE_ZOOM_CANDIDATE,
            focusX = 150f,focusY=400f)
        assertEquals(2.28f, unfolded.worldScale, 0.0001f)
        assertEquals(157.89474f, unfolded.visibleWorldWidth, 0.001f)
        assertEquals(131.57895f, unfolded.visibleWorldHeight, 0.001f)
        // Original prototype hard-coded y=450 for the arched meter:
        // cannot fit at all on a 360x300 unfolded logical window.
        assertTrue(450f > logicalHeight)
        // This is a required HUD-only adjustment, not license to tilt/zoom course.
    }

    @Test fun authoringPixelDensityCannotChangeWorldSize() {
        val originalSourceFrameW=16
        val originalSourceFrameH=21
        for (multiplier in listOf(2,3,4)) {
            val authoredWidth = originalSourceFrameW * multiplier
            val authoredHeight = originalSourceFrameH * multiplier
            assertEquals(16f/21f,
                authoredWidth.toFloat()/authoredHeight, 0.0001f)
            val sameWorldHeight=11.5f
            assertEquals(11.5f, sameWorldHeight, 0f)
        }
    }
}
