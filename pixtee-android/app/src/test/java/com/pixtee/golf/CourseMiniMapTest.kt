package com.pixtee.golf

import org.junit.Assert.*
import org.junit.Test

class CourseMiniMapTest {
    @Test fun projectedCourseMarkersMatchTruePlayableCoordinates() {
        val map=CourseMiniMap()
        // Subpixel inverse projections pass through a 31px-wide map, so
        // Float quantization in source world units merits 0.001 tolerance.
        for(course in listOf(0,6,19,34)) for(number in listOf(1,7,18)) {
            val hole=PixteeCourseCatalog.hole(course,number)
            val (gx,gy)=map.green(hole)
            val (tx,ty)=map.tee(hole)
            assertEquals(hole.pinX,map.originalX(gx),.001f)
            assertEquals(hole.pinY,map.originalY(gy),.001f)
            assertEquals(hole.teeX,map.originalX(tx),.001f)
            assertEquals(hole.teeY,map.originalY(ty),.001f)
            assertTrue("Pin and tee should be visually distinct",gy<ty)
            assertEquals(hole.waters.size,map.waterPatches(hole).size)
        }
    }

    @Test fun mapCentrelineFollowsActualFairwayCurveNotStockDrawing() {
        val map=CourseMiniMap()
        val hole=PixteeCourseCatalog.hole(32,13)
        val samples=map.centreline(hole,37)
        assertEquals(37,samples.size)
        assertEquals(map.sy(hole.pinY),samples.first().second,.0001f)
        assertEquals(map.sy(hole.teeY),samples.last().second,.0001f)
        for ((i,coords) in samples.withIndex()) {
            val y=hole.pinY+(hole.teeY-hole.pinY)*i/36f
            assertEquals(map.sx(hole.fairwayCentre(y)),coords.first,.0001f)
        }
    }

    @Test fun minimapNeverAffectsBallShotCourseOrPhysicsState() {
        val g=PixteeCore()
        val hole=PixteeCourseCatalog.hole(9,8)
        g.startHole(hole)
        val before=g.stableBall()
        val mini=CourseMiniMap()
        mini.centreline(hole)
        mini.green(hole)
        mini.tee(hole)
        mini.waterPatches(hole)
        assertEquals(before,g.stableBall())
        assertEquals(Ground.GREEN,g.groundAt(hole.pinX,hole.pinY))
    }
}
