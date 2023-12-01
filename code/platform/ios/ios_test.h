#ifndef IOS_TEST_H
#define IOS_TEST_H

#include <test.h>

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

struct ios_test_context : public test_context {
    UIView* View;
};

@interface ios_view : UIView
@end

@interface ios_delegate : UIResponder<UIApplicationDelegate>
{
    ios_test_context TestContext;
    CADisplayLink* DisplayLink;
}

@property (strong, nonatomic) UIWindow* Window;
@property (strong, nonatomic) ios_view* View;

@end

#endif