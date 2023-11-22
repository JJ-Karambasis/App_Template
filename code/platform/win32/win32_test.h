#ifndef WIN32_TEST_H
#define WIN32_TEST_H

#include <test.h>

struct win32_test_context : public test_context {
    HWND Window;
};

#endif