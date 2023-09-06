# Requirements

## Git LFS

```bash
$ sudo apt install git-lfs
```

## Android NDK

1. Go to https://developer.android.com/ndk/downloads and download r25c NDK
2. Set `ANDROID_NDK_HOME` on your system such that `${ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake` is valid
3. Install adb `sudo apt install adb`

# x86

## Building

```bash
$ cmake --preset default
$ cmake --build --preset default
```

## Running

Make sure LFS files are downloaded

```bash
$ ./build/default/bin/libraw_stripes -i rggb.raw --log debug -d 10 -b 64 -p rggb -o rggb.png
$ ./build/default/bin/libraw_stripes -i rggb.raw --log debug -d 10 -b 64 -p bggr -o bggr.png
$ ./build/default/bin/libraw_stripes -i rggb.raw --log debug -d 10 -b 64 -p gbrg -o gbrg.png
$ ./build/default/bin/libraw_stripes -i rggb.raw --log debug -d 10 -b 64 -p grbg -o grbg.png
```

Those generated images are in `output/x86`.

# Android

## Building

```bash
$ cmake --preset default
$ cmake --build --preset default
```

## Running (using adb)

1. Connect to an android device using adb `adb shell`
2. Push resources to the device

```bash
$ adb push build/android/libraw_stripes /data/local/tmp
$ adb push rggb.raw /data/local/tmp/libraw
$ adb push ${ANDROID_NDK_HOME}/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/aarch64-linux-android/libc++_shared.so /data/local/tmp/libraw
```

3. Connect to the device and run

```bash
$ adb shell
$ cd /data/local/tmp
$ LD_LIBRARY_PATH=. ./libraw_stripes -i rggb.raw --log debug -d 10 -b 64 -p rggb -o rggb.png
$ LD_LIBRARY_PATH=. ./libraw_stripes -i rggb.raw --log debug -d 10 -b 64 -p bggr -o bggr.png
$ LD_LIBRARY_PATH=. ./libraw_stripes -i rggb.raw --log debug -d 10 -b 64 -p gbrg -o gbrg.png
$ LD_LIBRARY_PATH=. ./libraw_stripes -i rggb.raw --log debug -d 10 -b 64 -p grbg -o grbg.png
```

Those generated images are in `output/arm64`.

