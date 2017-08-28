#include <stdio.h>

#include "include/libavcodec/avcodec.h"
#include "include/libavformat/avformat.h"
#include "include/libswscale/swscale.h"
#include "include/libavutil/imgutils.h"

const int STATUS_OK = 0;

void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
    FILE *pFile;
    char szFilename[128];
    int y;

    // Open file
    sprintf(szFilename, "/home/mehmet/CLionProjects/ffmpeg-wrapper/videos_out/frame%d.ppm", iFrame);
    printf("%s\n", szFilename);
    pFile = fopen(szFilename, "wb");
    if (pFile == NULL)
        return;

    // Write header
    fprintf(pFile, "P6\n%d %d\n255\n", width, height);

    // Write pixel data
    for (y = 0; y < height; y++)
        fwrite(pFrame->data[0] + (y * pFrame->linesize[0]), 1, width * 3, pFile);

    // Close file
    fclose(pFile);
}

int decode_demo() {

    /* Declaration of variables */
    AVFormatContext *pFormatCtx = NULL;
    AVCodecContext *pContext = NULL;
    AVCodec *pCodec = NULL;
    AVFrame *pFrameRGB = NULL;
    uint8_t *buffer = NULL;
    int numBytes;

    /* This registers all available file formats and codecs with the library so they will be used automatically
     * when a file with the corresponding format/codec is opened. Note that you only need to call av_register_all()
     * once, so we do it here in main(). If you like, it's possible to register only certain individual file formats
     * and codecs, but there's usually no reason why you would have to do that. */
    av_register_all();

    /* Open video file
     * open with avformat_open_input
     * open a video from local directory
     */
    const char *url = "/home/mehmet/CLionProjects/ffmpeg-tutorial/videos_in/mehmet.mp4";
//    const char *url = "/home/mehmet/Linkin_Park_-_In_The_End.mkv";
    int status = avformat_open_input(&pFormatCtx, url, NULL, NULL);
    printf("Open video file status: %d\n", status);
    /* If open file status is not OK then return */
    if (status != STATUS_OK) return -1;

    /* Retrieve stream information
     * This function only looks at the header, so next we need to check out the stream information in the file.
     */
    status = avformat_find_stream_info(pFormatCtx, NULL);
    if (status != STATUS_OK) return -2;
    printf("Retrieve stream info status: %d\n", status);

    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, url, 0);

    /* Now pFormatCtx->streams is just an array of pointers, of size pFormatCtx->nb_streams, so let's walk through it until we find a video stream.*/
    /* Find the first video stream */
    int videoStream = -1;
    for (int i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }
    if (videoStream == -1) return -3;

    /* Assign codec id to local variable, will use on find decoder. */
    enum AVCodecID avCodecID = pFormatCtx->streams[videoStream]->codecpar->codec_id;
    printf("Codec Parameter: %d\n", avCodecID);
    /* Find the video decoder */
    pCodec = avcodec_find_decoder(avCodecID);
    if (!pCodec) {
        printf("Specified codec not found\n");
        return -4;
    }

    /* Open codec */
    pContext = avcodec_alloc_context3(pCodec);
    status = avcodec_parameters_to_context(pContext, pFormatCtx->streams[videoStream]->codecpar);
    if (status != STATUS_OK) return -5;
    printf("Convert codec param to context status: %d", status);
    if (avcodec_open2(pContext, pCodec, NULL) < 0) return -6; // Could not open codec


    /* Since we're planning to output PPM files, which are stored in 24-bit RGB,
     * we're going to have to convert our frame from its native format to RGB.
     * ffmpeg will do these conversions for us.
     * For most projects (including ours) we're going to want to convert our initial frame to a specific format.
     * Let's allocate a frame for the converted frame now.
     * Allocate an AVFrame structure */
    pFrameRGB = av_frame_alloc();
    if (pFrameRGB == NULL) return -7;

    /* Even though we've allocated the frame, we still need a place to put the raw data when we convert it.
     * We use avpicture_get_size to get the size we need, and allocate the space manually
     * Determine required buffer size and allocate buffer
     */
    numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pContext->width, pContext->height, 1);
    buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));

    /* Assign appropriate parts of buffer to image planes in pFrameRGB.
    * Note that pFrameRGB is an AVFrame, but AVFrame is a superset of AVPicture */
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer, AV_PIX_FMT_RGB24, pContext->width,
                         pContext->height, 1);


    /* initialize SWS context for software scaling */
    struct SwsContext *sws_ctx = sws_getContext(pContext->width,
                                                pContext->height,
                                                pContext->pix_fmt,
                                                pContext->width,
                                                pContext->height,
                                                AV_PIX_FMT_RGB24,
                                                SWS_BILINEAR,
                                                NULL,
                                                NULL,
                                                NULL);

    /* Allocate AVPacket */
    AVPacket *packet = av_packet_alloc();

    /* Allocate video frame */
    AVFrame *pFrame = av_frame_alloc();

    int i = 0;
    /* Read frames and save first five frames to disk */
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        /* Is this a packet from the video stream? */
        if (packet->stream_index == videoStream) {
            /* Decode video frame by 2 steps:
             * - send_packet
             * - receive frame
             * */
            int st = avcodec_send_packet(pContext, packet);
            if (st < 0) continue;
            st = avcodec_receive_frame(pContext, pFrame);

            // Did we get a video frame?
            if (st >= 0) {
                // Convert the image from its native format to RGB
                sws_scale(sws_ctx, (uint8_t const *const *) pFrame->data,
                          pFrame->linesize, 0, pContext->height,
                          pFrameRGB->data, pFrameRGB->linesize);

                // Save the frame to disk
                if (i % 30 == 0)
                    SaveFrame(pFrameRGB, pContext->width, pContext->height, i);
                ++i;
            }
        }

        /* Free the packet that was allocated by av_read_frame */
        av_packet_unref(packet);
    }

    // Free the RGB image
    av_free(buffer);
    av_frame_free(&pFrameRGB);

    // Free the YUV frame
    av_frame_free(&pFrame);

    // Close the codecs
    avcodec_close(pContext);

    // Close the video file
    avformat_close_input(&pFormatCtx);

    return 0;
}

int main(int argc, char *argv[]) {
    printf("=== FFMPEG Basic ===\n");
    return decode_demo();
}