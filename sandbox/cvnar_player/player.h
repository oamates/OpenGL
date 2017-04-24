#ifndef PLAYER_H
#define PLAYER_H

#include <QtQuick/QQuickFramebufferObject>

#include "decoder.h"

class PlayerSurfaceRenderer;

class Player : public QQuickFramebufferObject
{
    Q_OBJECT
public:
    Player();
    ~Player();

    Renderer *createRenderer() const override;

    Q_PROPERTY(QString source READ getSource WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(int duration READ getDuration NOTIFY durationChanged)
    Q_PROPERTY(int frameCount READ getFrameCount NOTIFY frameCountChanged)
    Q_PROPERTY(unsigned int framePosition READ getFramePisition NOTIFY framePositionChanged)
    Q_PROPERTY(StateHolder::PlaybackState state READ getState NOTIFY stateChanged)

    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void setCurrentFrame(unsigned int frameIndex);
    Q_INVOKABLE void nextFrame();

    FrameHandle getCurrentFrame();

    void setSource(const QString &value);
    QString getSource()                          const { return decoder.source();        }
    int getDuration()                            const { return decoder.duration();      }
    int getFrameCount()                          const { return decoder.frameCount();    }
    unsigned int getFramePisition()              const { return decoder.framePosition(); }
    StateHolder::PlaybackState getState()        const { return decoder.state();         }

private:
    //! TODO: what about sync? Sync needed on render when we apply seek command.
    friend  PlayerSurfaceRenderer;
    void lock()   { decoder.lock.lock(); }
    void unlock() { decoder.lock.unlock(); }

signals:
    void durationChanged();
    void positionChanged();
    void frameCountChanged();
    void sourceChanged();
    void framePositionChanged();
    void stateChanged();

private:
    PlayerControl decoder;

    //! Needed for directly update on frame changed instead of re-creation of renderer.
    mutable PlayerSurfaceRenderer *m_renderer;
};

#endif // PLAYER_H
