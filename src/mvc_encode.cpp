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

#include <memory>
#include "pipeline_encode.h"
#include <stdarg.h>
#include <string>
#include "math.h"
#include "app_version.h"

#ifdef MOD_ENC
// Extensions for internal use, normally these macros are blank
#include "extension_macros.h"
#else
#define MOD_ENC_CREATE_PIPELINE
#define MOD_ENC_PRINT_HELP
#define MOD_ENC_PARSE_INPUT
#endif

static void init_video_param(mfxVideoParam *videoParam)
{
    if (videoParam == NULL)
    {
        return;
    }

    memset(videoParam, 0, sizeof(mfxVideoParam));
    videoParam->mfx.CodecId                 = MFX_CODEC_AVC;
    videoParam->mfx.CodecLevel              = MFX_LEVEL_AVC_41;
    videoParam->mfx.CodecProfile            = MFX_PROFILE_AVC_STEREO_HIGH;
    videoParam->mfx.RateControlMethod       = MFX_RATECONTROL_VBR;
    videoParam->mfx.TargetUsage             = MFX_TARGETUSAGE_BALANCED;
    videoParam->mfx.TargetKbps              = 5000;
    videoParam->mfx.GopOptFlag              = MFX_GOP_CLOSED;
    videoParam->mfx.FrameInfo.FourCC        = MFX_FOURCC_NV12;
    videoParam->mfx.FrameInfo.ChromaFormat  = MFX_CHROMAFORMAT_YUV420;
    videoParam->mfx.FrameInfo.PicStruct     = MFX_PICSTRUCT_PROGRESSIVE;
    videoParam->mfx.FrameInfo.FrameRateExtN = 25;
    videoParam->mfx.FrameInfo.FrameRateExtD = 1;
    videoParam->mfx.FrameInfo.Width         = 1920;
    videoParam->mfx.FrameInfo.CropW         = 1920;
    videoParam->mfx.FrameInfo.AspectRatioW  = 1;
    videoParam->mfx.FrameInfo.Height        = 1088;
    videoParam->mfx.FrameInfo.CropH         = 1080;
    videoParam->mfx.FrameInfo.AspectRatioH  = 1;
    videoParam->AsyncDepth                  = 8;
	videoParam->mfx.GopPicSize				= 25;
	videoParam->mfx.GopRefDist				= 3;
    videoParam->IOPattern                   = MFX_IOPATTERN_IN_SYSTEM_MEMORY;
}

