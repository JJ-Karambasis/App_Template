#ifndef WIN32_APP_H
#define WIN32_APP_H

#include <app.h>

struct win32_context : public app_platform {
    arena Arena;
};

#endif