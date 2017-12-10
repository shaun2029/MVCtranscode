/******************************************************************************\
MVCTranscode Copyright (c) 2017, Shaun Simpson

Copyright (c) 2005-2017, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This sample was distributed or derived from the Intel's Media Samples package.
The original version of this sample may be obtained from https://software.intel.com/en-us/intel-media-server-studio
or https://software.intel.com/en-us/media-client-solutions-support.
\**********************************************************************************/

#include "mfx_samples_config.h"

#include "pipeline_decode.h"
#include "pipeline_encode.h"
#include "mvc_encode.h"
#include <sstream>

mfxStatus ParseInputString(msdk_char* strInput[], mfxU8 nArgNum, int *nArgPos, sInputParams* pParams)
{
	bool complete = false;

    if (1 == nArgNum)
    {
        PrintHelp(strInput[0], NULL);
        exit(1);
    }

    MSDK_CHECK_POINTER(pParams, MFX_ERR_NULL_PTR);

    // set default implementation
	pParams->bUseHWLib = false;
    pParams->bUseFullColorRange = false;
	pParams->bDots = false;
	pParams->numViews = 0;

#if defined(LIBVA_SUPPORT)
    pParams->libvaBackend = MFX_LIBVA_DRM;
#endif

    for (mfxU8 i = 1; i < nArgNum; i++)
    {
		if ((pParams->numViews > 0) && (0 != msdk_strcmp(strInput[i], MSDK_STRING("-i")))) {
			complete = true;
		}

		*nArgPos = i;

		if (complete) {
			break;
		}


        if (MSDK_CHAR('-') != strInput[i][0])
        {
            mfxStatus sts = StrFormatToCodecFormatFourCC(strInput[i], pParams->videoType);
            if (sts != MFX_ERR_NONE)
            {
                PrintHelp(strInput[0], MSDK_STRING("Unknown codec"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (!IsDecodeCodecSupported(pParams->videoType))
            {
                PrintHelp(strInput[0], MSDK_STRING("Unsupported codec"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (pParams->videoType == CODEC_MVC)
            {
                pParams->videoType = MFX_CODEC_AVC;
                pParams->bIsMVC = true;
            }
            continue;
        }

        if ((0 == msdk_strcmp(strInput[i], MSDK_STRING("-help")))
			|| (0 == msdk_strcmp(strInput[i], MSDK_STRING("--help")))
			|| (0 == msdk_strcmp(strInput[i], MSDK_STRING("-h")))
			|| (0 == msdk_strcmp(strInput[i], MSDK_STRING("/h")))
			|| (0 == msdk_strcmp(strInput[i], MSDK_STRING("/?")))
			|| (0 == msdk_strcmp(strInput[i], MSDK_STRING("-?"))))
        {
            PrintHelp(strInput[0], NULL);
			exit(0);
        }
		else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-sw")))
        {
            pParams->bUseHWLib = false;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-hw")))
        {
            pParams->bUseHWLib = true;
        }
#if D3D_SURFACES_SUPPORT
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-d3d")))
        {
            pParams->memType = D3D9_MEMORY;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-d3d11")))
        {
            pParams->memType = D3D11_MEMORY;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-r")))
        {
            pParams->mode = MODE_RENDERING;
            // use d3d9 rendering by default
            if (SYSTEM_MEMORY == pParams->memType)
                pParams->memType = D3D9_MEMORY;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-wall")))
        {
            if(i + 6 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -wall key"));
                return MFX_ERR_UNSUPPORTED;
            }
            // use d3d9 rendering by default
            if (SYSTEM_MEMORY == pParams->memType)
                pParams->memType = D3D9_MEMORY;

            pParams->mode = MODE_RENDERING;

            msdk_opt_read(strInput[++i], pParams->nWallW);
            msdk_opt_read(strInput[++i], pParams->nWallH);
            msdk_opt_read(strInput[++i], pParams->nWallCell);
            msdk_opt_read(strInput[++i], pParams->nWallMonitor);

            mfxU32 nTitle;
            msdk_opt_read(strInput[++i], nTitle);

            pParams->bWallNoTitle = 0 == nTitle;

            msdk_opt_read(strInput[++i], pParams->nTimeout);
        }
#endif
#if defined(LIBVA_SUPPORT)
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-vaapi")))
        {
            pParams->memType = D3D9_MEMORY;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-r")))
        {
            pParams->memType = D3D9_MEMORY;
            pParams->mode = MODE_RENDERING;
            pParams->libvaBackend = MFX_LIBVA_X11;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-rwld")))
        {
            pParams->memType = D3D9_MEMORY;
            pParams->mode = MODE_RENDERING;
            pParams->libvaBackend = MFX_LIBVA_WAYLAND;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-perf")))
        {
            pParams->bPerfMode = true;
        }
        else if (0 == msdk_strncmp(strInput[i], MSDK_STRING("-rdrm"), 5))
        {
            pParams->memType = D3D9_MEMORY;
            pParams->mode = MODE_RENDERING;
            pParams->libvaBackend = MFX_LIBVA_DRM_MODESET;
            if (strInput[i][5]) {
                if (strInput[i][5] != '-') {
                    PrintHelp(strInput[0], MSDK_STRING("unsupported monitor type"));
                    return MFX_ERR_UNSUPPORTED;
                }
                pParams->monitorType = getMonitorType(&strInput[i][6]);
                if (pParams->monitorType >= MFX_MONITOR_MAXNUMBER) {
                    PrintHelp(strInput[0], MSDK_STRING("unsupported monitor type"));
                    return MFX_ERR_UNSUPPORTED;
                }
            } else {
                pParams->monitorType = MFX_MONITOR_AUTO; // that's case of "-rdrm" pure option
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-window")))
        {
            if(i +4 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -window key"));
                return MFX_ERR_UNSUPPORTED;
            }
            msdk_opt_read(strInput[++i], pParams->nRenderWinX);
            msdk_opt_read(strInput[++i], pParams->nRenderWinY);
            msdk_opt_read(strInput[++i], pParams->Width);
            msdk_opt_read(strInput[++i], pParams->Height);

            if (0 == pParams->Width)
                pParams->Width = 320;
            if (0 == pParams->Height)
                pParams->Height = 240;

            pParams->bRenderWin = true;
        }
#endif
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-low_latency")))
        {
            switch (pParams->videoType)
            {
                case MFX_CODEC_HEVC:
                case MFX_CODEC_AVC:
                case MFX_CODEC_JPEG:
                {
                    pParams->bLowLat = true;
                    if (!pParams->bIsMVC)
                        break;
                }
                default:
                {
                     PrintHelp(strInput[0], MSDK_STRING("-low_latency mode is suppoted only for H.264 and JPEG codecs"));
                     return MFX_ERR_UNSUPPORTED;
                }
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-jpeg_rotate")))
        {
            if(MFX_CODEC_JPEG != pParams->videoType)
                return MFX_ERR_UNSUPPORTED;

            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -jpeg_rotate key"));
                return MFX_ERR_UNSUPPORTED;
            }

            msdk_opt_read(strInput[++i], pParams->nRotation);
            if((pParams->nRotation != 90)&&(pParams->nRotation != 180)&&(pParams->nRotation != 270))
            {
                PrintHelp(strInput[0], MSDK_STRING("-jpeg_rotate is supported only for 90, 180 and 270 angles"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-calc_latency")))
        {
            switch (pParams->videoType)
            {
                case MFX_CODEC_HEVC:
                case MFX_CODEC_AVC:
                case MFX_CODEC_JPEG:
                {
                    pParams->bCalLat = true;
                    if (!pParams->bIsMVC)
                        break;
                }
                default:
                {
                     PrintHelp(strInput[0], MSDK_STRING("-calc_latency mode is suppoted only for H.264 and JPEG codecs"));
                     return MFX_ERR_UNSUPPORTED;
                }
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-async")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -async key"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nAsyncDepth))
            {
                PrintHelp(strInput[0], MSDK_STRING("async is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-timeout")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -timeout key"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nTimeout))
            {
                PrintHelp(strInput[0], MSDK_STRING("timeout is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }

        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-di")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -di key"));
                return MFX_ERR_UNSUPPORTED;
            }
            msdk_char diMode[32] = {};
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], diMode))
            {
                PrintHelp(strInput[0], MSDK_STRING("deinterlace value is not set"));
                return MFX_ERR_UNSUPPORTED;
            }

            if (0 == msdk_strcmp(diMode, MSDK_CHAR("bob")))
            {
                pParams->eDeinterlace = MFX_DEINTERLACING_BOB;
            }
            else if (0 == msdk_strcmp(diMode, MSDK_CHAR("adi")))
            {
                pParams->eDeinterlace = MFX_DEINTERLACING_ADVANCED;
            }
            else
            {
                PrintHelp(strInput[0], MSDK_STRING("deinterlace value is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-gpucopy::on")))
        {
            pParams->gpuCopy = MFX_GPUCOPY_ON;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-gpucopy::off")))
        {
            pParams->gpuCopy = MFX_GPUCOPY_OFF;
        }
#if !defined(_WIN32) && !defined(_WIN64)
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-threads_num")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -threads_num key"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nThreadsNum))
            {
                PrintHelp(strInput[0], MSDK_STRING("threads_num is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-threads_schedtype")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -threads_schedtype key"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_thread_get_schedtype(strInput[++i], pParams->SchedulingType))
            {
                PrintHelp(strInput[0], MSDK_STRING("threads_schedtype is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-threads_priority")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -threads_priority key"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->Priority))
            {
                PrintHelp(strInput[0], MSDK_STRING("threads_priority is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
#endif // #if !defined(_WIN32) && !defined(_WIN64)
#if _MSDK_API >= MSDK_API(1,22)
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-dec_postproc")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for \"-dec_postproc\", right is  \"--dec_postproc force//auto\""));
                return MFX_ERR_UNSUPPORTED;
            }
            msdk_char postProcMode[32] = {};
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], postProcMode))
            {
                PrintHelp(strInput[0], MSDK_STRING("dec_postproc value is not set"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (0 == msdk_strcmp(postProcMode, MSDK_STRING("auto")))
            {
                pParams->nDecoderPostProcessing = MODE_DECODER_POSTPROC_AUTO;
            }
            else if (0 == msdk_strcmp(postProcMode, MSDK_STRING("force")))
            {
                pParams->nDecoderPostProcessing = MODE_DECODER_POSTPROC_FORCE;
            }
            else
            {
                PrintHelp(strInput[0], MSDK_STRING("dec_postproc is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
#endif //_MSDK_API >= MSDK_API(1,22)
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-f")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -f key"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nMaxFPS))
            {
                PrintHelp(strInput[0], MSDK_STRING("rendering frame rate is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-scr:w")))
        {
            if (i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -scr:w key"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->scrWidth))
            {
                PrintHelp(strInput[0], MSDK_STRING("screen width rate is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-scr:h")))
        {
            if (i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -scr:h key"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->scrHeight))
            {
                PrintHelp(strInput[0], MSDK_STRING("screen height is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-w")))
        {
            if (i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -w key"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->Width))
            {
                PrintHelp(strInput[0], MSDK_STRING("width is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-h")))
        {
            if (i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -h key"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->Height))
            {
                PrintHelp(strInput[0], MSDK_STRING("height is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-n")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for -n key"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nFrames))
            {
                PrintHelp(strInput[0], MSDK_STRING("rendering frame rate is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-jpeg_rgb")))
        {
            if(MFX_CODEC_JPEG == pParams->videoType)
            {
               pParams->chromaType = MFX_JPEG_COLORFORMAT_RGB;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-dots")))
        {
            pParams->bDots = true;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-i420")))
        {
            pParams->fourcc = MFX_FOURCC_NV12;
            pParams->outI420 = true;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-nv12")))
        {
            pParams->fourcc = MFX_FOURCC_NV12;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-rgb4")))
        {
            pParams->fourcc = MFX_FOURCC_RGB4;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-rgb4_fcr")))
        {
            pParams->fourcc = MFX_FOURCC_RGB4;
            pParams->bUseFullColorRange = true;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-p010")))
        {
            pParams->fourcc = MFX_FOURCC_P010;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-a2rgb10")))
        {
            pParams->fourcc = MFX_FOURCC_A2RGB10;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-path")))
        {
            i++;
#if defined(_WIN32) || defined(_WIN64)
            msdk_char wchar[MSDK_MAX_FILENAME_LEN];
            msdk_opt_read(strInput[i], wchar);
            std::wstring wstr(wchar);
            std::string str(wstr.begin(), wstr.end());

            strcpy_s(pParams->pluginParams.strPluginPath, str.c_str());
#else
            msdk_opt_read(strInput[i], pParams->pluginParams.strPluginPath);
#endif
            pParams->pluginParams.type = MFX_PLUGINLOAD_TYPE_FILE;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-i:null")))
        {
            ;
        }
        else // 1-character options
        {
            switch (strInput[i][1])
            {
            case MSDK_CHAR('p'):
                if (++i < nArgNum) {
                   if (MFX_ERR_NONE == ConvertStringToGuid(strInput[i], pParams->pluginParams.pluginGuid))
                    {
                        if(pParams->pluginParams.type != MFX_PLUGINLOAD_TYPE_FILE)
                        {
                            pParams->pluginParams.type = MFX_PLUGINLOAD_TYPE_GUID;
                        }
                    }
                    else
                    {
                        PrintHelp(strInput[0], MSDK_STRING("Unknown options"));
                    }
                 }
                else {
                   PrintHelp(strInput[0], MSDK_STRING("error: option '-p' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('i'):
                if (++i < nArgNum) {
					if (pParams->numViews == 0) {
						msdk_opt_read(strInput[i], pParams->strSrcFile[0]);
						pParams->numViews = 1;
					}
					else {
						msdk_opt_read(strInput[i], pParams->strSrcFile[1]);
						pParams->numViews = 2;
					}

					pParams->mode = MODE_FILE_DUMP;
                }
                else {
                   PrintHelp(strInput[0], MSDK_STRING("error: option '-i' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('o'):
				complete = true;
                break;
            case MSDK_CHAR('?'):
                PrintHelp(strInput[0], NULL);
                return MFX_ERR_UNSUPPORTED;
            default:
                {
                    std::basic_stringstream<msdk_char> stream;
                    stream << MSDK_STRING("Unknown option: ") << strInput[i];
                    PrintHelp(strInput[0], stream.str().c_str());
                    return MFX_ERR_UNSUPPORTED;
                }
            }
        }
    }

    if (0 == msdk_strlen(pParams->strSrcFile[0]) && MFX_CODEC_CAPTURE != pParams->videoType)
    {
       PrintHelp(strInput[0], MSDK_STRING("error: source file name not found"));
        return MFX_ERR_UNSUPPORTED;
    }

	if ((pParams->numViews > 1) && (0 == msdk_strlen(pParams->strSrcFile[1])))
    {
       PrintHelp(strInput[0], MSDK_STRING("error: second source file name not found"));
        return MFX_ERR_UNSUPPORTED;
    }

	if ((pParams->bIsMVC) && (pParams->numViews > 1))
    {
		PrintHelp(strInput[0], MSDK_STRING("MVC codec only supports one input file"));
        return MFX_ERR_UNSUPPORTED;
	}

    if (MFX_CODEC_CAPTURE == pParams->videoType)
    {
        if (!pParams->scrWidth || !pParams->scrHeight)
        {
           PrintHelp(strInput[0], MSDK_STRING("error: for screen capture, width and height must be specified manually (-scr:w and -scr:h)"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (pParams->scrWidth || pParams->scrHeight)
    {
       PrintHelp(strInput[0], MSDK_STRING("error: width and height parameters are supported only by screen capture decoder"));
        return MFX_ERR_UNSUPPORTED;
    }

/*
    if ((pParams->mode == MODE_FILE_DUMP) && (0 == msdk_strlen(pParams->strDstFile)))
    {
       PrintHelp(strInput[0], MSDK_STRING("error: destination file name not found"));
        return MFX_ERR_UNSUPPORTED;
    }
*/

    if (MFX_CODEC_MPEG2   != pParams->videoType &&
        MFX_CODEC_AVC     != pParams->videoType &&
        MFX_CODEC_HEVC    != pParams->videoType &&
        MFX_CODEC_VC1     != pParams->videoType &&
        MFX_CODEC_JPEG    != pParams->videoType &&
        MFX_CODEC_CAPTURE != pParams->videoType &&
        MFX_CODEC_VP8     != pParams->videoType &&
        MFX_CODEC_VP9     != pParams->videoType)
    {
        PrintHelp(strInput[0], MSDK_STRING("Unknown codec"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (pParams->nAsyncDepth == 0)
    {
        pParams->nAsyncDepth = 8; //set by default;
    }

    return MFX_ERR_NONE;
}

DWORD WINAPI EncodeThreadRun( LPVOID lpParam )
{
    mfxStatus sts = MFX_ERR_NONE; // return value check
	CEncodingPipeline   *pPipelineEncode = (CEncodingPipeline*)lpParam;

	for (;;)
    {
        sts = pPipelineEncode->Run();

        if (MFX_ERR_DEVICE_LOST == sts || MFX_ERR_DEVICE_FAILED == sts)
        {
            msdk_printf(MSDK_STRING("\nERROR: Hardware device was lost or returned an unexpected error. Recovering...\n"));
            sts = pPipelineEncode->ResetDevice();
            MSDK_CHECK_STATUS(sts, "pPipeline->ResetDevice failed");
			break;
			/*
            sts = pPipelineEncode->ResetMFXComponents(&Params);
            MSDK_CHECK_STATUS(sts, "pPipeline->ResetMFXComponents failed");
            continue;
			*/
        }
        else
        {
            MSDK_CHECK_STATUS(sts, "pPipeline->Run failed");
            break;
        }
    }

    pPipelineEncode->CaptureStopV4L2Pipeline();

    pPipelineEncode->Close();

    msdk_printf(MSDK_STRING("\nProcessing finished\n"));
	return 0;
}

#if defined(_WIN32) || defined(_WIN64)
int _tmain(int argc, TCHAR *argv[])
#else
int main(int argc, char *argv[])
#endif
{

    sInputParams        Params;   // input parameters from command line
    CDecodingPipeline   Pipeline[2]; // pipeline for decoding, includes input file reader, decoder and output file writer
	CEncodingPipeline   *pPipelineEncode;
	CFrameFifo MemFrames;
	HANDLE  hEncodeThread;
	DWORD   dwEncodeThreadId;
	mfxU16 numDecoders;

	int argPos;

    mfxStatus sts = MFX_ERR_NONE; // return value check

    sts = ParseInputString(argv, (mfxU8)argc, &argPos, &Params);
    MSDK_CHECK_STATUS(sts, "Decoder options incorrect");

	/* Multiview required twice the number of frames. */

	if ((Params.bIsMVC) || (Params.numViews < 2)) {
		numDecoders = 1;

		if ((Params.bIsMVC)) {
			Pipeline[0].SetMultiView();
			Params.nFrames *= 2;
		}

		sts = Pipeline[0].Init(&Params, &MemFrames);
		MSDK_CHECK_STATUS(sts, "Pipeline.Init failed");
		// print stream info
		Pipeline[0].PrintInfo();
	}
	else {
		numDecoders = 2;

		sts = Pipeline[0].Init(&Params, &MemFrames);
		MSDK_CHECK_STATUS(sts, "Pipeline.Init failed");

		sts = Pipeline[1].Init(&Params, &MemFrames, 1);
		MSDK_CHECK_STATUS(sts, "Pipeline.Init failed");

		// print stream info
		Pipeline[0].PrintInfo();
		// print stream info
		Pipeline[1].PrintInfo();
	}
	
	mfxFrameInfo frameInfo = Pipeline[0].GetFrameInfo();
	sts = SetupEncoder(argc, argv, argPos, &frameInfo, pPipelineEncode, &MemFrames);
    MSDK_CHECK_STATUS(sts, "Pipeline.Init failed");

    // Create the thread to begin execution on its own.

    hEncodeThread = CreateThread( 
        NULL,                   // default security attributes
        0,                      // use default stack size  
        EncodeThreadRun,       // thread function name
        (void*)pPipelineEncode,          // argument to thread function 
        0,                      // use default creation flags 
        &dwEncodeThreadId);   // returns the thread identifier 


    // Check the return value for success.
    // If CreateThread fails, terminate execution. 
    // This will automatically clean up threads and memory. 

    if (hEncodeThread == NULL) 
    {
        msdk_printf(MSDK_STRING("\nERROR: Create thread failed!\n"));
        ExitProcess(3);
    }

    msdk_printf(MSDK_STRING("Decoding started\n"));

    for (;;)
    {
        sts = Pipeline.RunDecoding(pPipelineEncode);

        if (MFX_ERR_INCOMPATIBLE_VIDEO_PARAM == sts || MFX_ERR_DEVICE_LOST == sts || MFX_ERR_DEVICE_FAILED == sts)
        {
            if (MFX_ERR_INCOMPATIBLE_VIDEO_PARAM == sts)
            {
                msdk_printf(MSDK_STRING("\nERROR: Incompatible video parameters detected. Recovering...\n"));
            }
            else
            {
                msdk_printf(MSDK_STRING("\nERROR: Hardware device was lost or returned unexpected error. Recovering...\n"));
                sts = Pipeline.ResetDevice();
                MSDK_CHECK_STATUS(sts, "Pipeline.ResetDevice failed");
            }

            sts = Pipeline.ResetDecoder(&Params);
            MSDK_CHECK_STATUS(sts, "Pipeline.ResetDecoder failed");
            continue;
        }
        else
        {
            MSDK_CHECK_STATUS(sts, "Pipeline.RunDecoding failed");
            break;
        }
    }

    msdk_printf(MSDK_STRING("\nDecoding finished\n"));

    WaitForSingleObject(hEncodeThread, 20000);

    // Close all thread handles and free memory allocations.

    CloseHandle(hEncodeThread);

    return 0;
}