void PrintHelp(msdk_char *strAppName, const msdk_char *strErrorMessage)
{
    msdk_printf(MSDK_STRING("MVCTranscode Version %s\n\n"), GetAppVersion().c_str());

	if (strErrorMessage)
    {
        msdk_printf(MSDK_STRING("Error: %s\n"), strErrorMessage);
		return;
    }

	msdk_printf(MSDK_STRING("MVCtranscode can be used to combine two elementry video streams to create a 3D MVC output.\n")); 
	msdk_printf(MSDK_STRING("It can also transcode 3D/2D h264 streams with the aim of reducing the bitrate.\n"));
	msdk_printf(MSDK_STRING("By default Intel Quick Sync hardware accelerated encoding is used.\n"));
    msdk_printf(MSDK_STRING("Software only decoding/encoding options are offered for systems without Intel QuickSync support.\n\n"));
	msdk_printf(MSDK_STRING("NOTE:\tWhen combining two streams to generate a 3D MVC output, the frame sizes,\n\tframe rates and codecs of the two input streams must match.\n\n"));
	msdk_printf(MSDK_STRING("\tThis application works best with 4th generation Intel Core processor(codename Haswell) onward.\n\n"));
	msdk_printf(MSDK_STRING("\tAlthough not guaranteed, default settings are aimed at bluray compliance.\n\n"));
	msdk_printf(MSDK_STRING("\tThe default are geared towards high quality at a reasonable size.\n\n"));

    msdk_printf(MSDK_STRING("Usage: %s -help\n"), strAppName);
    msdk_printf(MSDK_STRING("   Print this help.\n\n"));
    msdk_printf(MSDK_STRING("Usage: %s -caps\n"), strAppName);
    msdk_printf(MSDK_STRING("   Print supported capabilities.\n\n"));
    msdk_printf(MSDK_STRING("Usage: %s <codecid> [<decode options>] -i InputBitstream [-i InputBitstream] <codecid> -o OutputBitstream [<encode options>]\n"), strAppName);
    msdk_printf(MSDK_STRING("\n"));
    msdk_printf(MSDK_STRING("Supported codecs (<codecid>):\n"));
    msdk_printf(MSDK_STRING("   <codecid>=h264|mpeg2|vc1|mvc - built-in Media SDK codecs\n"));
    msdk_printf(MSDK_STRING("\n"));
    msdk_printf(MSDK_STRING("Decode & Encode Options:\n"));
    msdk_printf(MSDK_STRING("   [-hw]                     - use platform specific SDK implementation (default)\n"));
    msdk_printf(MSDK_STRING("   [-sw]                     - use software implementation, if not specified platform specific SDK implementation is used\n"));
    msdk_printf(MSDK_STRING("   [-f]                      - rendering framerate\n"));
    msdk_printf(MSDK_STRING("   [-async]                 - depth of asynchronous pipeline. default value is 8. must be between 1 and 20.\n"));
#if D3D_SURFACES_SUPPORT
    msdk_printf(MSDK_STRING("   [-d3d]                    - work with d3d9 surfaces\n"));
    msdk_printf(MSDK_STRING("   [-d3d11]                  - work with d3d11 surfaces\n"));
    msdk_printf(MSDK_STRING("\n"));

#endif
    msdk_printf(MSDK_STRING("   [-async]                  - depth of asynchronous pipeline. default value is 8. must be between 1 and 20\n"));
    msdk_printf(MSDK_STRING("   [-gpucopy::<on,off>] Enable or disable GPU copy mode\n"));
	msdk_printf(MSDK_STRING("   [-timeout]                - timeout in seconds\n"));
    msdk_printf(MSDK_STRING("\n"));
    msdk_printf(MSDK_STRING("\n"));
    
	msdk_printf(MSDK_STRING("Decode Options:\n"));
	msdk_printf(MSDK_STRING("   [-di bob/adi]             - enable deinterlacing BOB/ADI\n"));
    msdk_printf(MSDK_STRING("   [-n number]               - number of frames to process\n"));
    msdk_printf(MSDK_STRING("   [-dots]                   - output a dot to stderr, every 20 frames.\n"));
	msdk_printf(MSDK_STRING("\n"));
    msdk_printf(MSDK_STRING("\n"));

	msdk_printf(MSDK_STRING("Encode Options:\n"));
    msdk_printf(MSDK_STRING("   [-tff|bff] - input stream is interlaced, top|bottom fielf first, if not specified progressive is expected\n"));
    msdk_printf(MSDK_STRING("   [-bref] - arrange B frames in B pyramid reference structure\n"));
    msdk_printf(MSDK_STRING("   [-nobref] -  do not use B-pyramid (by default the decision is made by library). enabled by default.\n"));
    msdk_printf(MSDK_STRING("   [-idr_interval size] - idr interval, default 0 means every I is an IDR, 1 means every other I frame is an IDR etc\n"));
    msdk_printf(MSDK_STRING("   [-b bitRate] - encoded bit rate (Kbits per second), valid for H.264, H.265, MPEG2 and MVC encoders \n"));
    msdk_printf(MSDK_STRING("   [-u speed|quality|balanced] - target usage, valid for H.264, H.265, MPEG2 and MVC encoders.\n"));
    msdk_printf(MSDK_STRING("   [-r distance] - Distance between I- or P- key frames (1 means no B-frames) \n"));
    msdk_printf(MSDK_STRING("   [-g size] - GOP size (default 256)\n"));
    msdk_printf(MSDK_STRING("   [-x numRefs]   - number of reference frames\n"));
    msdk_printf(MSDK_STRING("   [-la] - use the look ahead bitrate control algorithm (LA BRC) (by default constant bitrate control method is used)\n"));
    msdk_printf(MSDK_STRING("           for H.264, H.265 encoder. Supported only with -hw option on 4th Generation Intel Core processors. \n"));
    msdk_printf(MSDK_STRING("   [-lad depth] - depth parameter for the LA BRC, the number of frames to be analyzed before encoding. In range [10,100].\n"));
    msdk_printf(MSDK_STRING("            may be 1 in the case when -mss option is specified \n"));
    msdk_printf(MSDK_STRING("   [-dstw width] - destination picture width, invokes VPP resizing\n"));
    msdk_printf(MSDK_STRING("   [-dsth height] - destination picture height, invokes VPP resizing\n"));
    msdk_printf(MSDK_STRING("   [-gpucopy::<on,off>] Enable or disable GPU copy mode\n"));
    msdk_printf(MSDK_STRING("   [-qvbr quality]          - quality controlled variable bitrate control, quality in range [11,51] where 11 is the highest quality.\n"));
    msdk_printf(MSDK_STRING("                              Bit rate (-b) and max bit rate (-MaxKbps) are used by qvbr bitrate control.\n"));
    msdk_printf(MSDK_STRING("                              This algorithm tries to achieve the subjective quality with minimum no. of bits while trying to keep\n"));
    msdk_printf(MSDK_STRING("                              the bitrate constant and HRD compliance is being followed. QVBR is supported from 4th generation\n"));
    msdk_printf(MSDK_STRING("                              Intel® Core processor(codename Haswell) onward.\n"));
    msdk_printf(MSDK_STRING("   [-vbr]                   - variable bitrate control\n"));
    msdk_printf(MSDK_STRING("   [-cqp]                   - constant quantization parameter (CQP BRC) bitrate control method\n"));
    msdk_printf(MSDK_STRING("                              (by default constant bitrate control method is used), should be used along with -qpi, -qpp, -qpb.\n"));
    msdk_printf(MSDK_STRING("   [-qpi]                   - constant quantizer for I frames (if bitrace control method is CQP). In range [1,51]. 0 by default, i.e.no limitations on QP.\n"));
    msdk_printf(MSDK_STRING("   [-qpp]                   - constant quantizer for P frames (if bitrace control method is CQP). In range [1,51]. 0 by default, i.e.no limitations on QP.\n"));
    msdk_printf(MSDK_STRING("   [-qpb]                   - constant quantizer for B frames (if bitrace control method is CQP). In range [1,51]. 0 by default, i.e.no limitations on QP.\n"));
    msdk_printf(MSDK_STRING("   [-qsv-ff]       Enable QSV-FF mode\n"));
    msdk_printf(MSDK_STRING("   [-gpb:<on,off>]          - Turn this option OFF to make HEVC encoder use regular P-frames instead of GPB\n"));
    msdk_printf(MSDK_STRING("   [-num_slice]             - number of slices in each video frame. 1 by default.\n"));
    msdk_printf(MSDK_STRING("                              If num_slice equals zero, the encoder may choose any slice partitioning allowed by the codec standard.\n"));
    msdk_printf(MSDK_STRING("   [-CodecProfile]          - specifies codec profile. HIGH by default.\n"));
    msdk_printf(MSDK_STRING("   [-CodecLevel]            - specifies codec level. 4.1 by default.\n"));
    msdk_printf(MSDK_STRING("   [-GopOptFlag:closed]     - closed gop. open gop by default\n"));
    msdk_printf(MSDK_STRING("   [-GopOptFlag:strict]     - strict gop\n"));
    msdk_printf(MSDK_STRING("   [-BufferSizeInKB ]       - represents the maximum possible size of any compressed frames\n"));
    msdk_printf(MSDK_STRING("   [-MaxKbps ]              - for variable bitrate control, specifies the maximum bitrate at which \n"));
    msdk_printf(MSDK_STRING("                              the encoded data enters the Video Buffering Verifier buffer\n"));
    msdk_printf(MSDK_STRING("   [-viewoutput]            - instruct the MVC encoder to output each view in separate bitstream buffer.\n"));
	msdk_printf(MSDK_STRING("                              Depending on the number of -o options behaves as follows:\n"));
    msdk_printf(MSDK_STRING("                              1: two views are encoded in single file\n"));
    msdk_printf(MSDK_STRING("                              2: two views are encoded in separate files\n"));
    msdk_printf(MSDK_STRING("                              3: behaves like 2 -o opitons was used and then one -o\n\n"));
	msdk_printf(MSDK_STRING("\n"));
	msdk_printf(MSDK_STRING("Example:\n"));
    msdk_printf(MSDK_STRING("  %s h264 -i in-left.264 -i in-right.264 mvc -o out-leftright.264\n"), strAppName);
    msdk_printf(MSDK_STRING("  %s h264 -i in-left.264 -i in-right.264 mvc -o out.avc -o out.mvc -viewoutput\n"), strAppName);
    msdk_printf(MSDK_STRING("  %s mvc -i in.264 mvc -o out.264\n"), strAppName);
    msdk_printf(MSDK_STRING("  %s mvc -i in.264 mvc -o out.264 -qvbr 18 -b 20000 -MaxKbps 40000\n"), strAppName);
    msdk_printf(MSDK_STRING("  %s mvc -i in.264 mvc -viewoutput -o out.avc -o out.mvc\n"), strAppName);
    msdk_printf(MSDK_STRING("\n"));
	msdk_printf(MSDK_STRING("Software decoding/encoding:\n"));
    msdk_printf(MSDK_STRING("  %s mvc -i in.264 mvc -o out.264 -sw -u balanced\n"), strAppName);
    msdk_printf(MSDK_STRING("\n"));
	msdk_printf(MSDK_STRING("Default Equivalent (for 1920x1080 23.98 fps):\n"));
    msdk_printf(MSDK_STRING("  %s h264 -sw -i in-left.264 -i in-right.264 mvc -o out.264 -hw -qvbr 17 -b 20000 -MaxKbps 40000 -CodecLevel 41 -g 24 -r 3 -num_slice 1 -u 1 -nobref -x 2\n"), strAppName);
}

