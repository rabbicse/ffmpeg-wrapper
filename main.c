#include <stdio.h>

#include "include/libavcodec/avcodec.h"
#include "include/libavformat/avformat.h"
#include "include/libswscale/swscale.h"

const int STATUS_OK = 0;

int main(int argc, char *argv[]) {
    printf("=== FFMPEG Basic ===\n");

    /* This registers all available file formats and codecs with the library so they will be used automatically
     * when a file with the corresponding format/codec is opened. Note that you only need to call av_register_all()
     * once, so we do it here in main(). If you like, it's possible to register only certain individual file formats
     * and codecs, but there's usually no reason why you would have to do that. */
    av_register_all();

    /* Declaration of variables */
    AVFormatContext *pFormatCtx = NULL;

    /* Open video file
     * open with avformat_open_input
     * open a video from local directory
     */
    const char *url = "/home/mehmet/CLionProjects/ffmpeg-tutorial/videos_in/mehmet.mp4";
    int status = avformat_open_input(&pFormatCtx, url, NULL, NULL);
    printf("Open video file status: %d\n", status);

    /* If open file status is not OK then return */
    if(status != STATUS_OK) return -1;

    /* Retrieve stream information
     * This function only looks at the header, so next we need to check out the stream information in the file.
     */
    status = avformat_find_stream_info(pFormatCtx, NULL);
    printf("Retrieve stream info status: %d\n", status);
    if(status != STATUS_OK) return -2;
    return 0;
}