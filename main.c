//
//  main.cpp
//  SVAnimCancel
//
//  Created by Alvin on 21.07.23.
//

#include <sys/sysctl.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <libproc.h>
#include <pthread.h>
#include "configManager.h"
#define helpText "This tool is designed to cancel animation for Stardew Valley\n\nThis tool will listen to selected key (Space button by default) and if pressed it will \"press\" R-Shift + R + Delete key\n -h : Prints help text\n -r : Records for keyboards and changes selected key to pressed key\n -e : Resets configuration to default\n --start : Starts executable as a Service\n --stop : Stops service\n --service : This is indicates that executable is service. Use --start instead\n"

pid_t pidofApp = 0;
CGKeyCode selectedKey = 49; //Default key is Space
int captchureSelectedKey = false;

//This code will "press" R-Shift + R + Delete key
void cancelAnimation(void){
    // Simulate pressing R-Shift + R + Delete.
    CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);
    printf("Toggling button\n");
    // Keydown events
    CGEventRef keyEventRShiftDown = CGEventCreateKeyboardEvent(source, kVK_RightShift, true);
    CGEventRef keyEventRDown = CGEventCreateKeyboardEvent(source, kVK_ANSI_R, true);
    CGEventRef keyEventDeleteDown = CGEventCreateKeyboardEvent(source, kVK_ForwardDelete, true);
    
    // Keyup events
    CGEventRef keyEventRShiftUp = CGEventCreateKeyboardEvent(source, kVK_RightShift, false);
    CGEventRef keyEventRUp = CGEventCreateKeyboardEvent(source, kVK_ANSI_R, false);
    CGEventRef keyEventDeleteUp = CGEventCreateKeyboardEvent(source, kVK_ForwardDelete, false);
    
    // Simulate keydown events
    CGEventPost(kCGSessionEventTap, keyEventRShiftDown);
    CGEventPost(kCGSessionEventTap, keyEventRDown);
    CGEventPost(kCGSessionEventTap, keyEventDeleteDown);
    
    // Sleep for a short time to allow the keydown events to register
    usleep(20000); //20 ms

    // Simulate keyup events
    CGEventPost(kCGSessionEventTap, keyEventRShiftUp);
    CGEventPost(kCGSessionEventTap, keyEventRUp);
    CGEventPost(kCGSessionEventTap, keyEventDeleteUp);
    
    // Release event objects
    CFRelease(keyEventRShiftDown);
    CFRelease(keyEventRDown);
    CFRelease(keyEventDeleteDown);
    CFRelease(keyEventRShiftUp);
    CFRelease(keyEventRUp);
    CFRelease(keyEventDeleteUp);
    CFRelease(source);
}

void *cancelAnimationNewThread(void *arg) {
    cancelAnimation();
    return NULL;
}

bool getExecutablePath(char* executablePath, size_t bufferSize) {
    if (executablePath == NULL || bufferSize == 0)
        return false;

    memset(executablePath, 0, bufferSize);

    CFBundleRef mainBundle = CFBundleGetMainBundle();
    if (mainBundle != NULL) {
        CFURLRef executableURL = CFBundleCopyExecutableURL(mainBundle);
        if (executableURL != NULL) {
            if (CFURLGetFileSystemRepresentation(executableURL, true, (UInt8*)executablePath, bufferSize)) {
                CFRelease(executableURL);
                return true;
            } else {
                puts("Buffer size is insufficient to store the path." );
                CFRelease(executableURL);
                return false;
            }
        }
    }
    return executablePath;
}

// The following callback method is invoked on every keypress.
CGEventRef CGEventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {
    if (type != kCGEventKeyDown && type != kCGEventFlagsChanged) {
        return event;
    }
    // Retrieve the incoming keycode.
    CGKeyCode keyCode = (CGKeyCode) CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);

    // Calculate key up/down.
    bool down = false;
    if (type == kCGEventKeyDown) {
        down = true;
    }
    if (keyCode == selectedKey){
        pthread_t thread;
        pthread_create(&thread, NULL, cancelAnimationNewThread, NULL);
    }
    return event;
}

CGEventRef CGEventCallbackForRecording(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {
    if (type != kCGEventKeyDown && type != kCGEventFlagsChanged) {
        return event;
    }
    // Retrieve the incoming keycode.
    CGKeyCode keyCode = (CGKeyCode) CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);

    CFStringRef key = CFSTR("selectedKey");
    CFNumberRef value = CFNumberCreate(NULL, kCFNumberIntType, &keyCode);

    // Set the value for the key in the dictionary
    CFDictionarySetValue(plist, key, value);
    savePlist();
    puts("Trigger button changed and saved");
    exit(0);
}

int main(int argc, const char *argv[]) {
    // Create an event tap to retrieve keypresses.
    CGEventMask eventMask = CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventFlagsChanged);
    CFMachPortRef eventTap;
    if (argc > 1 && !strcmp(argv[1], "-h")) {
        printf(helpText);
        exit(0);
    }
    else if (argc > 1 && !strcmp(argv[1], "-r")) {
        eventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, 0, eventMask, CGEventCallbackForRecording, NULL);
        puts("Press key that want to be trigger button");
    }
    else if (argc > 1 && !strcmp(argv[1], "-e")) {
        puts("Reseting configuration file");
        erasePlist();
        exit(0);
    }
    else if (argc > 1 && !strcmp(argv[1], "--start")) {
        char launchCommand[PATH_MAX + 100];
        char exePath[PATH_MAX];
        if (getExecutablePath(exePath, PATH_MAX)) {
            sprintf(launchCommand, "launchctl submit -l SVAnimCancel %s --service", exePath);
            puts(launchCommand);
            system(launchCommand);
            puts("Starting service");
        }
        else
            puts("Failed to start service");
        exit(0);
    }
    else if (argc > 1 && !strcmp(argv[1], "--stop")) {
        system("launchctl remove SVAnimCancel");
        puts("Stoping service");
        exit(0);
    }
    else if (argc > 1 && !strcmp(argv[1], "--service")) {
        eventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, 0, eventMask, CGEventCallback, NULL);
    }
    else{
        printf(helpText);
        exit(0);
    }
    // Exit the program if unable to create the event tap.
    if (!eventTap) {
        fprintf(stderr, "ERROR: Unable to create event tap.\n");
        CFOptionFlags responseFlags;
        CFUserNotificationDisplayAlert(
                                       0, // no timeout
                                       kCFUserNotificationStopAlertLevel,
                                       NULL, // icon URL
                                       NULL, // sound URL
                                       NULL, // localization URL
                                       CFSTR("Accessibility permissions not granted"),
                                       CFSTR("To use this service, you must grant it accessibility permissions in System Preferences. Some cases app needs to be removed from Privacy -> Accessabilty and relaunched again"),
                                       CFSTR("Quit"),
                                       NULL, // alternate button title
                                       NULL, // other button title
                                       &responseFlags
                                       );
        exit(1);
    }
    initPlist();
    parsePlist();
    
    CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
    CGEventTapEnable(eventTap, true);
    CFRunLoopRun();
    return 0;
}
