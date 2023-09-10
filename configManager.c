//
//  configManager.c
//  SVAnimCancel
//  This code is used to handle saving configuration on plist
//  Created by Alvin on 21.07.23.
//

#include "configManager.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#define configPath "%@/.config/StardewValley/AnimCancel.plist"
#define configFolderPath "%@/.config/StardewValley/"

char filePath[PATH_MAX];
//Holds dictionary for config
CFMutableDictionaryRef plist;

void erasePlist(void) {
    initPlist();
    if(remove(filePath))
        puts("Clearing configuration failed");
    initPlist();
}

void savePlist(void) {
    CFDataRef xmlData = CFPropertyListCreateData(kCFAllocatorDefault, plist, kCFPropertyListXMLFormat_v1_0, 0, NULL);
    if (xmlData) {
        FILE* file = fopen(filePath, "wb");
        if (file) {
            fwrite(CFDataGetBytePtr(xmlData), sizeof(UInt8), CFDataGetLength(xmlData), file);
            fclose(file);
        } else {
            printf("Error creating plist file.\n");
        }
        CFRelease(xmlData);
    }
    
}

void parsePlist(void) {
    if (!plist)
        plist = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    CFNumberRef selectedKeyNum = CFDictionaryGetValue(plist, CFSTR("selectedKey"));
    //if key exist
    if (selectedKeyNum && CFGetTypeID(selectedKeyNum) == CFNumberGetTypeID()) {
        CFNumberGetValue(selectedKeyNum, kCFNumberIntType, &selectedKey);
        printf("Selected Key: %d\n", selectedKey);
    //if key dont exist create one
    } else {
        // Age key is missing or not a number; create a default one
        CFNumberRef defaultKey = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &selectedKey);
        CFDictionarySetValue(plist, CFSTR("selectedKey"), defaultKey);
        CFRelease(defaultKey);
    }
}

void initPlist(void) {
    
       const char* homeDirCStr = getenv("HOME");
       
       CFStringRef homeDir = CFStringCreateWithCString(kCFAllocatorDefault, homeDirCStr, kCFStringEncodingUTF8);
       CFStringRef configDirPath = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR(configFolderPath), homeDir);
       CFStringRef plistPath = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR(configPath), homeDir);

        //Check if StardewValley File path exist
       if (configDirPath) {
           char configDir[PATH_MAX];
           if (CFStringGetCString(configDirPath, configDir, sizeof(configDir), kCFStringEncodingUTF8)) {
               if (access(configDir, F_OK) == -1) {
                   printf("StardewValley folder doesnt exist in ~/.config folder. Are you sure that you installed the game? \n");
                   exit(1);
               }
           }
       }
        CFRelease(homeDir);
        CFRelease(configDirPath);

       if (plistPath) {
           CFURLRef fileURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, plistPath, kCFURLPOSIXPathStyle, false);
           CFRelease(plistPath);

           if (fileURL) {
                CFURLGetFileSystemRepresentation(fileURL, true, (UInt8*)filePath, PATH_MAX);
                   // Check if the file exists before proceeding
                   if (access(filePath, F_OK) != -1) {
                       CFDataRef data = NULL;
                       SInt32 errorCode = 0;
                       CFDataRef resourceData = NULL;
                       Boolean status = CFURLCreateDataAndPropertiesFromResource(kCFAllocatorDefault, fileURL, &resourceData, NULL, NULL, &errorCode);
                       //If true configuration already exist
                       if (status && resourceData) {
                           data = resourceData;
                           CFErrorRef parseErrorString = NULL;
                           plist = (CFMutableDictionaryRef)CFPropertyListCreateWithData(kCFAllocatorDefault, data, kCFPropertyListMutableContainersAndLeaves, NULL, &parseErrorString);

                           if (!plist) {
                               CFStringRef errorDescription = CFErrorCopyDescription(parseErrorString);
                               char errorCStr[1024]; // Buffer to hold the C-string

                               if (CFStringGetCString(errorDescription, errorCStr, sizeof(errorCStr), kCFStringEncodingUTF8)) {
                                   printf("Error parsing plist: %s\n", errorCStr);
                               } else {
                                   printf("Error parsing plist: Unknown error\n");
                               }
                               CFRelease(errorDescription);
                               CFRelease(parseErrorString);
                           }
                           CFRelease(data);

                       } else {
                           printf("Error reading plist file.\n");
                       }
                   } else {
                       printf("The file does not exist. Creating a new plist file with default values.\n");
                       plist = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
                       parsePlist();
                       savePlist();
               }
               
           }
           CFRelease(fileURL);
       }
}
