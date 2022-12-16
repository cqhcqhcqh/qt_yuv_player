#include "yuvplayer.h"
#include <QDebug>
#include <QPainter>

YuvPlayer::YuvPlayer(QWidget *parent)
    : QWidget{parent}
{
}

#define ERROR_BUF(res) \
    char errbuf[1024]; \
    av_strerror(res, errbuf, sizeof(errbuf));

void YuvPlayer::replace(YuvFile &in) {
    // 如果 YuvFile 的 path 是 char *const 来修饰的话, _in = in; 赋值语句就无法成功
    _in = in;
}

// 当组件想重绘的时候，就会调用这个函数
// 想要绘制什么内容，在这个函数中实现
void YuvPlayer::paintEvent(QPaintEvent *event) {
    if (!_currentImage) return;
    qDebug() << "paintEvent";
    // 将图片绘制到当前组件上
    QPainter(this).drawImage(_dstRect, *_currentImage);
}

void YuvPlayer::play() {
    _startPlayTimerId = startTimer(_interval);
    // 打开文件
    _file = new QFile(_in.path);
    int ret = _file->open(QFile::ReadOnly);
    if (ret == 0) {
        qDebug() << "QFile open error" << _in.path;
        return;
    }
    // 刷帧的时间间隔
    _interval = 1000 / _in.fps;

    // 一帧图片的大小
    _imgSize = av_image_get_buffer_size(_in.fmt,
                                        _in.width,
                                        _in.height, 1);

    // 组件的尺寸
    int w = width();
    int h = height();

    // 计算rect
    int dx = 0;
    int dy = 0;
    int dw = _in.width;
    int dh = _in.height;

    // 计算目标尺寸
    if (dw > w || dh > h) { // 缩放
        if (dw * h > w * dh) { // 视频的宽高比 > 播放器的宽高比
            dh = w * dh / dw;
            dw = w;
        } else {
            dw = h * dw / dh;
            dh = h;
        }
    }

    // 居中
    dx = (w - dw) >> 1;
    dy = (h - dh) >> 1;

    _dstRect = QRect(dx, dy, dw, dh);
    qDebug() << "视频的矩形框" << dx << dy << dw << dh;
}

void YuvPlayer::timerEvent(QTimerEvent *event) {
    RescaleVideoSpec in;
    in.fmt = _in.fmt;
    in.width = _in.width;
    in.height = _in.height;

    RescaleVideoSpec out;
    out.fmt = AV_PIX_FMT_RGB24;
    out.width = 640;
    out.height = 480;

    uint8_t image_buffer[_imgSize];
    in.imageBuffer = image_buffer;

    revertCurrentImage();

    if (_file->read((char *)image_buffer, _imgSize) > 0) {
        scale(in, out);
        _currentImage = new QImage(out.imageBuffer,
                                   out.width,
                                   out.height,
                                   QImage::Format_RGB888);
        update();
    } else {
        pause();
    }
}

void YuvPlayer::revertCurrentImage() {
    if (!_currentImage) return;
    free(_currentImage->bits()); // malloc <=> free
    delete _currentImage; // new <=> delete
    _currentImage = nullptr;
}

void YuvPlayer::scale(RescaleVideoSpec &in, RescaleVideoSpec &out) {
    SwsContext *ctx = sws_getContext(in.width,
                                     in.height,
                                     in.fmt,
                                     out.width,
                                     out.height,
                                     out.fmt,
                                     SWS_BILINEAR,
                                     nullptr,
                                     nullptr,
                                     nullptr);

    uint8_t *srcData[4], *dstData[4];
    int srcLineSize[4], dstLineSize[4];
    int ret = av_image_alloc(srcData, srcLineSize, in.width, in.height, in.fmt, 1);
    if (ret < 0) {
        ERROR_BUF(ret);
        return;
    }

    ret = av_image_alloc(dstData, dstLineSize, out.width, out.height, out.fmt, 1);
    if (ret < 0) {
        ERROR_BUF(ret);
        return;
    }

    int in_image_buffer_size = av_image_get_buffer_size(in.fmt, in.width, in.height, 1);
    int out_image_buffer_size = av_image_get_buffer_size(out.fmt, out.width, out.height, 1);

    memcpy(srcData[0], in.imageBuffer, in_image_buffer_size);

    int height = sws_scale(ctx, srcData, srcLineSize, 0, in.height, dstData, dstLineSize);
    qDebug() << "sws_scale height" << height;
    if (height != out.height) {
        return;
    }

    out.imageBuffer = (uint8_t *)malloc(out_image_buffer_size);
    memcpy(out.imageBuffer, dstData[0], out_image_buffer_size);

    av_freep(&srcData[0]);
    av_freep(&dstData[0]);
    sws_freeContext(ctx);
}

void YuvPlayer::pause() {
    if (_startPlayTimerId == 0) return;
    killTimer(_startPlayTimerId);
}

void YuvPlayer::destory() {
    pause();
    _file->close();
    _file = nullptr;
    revertCurrentImage();
}

/// todo
void YuvPlayer::seek() {

}
