#ifndef DECODER_H
#define DECODER_H

#include <QString>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <atomic>
#include <thread>
#include <mutex>
#include <map>
#include <QTimer>
#include <condition_variable>
#include <QObject>

#include <QDebug>

class Player;

class StateHolder : public QObject
{
    Q_OBJECT
public:
    enum class PlaybackState { NotLoaded, Loading, Loaded, Paused, Playing, Error };

    Q_ENUM(PlaybackState)
};

struct FrameHandle
{
    AVFrame *frame { nullptr };
    uint8_t *frameBuffer { nullptr };
};

class PlayerControl : public QObject
{
    Q_OBJECT
public:
    PlayerControl();
    ~PlayerControl();

    void init(const QString &, Player *p);
    void deinit();

    void play();
    void pause();
    void setCurrentFrame(uint32_t frameIndex);
    void nextFrame();

    FrameHandle currentFrame() const;

    StateHolder::PlaybackState state()    const;
    int64_t duration()       const;
    int64_t frameCount()     const;
    uint32_t framePosition() const;
    QString source()         const
      { return QString::fromStdString(video); }

    mutable std::recursive_mutex lock;

signals:
    void stateChanged();
    void frameChanged();
    void durationChanged();
    void frameCountChanged();
    void sourceChanged();

private:
    friend void decoderThread(PlayerControl *);
    friend bool initDecoder(PlayerControl *);

    void setState(StateHolder::PlaybackState state);
    void setFrameCount(int64_t frameCount);
    void setDuration(int64_t ms);
    void clearBuffer();

    uint32_t m_playbackFramePosition { 0 }; // current position [frame index]
    int64_t m_duration               { 0 }; // ms
    int64_t m_frameCount             { 0 }; // not possible or seek to file end [frames count]
    int m_timerInterval              { 0 };

    StateHolder::PlaybackState m_state;

    std::string video;

    std::atomic<bool> quit   { false };
    std::map<int64_t, FrameHandle> buffer;

    std::thread *thread { nullptr };
    std::condition_variable_any condWait;

    QTimer frameRateTimer;

    int             numBytes    { 0 };
    
    uint8_t         *pictBuffer { nullptr };
};

#endif // DECODER_H
