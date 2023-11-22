#include "android_test.h"
#include "android_shared.cpp"

static s32 Android_Event_Proc(android_app* App, AInputEvent* Event) {
    return 0;
}

static void Android_On_App_Cmd(android_app* App, s32 Cmd) {
    android_test_context* Context = (android_test_context*)App->userData;

    switch(Cmd) {
        case APP_CMD_INIT_WINDOW: {
            if(App->window != nullptr) {
                Context->NativeWindow = App->window;
                Context->IsReady = true;
            }
        } break;
    }
}

static int pfd[2];
static pthread_t thr;

static void* Android_Thread_Func(void*)
{
    ssize_t rdsz;
    char buf[1024];
    while((rdsz = read(pfd[0], buf, sizeof buf - 1)) > 0) {
        if(buf[rdsz - 1] == '\n') --rdsz;
        buf[rdsz] = 0;  /* add null-terminator */
        __android_log_write(ANDROID_LOG_DEBUG, "stdout", buf);
    }
    return 0;
}

static int Android_Start_Logger() {
    /* make stdout line-buffered and stderr unbuffered */
    setvbuf(stdout, 0, _IOLBF, 0);
    setvbuf(stderr, 0, _IONBF, 0);

    /* create the pipe and redirect stdout and stderr */
    pipe(pfd);
    dup2(pfd[1], 1);
    dup2(pfd[1], 2);

    /* spawn the logging thread */
    if(pthread_create(&thr, 0, Android_Thread_Func, 0) == -1)
        return -1;
    pthread_detach(thr);
    return 0;
}

void android_main(struct android_app* App) {
    android_test_context TestContext = {};

    Android_Start_Logger();
    
    App->userData = &TestContext;
    App->onAppCmd = Android_On_App_Cmd;
    App->onInputEvent = Android_Event_Proc;

    for(;;) {
        if(!Android_Poll_App(App)) {
            //End app
            return;
        }

        if(TestContext.IsReady && !TestContext.IsFinished) {
            testing::InitGoogleTest();
            RUN_ALL_TESTS();
            ANativeActivity_finish(App->activity);
            TestContext.IsFinished = true;
        }
    }
}

#include <test.cpp>