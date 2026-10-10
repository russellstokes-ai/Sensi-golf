package com.pixtee.golf

import org.junit.Assert.*
import org.junit.Test

class PixteeSwingRigTest {
    @Test fun eightSwingPosesAreUniqueAndOrdered() {
        val expected=listOf(SwingPose.ADDRESS,SwingPose.TAKEAWAY,
            SwingPose.BACKSWING,SwingPose.TOP,SwingPose.DOWNSWING,
            SwingPose.IMPACT,SwingPose.FOLLOW_THROUGH,SwingPose.FINISH)
        assertEquals(expected,PixteeSwingRig.FULL_SWING)
        assertEquals(8,expected.map { it.spriteId }.distinct().size)
        assertEquals(0f,PixteeSwingRig.BALL_OFFSET_Y,0f)
        assertEquals(12f,PixteeSwingRig.BALL_OFFSET_X,0f)
    }

    @Test fun realWhackSequenceDrivesCompleteSwingIncludingImpact() {
        val core=PixteeCore()
        assertEquals(SwingPose.ADDRESS,PixteeSwingRig.phase(core))
        assertEquals(GameStage.POWER,core.whack())
        assertEquals(SwingPose.ADDRESS,PixteeSwingRig.phase(core))
        repeat(9){core.tick()}
        assertEquals(SwingPose.TAKEAWAY,PixteeSwingRig.phase(core))
        repeat(11){core.tick()}
        assertEquals(SwingPose.BACKSWING,PixteeSwingRig.phase(core))
        repeat(11){core.tick()}
        assertEquals(SwingPose.TOP,PixteeSwingRig.phase(core))
        assertEquals(GameStage.ACCURACY,core.whack())
        repeat(7){core.tick()}
        assertEquals(SwingPose.DOWNSWING,PixteeSwingRig.phase(core))
        val before=core.x to core.y
        assertEquals(GameStage.FLIGHT,core.whack())
        assertEquals(SwingPose.IMPACT,PixteeSwingRig.phase(core))
        assertEquals(0,core.postContactTicks)
        assertEquals(before,core.x to core.y)
        assertEquals(before,core.golferWorldX to core.golferWorldY)
        core.tick()
        assertEquals(SwingPose.FOLLOW_THROUGH,PixteeSwingRig.phase(core))
        repeat(8){core.tick()}
        assertEquals(SwingPose.FINISH,PixteeSwingRig.phase(core))
        assertNotEquals("Ball should be moving during follow-through",
            before,core.x to core.y)
        assertEquals(before,core.golferWorldX to core.golferWorldY)
    }

    @Test fun impactAnchorIsExactForEveryCameraAndIsNotAnArtGuess() {
        val feetX=140f
        val feetY=420f
        val ball=PixteeSwingRig.expectedBallPosition(feetX,feetY)
        assertEquals(152f,ball.first,0f)
        assertEquals(420f,ball.second,0f)
        for(height in listOf(700f,760f,920f)) {
            val view=CourseViewport(360f,height,
                zoom=CourseViewport.REFERENCE_ZOOM_CANDIDATE,
                focusX=150f,focusY=380f)
            assertEquals(12f*view.worldScale,
                view.screenX(ball.first)-view.screenX(feetX),0.001f)
            assertEquals(view.screenY(feetY),view.screenY(ball.second),0.001f)
        }
    }

    @Test fun puttingNeverUsesAnUnrelatedDriverFollowThrough() {
        assertEquals(SwingPose.PUTT,PixteeSwingRig.phase(
            GameStage.ROLL,3,7,5,true))
        assertEquals(SwingPose.IMPACT,PixteeSwingRig.phase(
            GameStage.ROLL,3,7,0,true))
        assertEquals(SwingPose.ADDRESS,PixteeSwingRig.phase(
            GameStage.ROLL,3,7,20,true))
        assertEquals(SwingPose.FOLLOW_THROUGH,PixteeSwingRig.phase(
            GameStage.FLIGHT,30,10,2,false))
    }

    @Test fun allFullSwingFramesShareRegistrationAndContactExactly() {
        assertTrue(PixteeSwingRig.isValidFrameSize(128,64))
        assertFalse(PixteeSwingRig.isValidFrameSize(127,64))
        assertFalse(PixteeSwingRig.isValidFrameSize(128,63))
        val feetX=150f;val feetY=459f
        val rect=PixteeSwingRig.artFrameRect(feetX,feetY)
        val contact=PixteeSwingRig.contactFromFrameRect(rect)
        val expected=PixteeSwingRig.expectedBallPosition(feetX,feetY)
        assertEquals(expected.first,contact.first,0.0001f)
        assertEquals(expected.second,contact.second,0.0001f)
        assertEquals(128f/PixteeSwingRig.SOURCE_PIXELS_PER_WORLD,
            rect[2]-rect[0],0.0001f)
        assertEquals(64f/PixteeSwingRig.SOURCE_PIXELS_PER_WORLD,
            rect[3]-rect[1],0.0001f)
        // Direction/camera scaling must never move the clubhead relative
        // to the actual physical ball at shot release.
        for(h in listOf(720f,800f,920f)) {
            val camera=CourseViewport(360f,h,
                zoom=CourseViewport.REFERENCE_ZOOM_CANDIDATE,
                focusX=150f,focusY=420f)
            val shotBall=camera.screenX(contact.first)
            val shaftTarget=camera.screenX(expected.first)
            assertEquals(shotBall,shaftTarget,0.0001f)
        }
    }
}
