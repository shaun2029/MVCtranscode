# MVCtranscode

MVCtranscode can be used to combine two elementry video streams to create a 3D MVC output.
MVCtranscode can also transcode 3D/2D h264 streams with the aim of reducing the bitrate.

By default Intel Quick Sync hardware accelerated encoding is used.
Software only decoding/encoding options are offered for systems without Intel Quicksync support.

```
NOTE:   When combining two streams to generate a 3D MVC output, the frame sizes,
        frame rates and codecs of the two input streams must match.

        This application works best with 4th generation Intel Core processor(codename Haswell) onward.

        Although not guaranteed, default settings are aimed at bluray compliance.

        The default are geared towards high quality at a reasonable size.
```

```
MVCTranscode Version 1.0.0

Usage: MVCtranscode.exe <codecid> [<decode options>] -i InputBitstream [-i InputBitstream] <codecid> 
  -o OutputBitstream [<encode options>]

Supported codecs (<codecid>):
   <codecid>=h264|mpeg2|vc1|mvc - built-in Media SDK codecs

Decode & Encode Options:
   [-hw]                     - use platform specific SDK implementation (default)
   [-sw]                     - use software implementation, if not specified platform specific SDK implementation is used
   [-f]                      - rendering framerate
   [-async]                 - depth of asynchronous pipeline. default value is 8. must be between 1 and 20.
   [-async]                  - depth of asynchronous pipeline. default value is 8. must be between 1 and 20
   [-gpucopy::<on,off>] Enable or disable GPU copy mode
   [-timeout]                - timeout in seconds


Decode Options:
   [-di bob/adi]             - enable deinterlacing BOB/ADI
   [-n number]               - number of frames to process
   [-dots]                   - output a dot to stderr, every 20 frames.


Encode Options:
   [-tff|bff] - input stream is interlaced, top|bottom fielf first, if not specified progressive is expected
   [-bref] - arrange B frames in B pyramid reference structure
   [-nobref] -  do not use B-pyramid (by default the decision is made by library). enabled by default.
   [-idr_interval size] - idr interval, default 0 means every I is an IDR, 1 means every other I frame is an IDR etc
   [-b bitRate] - encoded bit rate (Kbits per second), valid for H.264, H.265, MPEG2 and MVC encoders
   [-u speed|quality|balanced] - target usage, valid for H.264, H.265, MPEG2 and MVC encoders.
   [-r distance] - Distance between I- or P- key frames (1 means no B-frames)
   [-g size] - GOP size (default 256)
   [-x numRefs]   - number of reference frames
   [-la] - use the look ahead bitrate control algorithm (LA BRC) (by default constant bitrate control method is used)
           for H.264, H.265 encoder. Supported only with -hw option on 4th Generation Intel Core processors.
   [-lad depth] - depth parameter for the LA BRC, the number of frames to be analyzed before encoding. In range [10,100].
            may be 1 in the case when -mss option is specified
   [-dstw width] - destination picture width, invokes VPP resizing
   [-dsth height] - destination picture height, invokes VPP resizing
   [-gpucopy::<on,off>] Enable or disable GPU copy mode
   [-qvbr quality]          - quality controlled variable bitrate control, quality in range [11,51] where 11 is the highest quality.
                              Bit rate (-b) and max bit rate (-MaxKbps) are used by qvbr bitrate control.
                              This algorithm tries to achieve the subjective quality with minimum no. of bits while trying to keep
                              the bitrate constant and HRD compliance is being followed. QVBR is supported from 4th generation
                              Intel« Core processor(codename Haswell) onward.
   [-vbr]                   - variable bitrate control
   [-cqp]                   - constant quantization parameter (CQP BRC) bitrate control method
                              (by default constant bitrate control method is used), should be used along with -qpi, -qpp, -qpb.
   [-qpi]                   - constant quantizer for I frames (if bitrace control method is CQP). In range [1,51]. 0 by default, i.e.no limitations on QP.
   [-qpp]                   - constant quantizer for P frames (if bitrace control method is CQP). In range [1,51]. 0 by default, i.e.no limitations on QP.
   [-qpb]                   - constant quantizer for B frames (if bitrace control method is CQP). In range [1,51]. 0 by default, i.e.no limitations on QP.
   [-qsv-ff]       Enable QSV-FF mode
   [-gpb:<on,off>]          - Turn this option OFF to make HEVC encoder use regular P-frames instead of GPB
   [-num_slice]             - number of slices in each video frame. 1 by default.
                              If num_slice equals zero, the encoder may choose any slice partitioning allowed by the codec standard.
   [-CodecProfile]          - specifies codec profile. HIGH by default.
   [-CodecLevel]            - specifies codec level. 4.1 by default.
   [-GopOptFlag:closed]     - closed gop. open gop by default
   [-GopOptFlag:strict]     - strict gop
   [-BufferSizeInKB ]       - represents the maximum possible size of any compressed frames
   [-MaxKbps ]              - for variable bitrate control, specifies the maximum bitrate at which
                              the encoded data enters the Video Buffering Verifier buffer
   [-viewoutput]            - instruct the MVC encoder to output each view in separate bitstream buffer.
                              Depending on the number of -o options behaves as follows:
                              1: two views are encoded in single file
                              2: two views are encoded in separate files
                              3: behaves like 2 -o opitons was used and then one -o


Example:
  MVCtranscode.exe h264 -i in-left.264 -i in-right.264 mvc -o out-leftright.264
  MVCtranscode.exe h264 -i in-left.264 -i in-right.264 mvc -o out.avc -o out.mvc -viewoutput
  MVCtranscode.exe mvc -i in.264 mvc -o out.264
  MVCtranscode.exe mvc -i in.264 mvc -o out.264 -qvbr 18 -b 20000 -MaxKbps 40000
  MVCtranscode.exe mvc -i in.264 mvc -viewoutput -o out.avc -o out.mvc

Software decoding/encoding:
  MVCtranscode.exe mvc -i in.264 mvc -o out.264 -sw -u balanced

Default Equivalent (for 1920x1080 23.98 fps):
  MVCtranscode.exe h264 -sw -i in-left.264 -i in-right.264 mvc -o out.264 -hw -qvbr 17 -b 20000 -MaxKbps 40000 -CodecLevel 41 -g 24 -r 3 -num_slice 1 -u 1 -nobref -x 2
```  