void PrintCaps() {
	mfxSession session;
	mfxVersion qsv_software_version = {0}, qsv_hardware_version = {0};

	// check for software fallback
	if (MFXInit(MFX_IMPL_SOFTWARE, NULL, &session) == MFX_ERR_NONE)
	{
		// Media SDK software found, but check that our minimum is supported
		MFXQueryVersion(session, &qsv_software_version);
		MFXClose(session);
		fprintf(stdout, "software: yes\n");
		fprintf(stdout, "software version: %d.%d\n", qsv_software_version.Major, qsv_software_version.Minor);
		fprintf(stdout, "software version#: %d\n", qsv_software_version.Version);
	}
	else {
		fprintf(stdout, "software: no\n");
	}

	if (MFXInit(MFX_IMPL_HARDWARE, NULL, &session) == MFX_ERR_NONE)
	{
		// Media SDK hardware found, but check that our minimum is supported
		MFXQueryVersion(session, &qsv_hardware_version);
		fprintf(stdout, "hardware: yes\n");
		fprintf(stdout, "hardware version: %d.%d\n", qsv_hardware_version.Major, qsv_hardware_version.Minor);
		fprintf(stdout, "hardware version#: %d\n", qsv_hardware_version.Version);

		mfxVideoParam inputParam = {0}, videoParam = {0};
		init_video_param(&inputParam);

		inputParam.mfx.RateControlMethod = MFX_RATECONTROL_QVBR;
		videoParam = inputParam;
		if (MFXVideoENCODE_Query(session, &inputParam, &videoParam) >= MFX_ERR_NONE) {
			if ((videoParam.mfx.RateControlMethod == MFX_RATECONTROL_QVBR)  && (inputParam.mfx.CodecProfile == MFX_PROFILE_AVC_STEREO_HIGH)) {
				fprintf(stdout, "qvbr: yes\n");
			}
			else {
				fprintf(stdout, "qvbr: no\n");
			}
		}

		MFXClose(session);
	}
	else {
		fprintf(stdout, "hardware: no\n");
	}
	exit(0);
}

