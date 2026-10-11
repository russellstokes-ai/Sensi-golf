package com.pixtee.golf

import org.junit.Assert.*
import org.junit.Test

class MenuInputTest {
    @Test fun shortTapMapsToSameButton() {
        val m = MenuInput()
        m.down(190f, 310f)
        assertEquals(190f to 310f, m.release(190f, 310f))
    }

    @Test fun scrollMakesBottomButtonsReachableOnShortFoldViewport() {
        val m = MenuInput()
        m.down(180f, 400f)
        m.move(180f, 120f, MenuInput.maxScroll(500f))
        assertNull(m.release(180f, 120f)) // never accidentally click during scroll
        assertEquals(260f, m.scrollY)
        m.down(180f, 424f)
        assertEquals(180f to 684f, m.release(180f, 424f))
        assertTrue(MenuInput.contains(180f, 684f, 75f, 677f, 285f, 723f))
    }

    @Test fun noScrollOnTallPhoneAndTapVsDrag() {
        val m = MenuInput()
        m.down(180f, 400f)
        m.move(180f, 310f, MenuInput.maxScroll(820f))
        assertEquals(0f, m.scrollY)
        assertNull(m.release(180f, 310f))
        m.down(180f, 400f)
        m.move(183f, 404f, 0f)
        assertEquals(183f to 404f, m.release(183f, 404f))
    }

    @Test fun newScreenClearsScroll() {
        val m = MenuInput()
        m.down(120f, 350f); m.move(120f, 190f, 260f); m.release(120f, 190f)
        m.reset()
        assertEquals(0f, m.scrollY)
        m.down(50f, 40f)
        assertEquals(50f to 40f, m.release(50f, 40f))
    }
}
