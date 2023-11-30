#ifndef WIN32_APP_H
#define WIN32_APP_H

#include <app.h>
#include <mmreg.h>
#include <dsound.h>

#define WIN32_SOUND_BYTES_PER_SAMPLE 2
#define WIN32_SOUND_HZ 1.0/30.0
#define WIN32_SOUND_MAX_LATENCY 10

struct win32_dsound {
    HMODULE             Library;
    IDirectSound*       Context;
    IDirectSoundBuffer* Buffer;
};

struct win32_context : public app_platform {
    arena         Arena;
    win32_dsound* DSound;
    app*          App;
};

typedef HRESULT WINAPI direct_sound_create(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN  pUnkOuter);

#endif