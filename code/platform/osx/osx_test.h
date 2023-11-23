#ifndef OSX_TEST_H
#define OSX_TEST_H

#include <test.h>

#import <QuartzCore/QuartzCore.h>

struct osx_test_context : public test_context {
    NSWindow* Window;
};

#endif