static mfxStatus ParseInputString(msdk_char* strInput[], mfxU8 nArgNum, mfxU8 argPos, sEncInputParams* pParams)
{

    if (1 == nArgNum)
    {
        PrintHelp(strInput[0], NULL);
        return MFX_ERR_UNSUPPORTED;
    }

    MSDK_CHECK_POINTER(pParams, MFX_ERR_NULL_PTR);
    msdk_opt_read(MSDK_CPU_ROTATE_PLUGIN, pParams->strPluginDLLPath);

    // default implementation
    pParams->isV4L2InputEnabled = false;
    pParams->nNumFrames = 0;
    pParams->FileInputFourCC = MFX_FOURCC_I420;
    pParams->EncodeFourCC = MFX_FOURCC_NV12;
#if defined (ENABLE_V4L2_SUPPORT)
    pParams->MipiPort = -1;
    pParams->MipiMode = NONE;
    pParams->v4l2Format = NO_FORMAT;
#endif

    // parse command line parameters
    for (mfxU8 i = argPos; i < nArgNum; i++)
    {
        MSDK_CHECK_POINTER(strInput[i], MFX_ERR_NULL_PTR);

        if (MSDK_CHAR('-') != strInput[i][0])
        {
            mfxStatus sts = StrFormatToCodecFormatFourCC(strInput[i], pParams->CodecId);
            if (sts != MFX_ERR_NONE)
            {
                PrintHelp(strInput[0], MSDK_STRING("Unknown codec"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (!IsEncodeCodecSupported(pParams->CodecId))
            {
                PrintHelp(strInput[0], MSDK_STRING("Unsupported codec"));
                return MFX_ERR_UNSUPPORTED;
            }
            if (pParams->CodecId == CODEC_MVC)
            {
                pParams->CodecId = MFX_CODEC_AVC;
                pParams->MVC_flags |= MVC_ENABLED;
            }
            continue;
        }

        // process multi-character options
        if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-dstw")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for Destination picture Width"));
                return MFX_ERR_UNSUPPORTED;
            }
		}
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-dsth")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for Destination picture Height"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-sw")))
        {
            pParams->bUseHWLib = false;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-hw")))
        {
            pParams->bUseHWLib = true;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-yuy2")))
        {
#if defined (ENABLE_V4L2_SUPPORT)
            pParams->v4l2Format = YUY2;
#endif
            pParams->FileInputFourCC = MFX_FOURCC_YUY2;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-nv12")))
        {
            pParams->FileInputFourCC = MFX_FOURCC_NV12;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-rgb4")))
        {
            pParams->FileInputFourCC = MFX_FOURCC_RGB4;
            pParams->EncodeFourCC = MFX_FOURCC_RGB4;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-p010")))
        {
            pParams->FileInputFourCC = MFX_FOURCC_P010;
            pParams->EncodeFourCC = MFX_FOURCC_P010;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-ec::p010")))
        {
            pParams->EncodeFourCC = MFX_FOURCC_P010;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-tff")))
        {
            pParams->nPicStruct = MFX_PICSTRUCT_FIELD_TFF;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-bff")))
        {
            pParams->nPicStruct = MFX_PICSTRUCT_FIELD_BFF;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-bref")))
        {
            pParams->nBRefType = MFX_B_REF_PYRAMID;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-nobref")))
        {
            pParams->nBRefType = MFX_B_REF_OFF;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-idr_interval")))
        {
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nIdrInterval))
            {
                PrintHelp(strInput[0], MSDK_STRING("IdrInterval is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-opencl")))
        {
            msdk_opt_read(MSDK_OCL_ROTATE_PLUGIN, pParams->strPluginDLLPath);
            pParams->nRotationAngle = 180;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-viewoutput")))
        {
            if (!(MVC_ENABLED & pParams->MVC_flags))
            {
                PrintHelp(strInput[0], MSDK_STRING("-viewoutput option is supported only when mvc codec specified"));
                return MFX_ERR_UNSUPPORTED;
            }
            pParams->MVC_flags |= MVC_VIEWOUTPUT;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-la")))
        {
            pParams->nRateControlMethod = MFX_RATECONTROL_LA;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-lad")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for Look Ahead Depth"));
                return MFX_ERR_UNSUPPORTED;
            }

            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nLADepth))
            {
                PrintHelp(strInput[0], MSDK_STRING("Look Ahead Depth is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-vbr")))
        {
            pParams->nRateControlMethod = MFX_RATECONTROL_VBR;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-qvbr")))
        {
            pParams->nRateControlMethod = MFX_RATECONTROL_QVBR;

            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for qvbr"));
                return MFX_ERR_UNSUPPORTED;
            }

			if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nBRCQuality))
            {
                PrintHelp(strInput[0], MSDK_STRING("Quality for qvpr is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-mss")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for MaxSliceSize"));
                return MFX_ERR_UNSUPPORTED;
            }

			if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nMaxSliceSize))
            {
                PrintHelp(strInput[0], MSDK_STRING("MaxSliceSize is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-mfs")))
       {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for MaxFrameSize"));
                return MFX_ERR_UNSUPPORTED;
            }

			if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nMaxFrameSize))
            {
                PrintHelp(strInput[0], MSDK_STRING("MaxFrameSize is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
       }
#if D3D_SURFACES_SUPPORT
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-d3d")))
        {
            pParams->memType = ENC_D3D9_MEMORY;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-d3d11")))
        {
            pParams->memType = ENC_D3D11_MEMORY;
        }
#endif
#ifdef LIBVA_SUPPORT
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-vaapi")))
        {
            pParams->memType = D3D9_MEMORY;
        }
#endif
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-async")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for Async Depth"));
                return MFX_ERR_UNSUPPORTED;
            }

            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nAsyncDepth))
            {
                PrintHelp(strInput[0], MSDK_STRING("Async Depth is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-CodecLevel")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for CodecLevel"));
                return MFX_ERR_UNSUPPORTED;
            }

			if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->CodecLevel))
            {
                PrintHelp(strInput[0], MSDK_STRING("CodecLevel is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-CodecProfile")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for CodecProfile"));
                return MFX_ERR_UNSUPPORTED;
            }

			if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->CodecProfile))
            {
                PrintHelp(strInput[0], MSDK_STRING("CodecProfile is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-GopOptFlag:closed")))
        {
            pParams->GopOptFlag = MFX_GOP_CLOSED;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-GopOptFlag:strict")))
        {
            pParams->GopOptFlag = MFX_GOP_STRICT;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-InitialDelayInKB")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for InitialDelayInKB"));
                return MFX_ERR_UNSUPPORTED;
            }

            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->InitialDelayInKB))
            {
                PrintHelp(strInput[0], MSDK_STRING("InitialDelayInKB is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-MaxKbps")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for MaxKbps"));
                return MFX_ERR_UNSUPPORTED;
            }

            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->MaxKbps))
            {
                PrintHelp(strInput[0], MSDK_STRING("MaxKbps is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-BufferSizeInKB")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for BufferSizeInKB"));
                return MFX_ERR_UNSUPPORTED;
            }

            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->BufferSizeInKB))
            {
                PrintHelp(strInput[0], MSDK_STRING("BufferSizeInKB is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-timeout")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for timeout"));
                return MFX_ERR_UNSUPPORTED;
            }

            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nTimeout))
            {
                PrintHelp(strInput[0], MSDK_STRING("Timeout is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-uncut")))
        {
            pParams->bUncut = true;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-gpucopy::on")))
        {
            pParams->gpuCopy = MFX_GPUCOPY_ON;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-gpucopy::off")))
        {
            pParams->gpuCopy = MFX_GPUCOPY_OFF;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-cqp")))
        {
            pParams->nRateControlMethod = MFX_RATECONTROL_CQP;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-qpi")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for I frames"));
                return MFX_ERR_UNSUPPORTED;
            }

			if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nQPI))
            {
                PrintHelp(strInput[0], MSDK_STRING("Quantizer for I frames is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-qpp")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for P frames"));
                return MFX_ERR_UNSUPPORTED;
            }

			if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nQPP))
            {
                PrintHelp(strInput[0], MSDK_STRING("Quantizer for P frames is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-qpb")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for B frames"));
                return MFX_ERR_UNSUPPORTED;
            }

			if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nQPB))
            {
                PrintHelp(strInput[0], MSDK_STRING("Quantizer for B frames is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-gpb:on")))
        {
            pParams->nGPB = MFX_CODINGOPTION_ON;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-gpb:off")))
        {
            pParams->nGPB = MFX_CODINGOPTION_OFF;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-qsv-ff")))
        {
            pParams->enableQSVFF=true;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-num_slice")))
        {
            if(i + 1 >= nArgNum)
            {
                PrintHelp(strInput[0], MSDK_STRING("Not enough parameters for number of slices"));
                return MFX_ERR_UNSUPPORTED;
            }

			if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->nNumSlice))
            {
                PrintHelp(strInput[0], MSDK_STRING("Number of slices is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        } else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-path")))
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
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-re")))
        {
            pParams->UseRegionEncode = true;
        }
#ifdef MOD_ENC
        MOD_ENC_PARSE_INPUT
#endif
#if defined (ENABLE_V4L2_SUPPORT)
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-d")))
        {
            VAL_CHECK(i+1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->DeviceName))
            {
                PrintHelp(strInput[0], MSDK_STRING("Device name is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-uyvy")))
        {
            pParams->v4l2Format = UYVY;

        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-p")))
        {
            VAL_CHECK(i+1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->MipiPort))
            {
                PrintHelp(strInput[0], MSDK_STRING("Mipi-port is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-m")))
        {
            VAL_CHECK(i+1 >= nArgNum, i, strInput[i]);
            if (MFX_ERR_NONE != msdk_opt_read(strInput[++i], pParams->MipiModeName))
            {
                PrintHelp(strInput[0], MSDK_STRING("Device name is invalid"));
                return MFX_ERR_UNSUPPORTED;
            }

            if(strcasecmp(pParams->MipiModeName,"STILL") == 0)
                pParams->MipiMode = STILL;
            else if(strcasecmp(pParams->MipiModeName,"VIDEO") == 0)
                pParams->MipiMode = VIDEO;
            else if(strcasecmp(pParams->MipiModeName,"PREVIEW") == 0)
                pParams->MipiMode = PREVIEW;
            else if(strcasecmp(pParams->MipiModeName,"CONTINUOUS") == 0)
                pParams->MipiMode = CONTINUOUS;
            else
                pParams->MipiMode = NONE;
        }
        else if (0 == msdk_strcmp(strInput[i], MSDK_STRING("-i::v4l2")))
        {
            pParams->isV4L2InputEnabled = true;
        }
#endif
        else // 1-character options
        {
            switch (strInput[i][1])
            {
            case MSDK_CHAR('u'):
                if (++i < nArgNum) {
                    pParams->nTargetUsage = StrToTargetUsage(strInput[i]);
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-u' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('w'):
                if (++i < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i], pParams->nWidth))
                    {
                        PrintHelp(strInput[0], MSDK_STRING("Width is invalid"));
                        return MFX_ERR_UNSUPPORTED;
                    }
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-w' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('h'):
                if (++i < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i], pParams->nHeight))
                    {
                        PrintHelp(strInput[0], MSDK_STRING("Height is invalid"));
                        return MFX_ERR_UNSUPPORTED;
                    }
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-h' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('f'):
                if (++i < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i], pParams->dFrameRate))
                    {
                        PrintHelp(strInput[0], MSDK_STRING("Frame Rate is invalid"));
                        return MFX_ERR_UNSUPPORTED;
                    }
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-f' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('b'):
                if (++i < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i], pParams->nBitRate))
                    {
                        PrintHelp(strInput[0], MSDK_STRING("Bit Rate is invalid"));
                        return MFX_ERR_UNSUPPORTED;
                    }
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-b' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('x'):
                if (++i < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i], pParams->nNumRefFrame))
                    {
                        PrintHelp(strInput[0], MSDK_STRING("Ref Num is invalid"));
                        return MFX_ERR_UNSUPPORTED;
                    }
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-x' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('g'):
                if (++i < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i], pParams->nGopPicSize))
                    {
                        PrintHelp(strInput[0], MSDK_STRING("Gop Size is invalid"));
                        return MFX_ERR_UNSUPPORTED;
                    }
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-g' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('r'):
                if (++i < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i], pParams->nGopRefDist))
                    {
                        PrintHelp(strInput[0], MSDK_STRING("Ref Dist is invalid"));
                        return MFX_ERR_UNSUPPORTED;
                    }
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-r' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('i'):
                if (++i < nArgNum) {
                    pParams->InputFiles.push_back(msdk_string(strInput[i]));
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-i' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('o'):
                if (++i < nArgNum) {
                    pParams->dstFileBuff.push_back(strInput[i]);
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-o' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('q'):
                if (++i < nArgNum) {
                    if (MFX_ERR_NONE != msdk_opt_read(strInput[i], pParams->nQuality))
                    {
                        PrintHelp(strInput[0], MSDK_STRING("Quality is invalid"));
                        return MFX_ERR_UNSUPPORTED;
                    }
                }
                else {
                    msdk_printf(MSDK_STRING("error: option '-q' expects an argument\n"));
                }
                break;
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
                    msdk_printf(MSDK_STRING("error: option '-p' expects an argument\n"));
                }
                break;
            case MSDK_CHAR('?'):
                PrintHelp(strInput[0], NULL);
                return MFX_ERR_UNSUPPORTED;
            default:
                PrintHelp(strInput[0], MSDK_STRING("Unknown options"));
            }
        }
    }

