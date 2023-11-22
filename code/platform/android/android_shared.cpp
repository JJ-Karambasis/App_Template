static bool Android_Poll_App(android_app* App) {
    int EventID;
    int Events;

    android_poll_source* Source;

    while((EventID = ALooper_pollAll(0, nullptr, &Events, (void**)&Source)) >= 0) {
        if(Source != nullptr) {
            Source->process(App, Source);
        }

        if(App->destroyRequested != 0) {
            //End app
            return false;
        }
    }

    return true;
}