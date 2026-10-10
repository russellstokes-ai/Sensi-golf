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

    private fun waitFor(screen: String, stage: String? = null) {
        val match = if (stage == null) "Pixtee screen: $screen" else
            "Pixtee screen: $screen; stage: $stage"
        val found = device.wait(Until.hasObject(By.descContains(match)), 12000)
        if (!found) {
            val current = device.findObject(By.descContains("Pixtee screen:"))
            android.util.Log.e("PixteeSmoke", "EXPECTED $match; ACTUAL ${current?.contentDescription}; bounds=${current?.visibleBounds}")
        }
        assertTrue("Timed out waiting for $match", found)
    }

    @Test
    fun mainCoursePlayerTeeAndFirstSwingAreTouchable() {
        tap(180f, 300f); waitFor("COURSES")
        tap(180f, 160f); waitFor("PLAYER")
        tap(180f, 550f); waitFor("TEE")
        tap(180f, 642f); waitFor("PLAYING", "READY")
        tap(310f, 725f); waitFor("PLAYING", "POWER")
        Thread.sleep(650L)
        tap(310f, 725f); waitFor("PLAYING", "ACCURACY")
        Thread.sleep(300L)
        tap(310f, 725f); waitFor("PLAYING", "FLIGHT")
        // A full human-controlled stroke must make it back to READY or HOLED.
        val match = By.descContains("Pixtee screen: PLAYING; stage: READY; strokes: 1")
        assertTrue("Shot did not finish", device.wait(Until.hasObject(match), 12000))
    }

    @Test
    fun optionsTogglesAndBackAreTouchable() {
        tap(180f, 640f); waitFor("OPTIONS")
        tap(180f, 445f); waitFor("OPTIONS")
        tap(180f, 525f); waitFor("OPTIONS")
        tap(180f, 692f); waitFor("MAIN")
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
