#ifndef ANDROID_TEST_H
#define ANDROID_TEST_H

#include <test.h>
#include <android_native_app_glue.h>

struct android_test_context : public test_context {
    ANativeWindow* NativeWindow;
    bool IsReady;
    bool IsFinished;
};

#endif