#!/usr/bin/env python3
"""Bridge Android physical/hosted keys to the actual DOSBox Pure keyboard.

Pinned LibretroDroid 8835c309 only converts Android keydowns into libretro
JOYPAD ids. It *ignores* AKEYCODE_SPACE, ARROWS, CTRL, ESC, ENTER and letters;
its Environment also does not register RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK.
DOSBox Pure offers this callback. Without both bridges HUMAN 1 / Keyboard
can reach a tee but cannot swing or aim, even with hardware buttons.

Patch only the local pinned GPL library at build time, never the original
Sensible Golf binaries/physics. Fail closed if upstream anchors change.
"""
from pathlib import Path
import argparse


def inject_once(source: str, anchor: str, addition: str) -> tuple[str,bool]:
    if addition in source:
        return source,False
    if source.count(anchor)!=1:
        raise ValueError(f"Pinned LibretroDroid anchor changed: {anchor!r}")
    return source.replace(anchor,addition+anchor,1),True


def patch_header(source: str) -> str:
    declaration="""    // Registered by libretro Environment when the DOS core requests keys.
    using KeyboardEvent = void (*)(bool, unsigned, uint32_t, uint16_t);
    static void setKeyboardCallback(KeyboardEvent callback);

"""
    return inject_once(source,"    int16_t getInputState(",declaration)[0]


def patch_input(source: str) -> str:
    storage="""static Input::KeyboardEvent originalDosKeyboardCallback = nullptr;

void Input::setKeyboardCallback(KeyboardEvent callback) {
    originalDosKeyboardCallback = callback;
}

"""
    source=inject_once(source,"int16_t Input::getInputState(",storage)[0]
    keymapping="""    // Actual DOS KEYBOARD events, in addition to existing libretro JOYPAD
    // mapping. The original game requires these for a human player's swing.
    unsigned dosKey=0;
    switch (keyCode) {
        case AKEYCODE_DPAD_UP: dosKey=RETROK_UP; break;
        case AKEYCODE_DPAD_DOWN: dosKey=RETROK_DOWN; break;
        case AKEYCODE_DPAD_LEFT: dosKey=RETROK_LEFT; break;
        case AKEYCODE_DPAD_RIGHT: dosKey=RETROK_RIGHT; break;
        case AKEYCODE_SPACE: dosKey=RETROK_SPACE; break;
        case AKEYCODE_CTRL_LEFT: dosKey=RETROK_LCTRL; break;
        case AKEYCODE_CTRL_RIGHT: dosKey=RETROK_RCTRL; break;
        case AKEYCODE_ENTER: dosKey=RETROK_RETURN; break;
        case AKEYCODE_ESCAPE: dosKey=RETROK_ESCAPE; break;
        case AKEYCODE_TAB: dosKey=RETROK_TAB; break;
        case AKEYCODE_DEL: dosKey=RETROK_BACKSPACE; break;
        case AKEYCODE_SHIFT_LEFT: dosKey=RETROK_LSHIFT; break;
        case AKEYCODE_SHIFT_RIGHT: dosKey=RETROK_RSHIFT; break;
    }
    if (!dosKey && keyCode >= AKEYCODE_A && keyCode <= AKEYCODE_Z) {
        dosKey=RETROK_a+(keyCode-AKEYCODE_A);
    }
    if (!dosKey && keyCode >= AKEYCODE_0 && keyCode <= AKEYCODE_9) {
        dosKey=RETROK_0+(keyCode-AKEYCODE_0);
    }
    if (port == 0 && originalDosKeyboardCallback && dosKey) {
        originalDosKeyboardCallback(action == AKEY_EVENT_ACTION_DOWN,
                                   dosKey, 0, 0);
    }
"""
    anchor="void Input::onKeyEvent(unsigned int port, int action, int keyCode) {\n"
    return inject_once(source,anchor,anchor+keymapping)[0].replace(
        anchor+anchor+keymapping,anchor+keymapping,1)


def patch_environment(source: str) -> str:
    source=inject_once(source,'#include "environment.h"\n','#include "input.h"\n')[0]
    environment_case="""        case RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK: {
            const auto* keyboard = static_cast<const struct retro_keyboard_callback*>(data);
            libretrodroid::Input::setKeyboardCallback(keyboard ? keyboard->callback : nullptr);
            return true;
        }

"""
    source=inject_once(source,"        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:",environment_case)[0]
    source=inject_once(source,"void Environment::deinitialize() {\n",
                       "void Environment::deinitialize() {\n    libretrodroid::Input::setKeyboardCallback(nullptr);\n")[0]
    return source.replace(
        "void Environment::deinitialize() {\nvoid Environment::deinitialize() {\n",
        "void Environment::deinitialize() {\n",1)


def main() -> None:
    parser=argparse.ArgumentParser()
    parser.add_argument("root",type=Path,nargs="?",default=Path("android-full/vendor/LibretroDroid/libretrodroid/src/main/cpp"))
    args=parser.parse_args()
    files={
       "input.h":patch_header,
       "input.cpp":patch_input,
       "environment.cpp":patch_environment,
    }
    new={}
    for filename,fn in files.items():
        source=(args.root/filename).read_text()
        new[filename]=fn(source)
    for filename,patched in new.items():
        target=args.root/filename
        if patched!=target.read_text():
            target.write_text(patched)
        print(f"Original DOS keyboard bridge OK: {filename}")


if __name__=="__main__":
    main()