#if defined (ENABLE_V4L2_SUPPORT)
    if (pParams->isV4L2InputEnabled)
    {
        if (0 == msdk_strlen(pParams->DeviceName))
        {
            PrintHelp(strInput[0], MSDK_STRING("Device Name not found"));
            return MFX_ERR_UNSUPPORTED;
        }

        if ((pParams->MipiPort > -1 && pParams->MipiMode == NONE) ||
            (pParams->MipiPort < 0 && pParams->MipiMode != NONE))
        {
            PrintHelp(strInput[0], MSDK_STRING("Invalid Mipi Configuration\n"));
            return MFX_ERR_UNSUPPORTED;
        }

        if (pParams->v4l2Format == NO_FORMAT)
        {
            PrintHelp(strInput[0], MSDK_STRING("NO input v4l2 format\n"));
            return MFX_ERR_UNSUPPORTED;
        }
    }
#endif

    // check if all mandatory parameters were set
/*
	if (!pParams->InputFiles.size() && !pParams->isV4L2InputEnabled)
    {
        PrintHelp(strInput[0], MSDK_STRING("Source file name not found"));
        return MFX_ERR_UNSUPPORTED;
    };
*/

    if (0 == pParams->nWidth || 0 == pParams->nHeight)
    {
        PrintHelp(strInput[0], MSDK_STRING("-w, -h must be specified"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (MFX_CODEC_MPEG2 != pParams->CodecId &&
        MFX_CODEC_AVC != pParams->CodecId &&
        MFX_CODEC_JPEG != pParams->CodecId &&
        MFX_CODEC_HEVC != pParams->CodecId)
    {
        PrintHelp(strInput[0], MSDK_STRING("Unknown codec"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (MFX_CODEC_JPEG != pParams->CodecId &&
        pParams->FileInputFourCC == MFX_FOURCC_YUY2 &&
        !pParams->isV4L2InputEnabled)
    {
        PrintHelp(strInput[0], MSDK_STRING("-yuy2 option is supported only for JPEG encoder"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (MFX_CODEC_HEVC != pParams->CodecId && (pParams->EncodeFourCC == MFX_FOURCC_P010) )
    {
        PrintHelp(strInput[0], MSDK_STRING("P010 surfaces are supported only for HEVC encoder"));
        return MFX_ERR_UNSUPPORTED;
    }

    // check parameters validity
    if (pParams->nRotationAngle != 0 && pParams->nRotationAngle != 180)
    {
        PrintHelp(strInput[0], MSDK_STRING("Angles other than 180 degrees are not supported."));
        return MFX_ERR_UNSUPPORTED; // other than 180 are not supported
    }

    if (pParams->nQuality && (MFX_CODEC_JPEG != pParams->CodecId))
    {
        PrintHelp(strInput[0], MSDK_STRING("-q option is supported only for JPEG encoder"));
        return MFX_ERR_UNSUPPORTED;
    }

    if ((pParams->nTargetUsage || pParams->nBitRate) && (MFX_CODEC_JPEG == pParams->CodecId))
    {
        PrintHelp(strInput[0], MSDK_STRING("-u and -b options are supported only for H.264, MPEG2 and MVC encoders. For JPEG encoder use -q"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (!pParams->nQuality && (MFX_CODEC_JPEG == pParams->CodecId))
    {
        PrintHelp(strInput[0], MSDK_STRING("-q must be specified for JPEG encoder"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (MFX_TRANSFERMATRIX_UNKNOWN != pParams->TransferMatrix &&
        MFX_TRANSFERMATRIX_BT601 != pParams->TransferMatrix &&
        MFX_TRANSFERMATRIX_BT709 != pParams->TransferMatrix)
    {
        PrintHelp(strInput[0], MSDK_STRING("Invalid transfer matrix type"));
        return MFX_ERR_UNSUPPORTED;
    }

    // set default values for optional parameters that were not set or were set incorrectly
    mfxU32 nviews = (mfxU32)pParams->InputFiles.size();
    if ((nviews <= 1) || (nviews > 2))
    {
        if (!(MVC_ENABLED & pParams->MVC_flags))
        {
            pParams->numViews = 1;
        }
        else
        {
            pParams->numViews = 2;
        }
    }
    else
    {
        pParams->numViews = nviews;
    }

    if (pParams->nTargetUsage < MFX_TARGETUSAGE_BEST_QUALITY || pParams->nTargetUsage > MFX_TARGETUSAGE_BEST_SPEED)
    {
        pParams->nTargetUsage = MFX_TARGETUSAGE_BALANCED;
    }

    if (pParams->dFrameRate <= 0)
    {
        pParams->dFrameRate = 30;
    }

    // if no destination picture width or height wasn't specified set it to the source picture size
    if (pParams->nDstWidth == 0)
    {
        pParams->nDstWidth = pParams->nWidth;
    }

    if (pParams->nDstHeight == 0)
    {
        pParams->nDstHeight = pParams->nHeight;
    }

    if (!pParams->nPicStruct)
    {
        pParams->nPicStruct = MFX_PICSTRUCT_PROGRESSIVE;
    }

    if ((pParams->nRateControlMethod == MFX_RATECONTROL_LA) && (!pParams->bUseHWLib))
    {
        PrintHelp(strInput[0], MSDK_STRING("Look ahead BRC is supported only with -hw option!"));
        return MFX_ERR_UNSUPPORTED;
    }

    if ((pParams->nMaxSliceSize) && (!pParams->bUseHWLib))
    {
        PrintHelp(strInput[0], MSDK_STRING("MaxSliceSize option is supported only with -hw option!"));
        return MFX_ERR_UNSUPPORTED;
    }

    if ((pParams->nMaxSliceSize) && (pParams->nNumSlice))
    {
        PrintHelp(strInput[0], MSDK_STRING("-mss and -num_slice options are not compatible!"));
        return MFX_ERR_UNSUPPORTED;
    }

    //if ((pParams->nRateControlMethod == MFX_RATECONTROL_LA) && (pParams->CodecId != MFX_CODEC_AVC))
    //{
    //    PrintHelp(strInput[0], MSDK_STRING("Look ahead BRC is supported only with H.264 encoder!"));
    //    return MFX_ERR_UNSUPPORTED;
    //}

    if ((pParams->nMaxSliceSize) && (pParams->CodecId != MFX_CODEC_AVC))
    {
        PrintHelp(strInput[0], MSDK_STRING("MaxSliceSize option is supported only with H.264 encoder!"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (pParams->nLADepth && (pParams->nLADepth < 10 || pParams->nLADepth > 100))
    {
        if ((pParams->nLADepth != 1) || (!pParams->nMaxSliceSize))
        {
            PrintHelp(strInput[0], MSDK_STRING("Unsupported value of -lad parameter, must be in range [10, 100] or 1 in case of -mss option!"));
            return MFX_ERR_UNSUPPORTED;
        }
    }

    // not all options are supported if rotate plugin is enabled
    if (pParams->nRotationAngle == 180 && (
        MFX_PICSTRUCT_PROGRESSIVE != pParams->nPicStruct ||
        pParams->nDstWidth != pParams->nWidth ||
        pParams->nDstHeight != pParams->nHeight ||
        MVC_ENABLED & pParams->MVC_flags ||
        pParams->nRateControlMethod == MFX_RATECONTROL_LA))
    {
        PrintHelp(strInput[0], MSDK_STRING("Some of the command line options are not supported with rotation plugin!"));
        return MFX_ERR_UNSUPPORTED;
    }

    if (pParams->nAsyncDepth == 0)
    {
        pParams->nAsyncDepth = 4; //set by default;
    }

    // Ignoring user-defined Async Depth for LA
    if (pParams->nMaxSliceSize)
    {
        pParams->nAsyncDepth = 1;
    }

	if ((pParams->nRateControlMethod == 0) && (pParams->bUseHWLib))
    {
        pParams->nRateControlMethod = MFX_RATECONTROL_QVBR;
    }
		
	if (pParams->nRateControlMethod == 0)
    {
        pParams->nRateControlMethod = MFX_RATECONTROL_CBR;
    }
    if(pParams->UseRegionEncode)
    {
        if(pParams->CodecId != MFX_CODEC_HEVC)
        {
            msdk_printf(MSDK_STRING("Region encode option is compatible with h265(HEVC) encoder only.\nRegion encoding is disabled\n"));
            pParams->UseRegionEncode=false;
        }
        if (pParams->nWidth  != pParams->nDstWidth ||
            pParams->nHeight != pParams->nDstHeight ||
            pParams->nRotationAngle!=0)
        {
            msdk_printf(MSDK_STRING("Region encode option is not compatible with VPP processing and rotation plugin.\nRegion encoding is disabled\n"));
            pParams->UseRegionEncode=false;
        }
    }

    if (pParams->dstFileBuff.size() == 0)
    {
        msdk_printf(MSDK_STRING("File output is disabled as -o option isn't specified\n"));
    }

    return MFX_ERR_NONE;
}

CEncodingPipeline* CreatePipeline(const sEncInputParams& params)
{
#ifdef MOD_ENC
    MOD_ENC_CREATE_PIPELINE;
#endif
    return new CEncodingPipeline;
}

#if defined(_WIN32) || defined(_WIN64)
mfxStatus SetupEncoder(int argc, msdk_char *argv[], int argPos, mfxFrameInfo *pFrameInfo, CEncodingPipeline*& pPipeline, CFrameFifo **pFrameFifo)
#else
mfxStatus SetupEncoder(int argc, char *argv[])
#endif
{
    sEncInputParams Params = {0};   // input parameters from command line

    mfxStatus sts = MFX_ERR_NONE; // return value check

	Params.dFrameRate = CalculateFrameRate(pFrameInfo->FrameRateExtN, pFrameInfo->FrameRateExtD);
	Params.nWidth = pFrameInfo->Width;

	/* TODO: Height seems worng, needs more work to fix properly.
	 * e.g. 1080 -> 1088, 720 -> 738 ????
	 * Workaround by using cases based on width. */
	switch (Params.nWidth) {
	case 1280:
		Params.nHeight = 720;
		break;
	case 1920:
		Params.nHeight = 1080;
		break;
	default:
		Params.nHeight = pFrameInfo->Height;
		break;
	}
	
	Params.CodecLevel = MFX_LEVEL_AVC_41;
	Params.nBRefType = MFX_B_REF_OFF;
	Params.bUseHWLib = true;
	Params.MaxKbps = 0;
	Params.nBitRate = 0;
	Params.nBRCQuality = 17;
	Params.nGopPicSize = (mfxU16)(Params.dFrameRate + 0.5);
	Params.nGopRefDist = 3;
	Params.nAsyncDepth = 8;
	Params.nNumRefFrame = 3;
	Params.nTargetUsage = 1;
	Params.nNumSlice = 1;

	sts = ParseInputString(argv, (mfxU8)argc, (mfxU8)argPos, &Params);
    MSDK_CHECK_STATUS(sts, "Encoder options incorrect");

    msdk_printf(MSDK_STRING("MVCTranscode Version %s\n\n"), GetAppVersion().c_str());

    // calculate default bitrate based on the resolution
    if (!Params.nBitRate)
    {
		/* Base bitrate on 1920x1080. Provides a good quality. */
		Params.nBitRate = (mfxU16)(10000 * (Params.nWidth * Params.nHeight * Params.dFrameRate) / (1920*1080*24));
		/* Quantise to 1000kbps */
		Params.nBitRate = (mfxU16)(((Params.nBitRate + 500) / 1000) * 1000);

		/* Double bitrate for MVC. */
		if (Params.numViews > 1) {
			Params.nBitRate *= 2;
		}
    }

	if (!Params.MaxKbps)
	{
		Params.MaxKbps = (mfxU16)(Params.nBitRate * 3);
	}

	/* Max MVC bitrate for Intel SDK is 43000 Kbps */
	if (Params.MaxKbps > 40000) {
		Params.MaxKbps = 40000;
	}

    // Choosing which pipeline to use
    pPipeline = CreatePipeline(Params);
    MSDK_CHECK_POINTER(pPipeline, MFX_ERR_MEMORY_ALLOC);

    if (MVC_ENABLED & Params.MVC_flags)
    {
        pPipeline->SetNumView(Params.numViews);
    }

    sts = pPipeline->Init(&Params, pFrameFifo);
    MSDK_CHECK_STATUS(sts, "pPipeline->Init failed");

    pPipeline->PrintInfo();

    sts = pPipeline->CaptureStartV4L2Pipeline();
    MSDK_CHECK_STATUS(sts, "V4l2 failure terminating the program");

	msdk_printf(MSDK_STRING("Encoder initialised\n"));

    return sts;
}
