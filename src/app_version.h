/******************************************************************************\
Copyright (c) 2017, Shaun Simpson
All rights reserved.

\**********************************************************************************/

#pragma once

#ifndef APP_MAJOR
#define APP_MAJOR 1
#endif

#ifndef APP_MINOR
#define APP_MINOR 1
#endif

#ifndef APP_RELEASE
#define APP_RELEASE 3
#endif

static msdk_string GetAppVersion()
{
    msdk_stringstream ss;
    ss << APP_MAJOR << "." << APP_MINOR << "." << APP_RELEASE;
    return ss.str();
}