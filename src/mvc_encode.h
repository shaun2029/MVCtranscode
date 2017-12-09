/******************************************************************************\
MVCTranscode Copyright (c) 2017, Shaun Simpson

Copyright (c) 2005-2017, Intel Corporation
All rights reserved.
/******************************************************************************/

#ifndef __BLURAY_ENCODE_H__
#define __BLURAY_ENCODE_H__

void PrintHelp(msdk_char *strAppName, const msdk_char *strErrorMessage);
mfxStatus SetupEncoder(int argc, msdk_char *argv[], int argPos, mfxFrameInfo *pFrameInfo, CEncodingPipeline*& pPipeline, CFrameFifo *pFrameFifo);

#endif
