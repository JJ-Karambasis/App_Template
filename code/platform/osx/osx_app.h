#ifndef OSX_APP_H
#define OSX_APP_H

#include <app.h>

#import <QuartzCore/QuartzCore.h>

struct osx_context : public app_platform {
    arena Arena;
};

@interface osx_window_delegate : NSObject<NSWindowDelegate>

    @property (nonatomic, assign) NSWindow* mainWindow;
    @property BOOL isMainWindowClosed;

@end

#endif