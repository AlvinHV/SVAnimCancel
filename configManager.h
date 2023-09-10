//
//  configManager.h
//  SVAnimCancel
//
//  Created by Alvin on 21.07.23.
//

#ifndef configManager_h
#define configManager_h

#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>

extern CGKeyCode selectedKey; //Default key is V
extern CFMutableDictionaryRef plist;
void erasePlist(void);
void initPlist(void);
void savePlist(void);
void parsePlist(void);

#endif /* configManager_h */
