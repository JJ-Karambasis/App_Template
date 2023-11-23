#include "ios_test.h"

@implementation ios_view

+ (Class)layerClass
{
    return [CAMetalLayer class];
}

@end

@implementation ios_delegate

- (void)doFrame:(CADisplayLink *)Sender {
    int Code = RUN_ALL_TESTS();
    exit(Code);
}

- (BOOL)application:(UIApplication *)Application willFinishLaunchingWithOptions:(NSDictionary *)LaunchOptions {
    core::Create();

    TestContext = {};

    self.Window = [[UIWindow alloc] init];
    self.Window.frame = [[UIScreen mainScreen] bounds];

    self.View = [[ios_view alloc] initWithFrame:self.Window.frame];

    [self.Window addSubview: self.View];
    self.Window.rootViewController = [[UIViewController alloc] init];
    [self.Window makeKeyAndVisible];

    TestContext.View = self.View;

    DisplayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(doFrame:)];
    DisplayLink.preferredFramesPerSecond = 60;
    [DisplayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];

    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)Application {
	// Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
	// Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)Application {
	// Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
	// If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)Application {
	// Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)Application {
	// Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)Application {
	// Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

@end

int main(int ArgCount, char* Args[]) {
    testing::InitGoogleTest(&ArgCount, Args);

    @autoreleasepool {
        return UIApplicationMain(ArgCount, Args, nil, NSStringFromClass([ios_delegate class]));
    }
}

#include <test.cpp>
