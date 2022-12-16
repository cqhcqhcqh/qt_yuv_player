#ifndef YUVPLAYER_H
#define YUVPLAYER_H

#include <QWidget>
#include <QFile>
extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
}

typedef struct {
    char *path;
    int width;
    int height;
    AVPixelFormat fmt;
    int fps;
} YuvFile;

typedef struct {
    int width;
    int height;
    AVPixelFormat fmt;
    uint8_t *imageBuffer;
} RescaleVideoSpec;

class YuvPlayer : public QWidget
{
    Q_OBJECT
public:
    explicit YuvPlayer(QWidget *parent = nullptr);
    void play();
    void pause();
    void destory();
    void seek();
    void replace(YuvFile &in);
signals:
private:
    void timerEvent(QTimerEvent *event);
    YuvFile _in;
    QFile *_file = nullptr;
    int _startPlayTimerId = 0;
    void paintEvent(QPaintEvent *event);
    QImage *_currentImage = nullptr;
    void scale(RescaleVideoSpec &in, RescaleVideoSpec &out);
    void revertCurrentImage();
    int _interval;
    QRect _dstRect;
    int _imgSize;
};

#endif // YUVPLAYER_H
