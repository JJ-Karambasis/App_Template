#ifndef IOS_APP_H
#define IOS_APP_H

#include <app.h>

#import <QuartzCore/QuartzCore.h>

struct ios_context : public app_platform {
    arena Arena;
    app_renderer*  Renderer;
    app*           App;
};

@interface ios_view : UIView
@end

@interface ios_delegate : UIResponder<UIApplicationDelegate>
{
    ios_context    IOSContext;
    CADisplayLink* DisplayLink;
}

@property (strong, nonatomic) UIWindow* Window;
@property (strong, nonatomic) ios_view* View;

@end

#endif