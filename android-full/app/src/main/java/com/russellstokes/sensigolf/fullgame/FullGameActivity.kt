package com.russellstokes.sensigolf.fullgame

import android.app.AlertDialog
import android.content.res.Configuration
import android.graphics.Color
import android.os.Bundle
import android.view.Gravity
import android.view.KeyEvent
import android.view.View
import android.view.ViewGroup
import android.view.WindowManager
import android.widget.FrameLayout
import android.widget.ImageView
import androidx.activity.ComponentActivity
import com.swordfish.libretrodroid.GLRetroView
import com.swordfish.libretrodroid.GLRetroViewData
import com.swordfish.libretrodroid.ShaderConfig
import com.swordfish.libretrodroid.Variable
import java.io.File
import java.io.FileOutputStream

/**
 * Dedicated single-game Android shell.
 * Original DOS game logic, menus, courses, graphics, music and sound run in
 * the integrated GPL LibretroDroid/DOSBox Pure modules, NOT in a rewritten game.
 * No DOSBox UI, file picker, display-mode switch or runtime branding is exposed.
 *
 * This source is NOT evidence that original rights have been cleared, all
 * game screens work or an actual device playthrough has passed.
 */
class FullGameActivity : ComponentActivity() {
    private lateinit var gameView: GLRetroView
    private var initialized = false
    private var pausedByMenu = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        window.navigationBarColor = Color.BLACK
        window.statusBarColor = Color.BLACK
        enableFullScreen()

        // Licensed game archive is committed nowhere in the public repository.
        // It is injected into the APK only during a rights-approved build.
        val archive = File(filesDir, "SensibleGolf.dosz")
        if (!archive.isFile || archive.length() == 0L) {
            val packaged = assets.open("game/SensibleGolf.dosz")
            FileOutputStream(archive).use { dst -> packaged.use { src -> src.copyTo(dst) } }
        }

        val config = GLRetroViewData(this).apply {
            coreFilePath = "libdosbox_pure_libretro_android.so"
            gameFilePath = archive.absolutePath
            systemDirectory = filesDir.absolutePath
            savesDirectory = File(filesDir, "saves").apply { mkdirs() }.absolutePath

            // Single fixed graphics path, preserving original 4:3/asset aspect.
            // Gentle hardware bilinear smoothing: no user-facing graphics mode.
            shader = ShaderConfig.Default

            // Internal config. Never show core menus, keyboard or emulation UI.
            variables = arrayOf(
                Variable("dosbox_pure_mouse_input", "pad"),
                Variable("dosbox_pure_on_screen_keyboard", "false"),
                Variable("dosbox_pure_menu_time", "0"),
                Variable("dosbox_pure_auto_mapping", "false")
            )
            rumbleEventsEnabled = false
            preferLowLatencyAudio = true
        }

        gameView = GLRetroView(this, config)
        lifecycle.addObserver(gameView)
        val frame = FrameLayout(this)
        frame.setBackgroundColor(Color.BLACK)
        frame.addView(
            gameView,
            FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT,
                Gravity.CENTER
            )
        )
        // Branded launch overlay masks all internal runtime initialization.
        // The original game and its own menu are the first thing the player sees.
        val splash = FrameLayout(this).apply { setBackgroundColor(Color.rgb(15, 34, 30)) }
        val logo = ImageView(this).apply {
            setImageResource(R.drawable.approved_eagle)
            scaleType = ImageView.ScaleType.FIT_CENTER
            contentDescription = "Sensible Golf"
        }
        val iconSize = (144 * resources.displayMetrics.density).toInt()
        splash.addView(logo, FrameLayout.LayoutParams(iconSize, iconSize, Gravity.CENTER))
        frame.addView(splash, FrameLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT
        ))
        setContentView(frame)
        // Runtime screen capture validation must confirm the core has reached the
        // actual game before uncovering it; this delay alone is NOT that proof.
        splash.postDelayed({
            splash.animate().alpha(0f).setDuration(250).withEndAction {
                frame.removeView(splash)
            }.start()
        }, 3200L)
        initialized = true
    }

    private fun enableFullScreen() {
        @Suppress("DEPRECATION")
        window.decorView.systemUiVisibility =
            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY or
            View.SYSTEM_UI_FLAG_FULLSCREEN or
            View.SYSTEM_UI_FLAG_HIDE_NAVIGATION or
            View.SYSTEM_UI_FLAG_LAYOUT_STABLE or
            View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN or
            View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) enableFullScreen()
    }

    override fun onConfigurationChanged(newConfig: Configuration) {
        super.onConfigurationChanged(newConfig)
        enableFullScreen()
        // Same instance + same game session survive Fold open/close/rotation.
        if (initialized) gameView.requestLayout()
    }

    @Deprecated("Back action is a game pause dialog, never the emulator menu.")
    override fun onBackPressed() {
        if (!initialized || pausedByMenu) return
        pausedByMenu = true
        gameView.onPause()
        AlertDialog.Builder(this)
            .setTitle("Paused")
            .setItems(arrayOf("Resume game", "Restart game", "Exit")) { dialog, index ->
                when (index) {
                    0 -> gameView.onResume()
                    1 -> { gameView.reset(); gameView.onResume() }
                    2 -> { finish(); return@setItems }
                }
                pausedByMenu = false
                enableFullScreen()
            }
            .setOnCancelListener {
                pausedByMenu = false
                gameView.onResume()
                enableFullScreen()
            }
            .show()
    }

    override fun onKeyDown(keyCode: Int, event: KeyEvent): Boolean {
        // Let the core handle its genuine original keyboard/gamepad mapping.
        if (keyCode == KeyEvent.KEYCODE_BACK ||
            keyCode == KeyEvent.KEYCODE_ESCAPE) {
            onBackPressed()
            return true
        }
        return super.onKeyDown(keyCode, event)
    }
}
