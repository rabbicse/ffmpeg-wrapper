#include <stdio.h>

#include "include/libavcodec/avcodec.h"
#include "include/libavformat/avformat.h"
#include "include/libswscale/swscale.h"

int main(int argc, char *argv[]) {
    printf("Hello, World!\n");
    av_register_all();
    return 0;
}