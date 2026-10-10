package com.pixtee.golf

import android.content.Intent
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import androidx.test.uiautomator.By
import androidx.test.uiautomator.UiDevice
import androidx.test.uiautomator.Until
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith

/**
 * Runs on an Android emulator, not a JVM: actually taps through the Canvas.
 * A successful APK build alone never qualifies a game as playable.
 */
@RunWith(AndroidJUnit4::class)
class PixteeNavigationSmokeTest {
    private val device = UiDevice.getInstance(InstrumentationRegistry.getInstrumentation())

    @Before
    fun launchFresh() {
        // A fresh API 35 emulator opens the platform ImmersiveModeConfirmation
        // system window over a fullscreen app. It silently swallows menu taps
        // until dismissed, so this is test environment setup, not a game fix.
        device.executeShellCommand("settings put secure immersive_mode_confirmations confirmed")
        device.pressHome()
        val ctx = InstrumentationRegistry.getInstrumentation().targetContext
        val intent = ctx.packageManager.getLaunchIntentForPackage(ctx.packageName)!!
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK)
        ctx.startActivity(intent)
        waitFor("MAIN")
    }

    private fun tap(x: Float, y: Float) {
        // Android 15 and Fold viewport insets can offset a custom View from
        // device (0,0). Always map against the ACTUAL visible canvas bounds.
        val view = device.findObject(By.descContains("Pixtee screen:"))
            ?: error("Pixtee canvas is missing from the Android accessibility tree")
        val bounds = view.visibleBounds
        val scale = bounds.width() / 360f
        val touchX = (bounds.left + x * scale).toInt()
        val touchY = (bounds.top + y * scale).toInt()
        android.util.Log.i("PixteeSmoke", "Tap $x,$y -> $touchX,$touchY; bounds=$bounds; screen=${view.contentDescription}")
        device.click(touchX, touchY)
    }

    // Gameplay controls are bottom anchored and move lower on taller phones.
    // Fixed y=725 misses WHACK on 20:9 displays.
    private fun tapWhack() {
        val view = device.findObject(By.descContains("Pixtee screen:"))
            ?: error("Canvas not visible")
        val bounds = view.visibleBounds
        val logicalHeight = bounds.height() * 360f / bounds.width()
        tap(310f, logicalHeight - 25f)
    }

    private fun waitFor(screen: String, stage: String? = null) {
        val match = if (stage == null) "Pixtee screen: $screen" else
            "Pixtee screen: $screen; stage: $stage"
        val found = device.wait(Until.hasObject(By.descContains(match)), 12000)
        if (!found) {
            val current = device.findObject(By.descContains("Pixtee screen:"))
            val activeWindow = device.currentPackageName
            val actual = current?.contentDescription?.toString() ?: "missing view"
            val bounds = current?.visibleBounds?.toString() ?: "no bounds"
            // UiDevice.executeShellCommand invokes the service without a shell pipe.
            // Filter locally, otherwise Android treats '|' as a dumpsys argument.
            val traces = device.executeShellCommand(
                "logcat -d -t 120 -s PixteeTouch:I PixteeSmoke:I"
            ).takeLast(4500).replace('\n', ';')
            val focus = device.executeShellCommand("dumpsys window")
                .lineSequence()
                .filter { it.contains("mCurrentFocus") || it.contains("mFocusedApp") }
                .take(5)
                .joinToString(" / ")
                .take(800)
            throw AssertionError("Expected [$match], actual [$actual], bounds=[$bounds], package=[$activeWindow]; focus=[$focus]; native-touch-events=[$traces]")
        }
    }

    @Test
    fun mainCoursePlayerTeeAndFirstSwingAreTouchable() {
        tap(180f, 300f); waitFor("COURSES")
        tap(180f, 160f); waitFor("PLAYER")
        tap(180f, 550f); waitFor("TEE")
        tap(180f, 642f); waitFor("PLAYING", "READY")
        tapWhack(); waitFor("PLAYING", "POWER")
        Thread.sleep(540L)
        // Welly accuracy is intentionally a short moving window. UiAutomator's
        // accessibility wait itself can last longer than that window; issue
        // the real second and third taps without waiting for a transient label.
        tapWhack()
        Thread.sleep(100L)
        tapWhack()
        // Separate JVM tests assert the exact READY->POWER->ACCURACY->FLIGHT
        // progression; here Android must accept a real three-tap full shot.
        val match = By.descContains("Pixtee screen: PLAYING; stage: READY; strokes: 1")
        assertTrue("Android three-tap shot did not finish",
            device.wait(Until.hasObject(match), 12000))
    }

    @Test
    fun optionsTogglesAndBackAreTouchable() {
        tap(180f, 640f); waitFor("OPTIONS")
        tap(180f, 445f); waitFor("OPTIONS")
        tap(180f, 525f); waitFor("OPTIONS")
        tap(180f, 692f); waitFor("MAIN")
    }

    @Test
    fun shortFoldLikePortraitScreenScrollsToOffscreenOptions() {
        // A narrowed window reproduces the short usable height encountered
        // in Fold multi-window and some unfolded portrait configurations.
        device.executeShellCommand("wm size 1080x1500")
        try {
            var bounds = device.findObject(By.descContains("Pixtee screen:"))!!.visibleBounds
            repeat(20) {
                if (bounds.height() <= 1550) return@repeat
                Thread.sleep(150L)
                bounds = device.findObject(By.descContains("Pixtee screen:"))!!.visibleBounds
            }
            assertTrue("Emulator did not adopt a compact viewport: $bounds",
                bounds.height() <= 1550)
            val middleX = bounds.left + bounds.width() / 2
            device.swipe(middleX, bounds.top + 1200, middleX, bounds.top + 480, 30)
            waitFor("MAIN")
            // OPTIONS is offscreen before scrolling (logical y 609..658).
            // A 240 logical-unit scroll places it near y 370..420.
            tap(180f, 392f)
            waitFor("OPTIONS")
        } finally {
            device.executeShellCommand("wm size reset")
        }
    }

    @Test
    fun unlockableWardrobeCanBeBrowsedWithoutLeavingGameplay() {
        // Main menu seventh entry uses y=279+6*66=675; fully visible at 800.
        tap(180f,699f)
        waitFor("WARDROBE")
        tap(180f,232f) // hat category
        waitFor("WARDROBE")
        tap(258f,673f) // next equipped unlocked hat
        waitFor("WARDROBE")
        tap(180f,728f) // back
        waitFor("MAIN")
    }

    @Test
    fun secondaryScreensCanBeOpenedAndExited() {
        tap(180f, 368f); waitFor("CAREER")
        tap(180f, 47f); waitFor("MAIN")
        tap(180f, 500f); waitFor("STATS")
        tap(180f, 692f); waitFor("MAIN")
        tap(180f, 569f); waitFor("TROPHIES")
        tap(180f, 47f); waitFor("MAIN")
    }
}
