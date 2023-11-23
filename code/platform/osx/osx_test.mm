#include "osx_test.h"
#include "osx_shared.mm"

int main(int ArgCount, char* Args[]) {
    @autoreleasepool {
        core::Create();

        osx_test_context TestContext = {};
        TestContext.Window = OSX_Create_Window(1920, 1080, @"App_Test", nil);

        testing::InitGoogleTest(&ArgCount, Args);
        return RUN_ALL_TESTS();
    }
}

#include <test.cpp>