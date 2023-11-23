static NSWindow* OSX_Create_Window(u32 Width, u32 Height, NSString* Label, id<NSWindowDelegate> WindowDelegate) {
    NSRect ScreenRect = [[NSScreen mainScreen] frame];

    //Center the window on the screen
    NSRect InitialFrame = NSMakeRect((ScreenRect.size.width-Width)*0.5f,
                                     (ScreenRect.size.height-Height)*0.5f,
                                     (f32)Width, (f32)Height);

    NSWindow* Window = [[NSWindow alloc] initWithContentRect: InitialFrame
                                         styleMask: NSWindowStyleMaskTitled |
                                                    NSWindowStyleMaskClosable |
                                                    NSWindowStyleMaskResizable
                                                    backing: NSBackingStoreBuffered
                                         defer: NO];
    
    [Window setBackgroundColor: NSColor.blackColor];
    [Window setTitle: Label];
    [Window makeKeyAndOrderFront: nil];
    [Window setDelegate: WindowDelegate];
    
    NSView* View = [Window contentView];

    [View setWantsLayer:YES];
    [View setLayer:[CAMetalLayer layer]];

    return Window;
}
