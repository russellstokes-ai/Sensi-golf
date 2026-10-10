package com.russellstokes.sensigolf;

final class NativeBridge {
    static { System.loadLibrary("sensigolf_android"); }
    private NativeBridge() {}

    static native void nativeLoad(String originalResourcesDir);
    static native String nativeSnapshot();
    static native String nativeStatus();
    static native int[] nativePixels();
    static native void nativeStep(int ticks);
    static native void nativeAim(int delta);
    static native void nativeClub(int delta);
    static native void nativeClick(int reading);
    static native void nativeCancel();
    static native void nativeReset();
}
