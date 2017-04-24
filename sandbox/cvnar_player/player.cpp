#include "player.h"
#include "shader.hpp"
#include "utils.h"

#include <QtGui/QOpenGLFramebufferObject>

#include <QtQuick/QQuickWindow>
#include <qsgsimpletexturenode.h>
#include <QOpenGLFunctions_3_3_Compatibility>

#include <QOpenGLFunctions>
//#include <QGLWidget>

#include <QPainter>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QThread>

//#define DUMP_FRAME

class PlayerSurfaceRenderer : public QQuickFramebufferObject::Renderer
{
public:
    PlayerSurfaceRenderer()
        : textures_initialized(false), texture_y(0), texture_u(0), texture_v(0)
    {
        f.reset( new QOpenGLFunctions_3_3_Core);
        auto openGLInit = f->initializeOpenGLFunctions();
        ms_assert(openGLInit);

        try {
            yuv420p_converter.init(f.get(), ":/res_int/shaders/player/yuv420p_conv.vs", ":/res_int/shaders/player/yuv420p_conv.fs");
            yuv420p_converter.enable();
        } catch(...) {
            qDebug() << "nu ebana rota";
            throw;
        }
        // important : texture unit 0 is occupied by QT and should not be touched!
        f->glUniform1i(yuv420p_converter["y_channel"], 1);      // luminance data will be fed through texture unit 1
        f->glUniform1i(yuv420p_converter["u_channel"], 2);      // chrominance data will use texture unit 2 ...
        f->glUniform1i(yuv420p_converter["v_channel"], 3);      //        ... and texture unit 3

        yuv420p_converter.disable();                            // protect shader from QT -- equivalent to glUseProgram(0)

        float uvs[] =                                           // full screen(viewport) quad ...
        {
                -1.0f, -1.0f,
                1.0f, -1.0f,
                1.0f,  1.0f,
                -1.0f,  1.0f
        };

        f->glGenVertexArrays(1, &vao_id);                       // ... packed into vertex array object
        f->glBindVertexArray(vao_id);
        f->glGenBuffers(1, &vbo_id);
        f->glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        f->glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

        f->glBindTexture(GL_TEXTURE_2D, 0);                     // protect our data from QT scene graph bitch
        f->glBindBuffer(GL_ARRAY_BUFFER, 0);                    // it really destroys buffer and
        f->glBindVertexArray(0);                                //        VAO content if you don't hide it

    }

    GLuint single_channel_texture(GLsizei width, GLsizei height)
    {
        GLuint texture_id;
        f->glGenTextures(1, &texture_id);
        f->glBindTexture(GL_TEXTURE_2D, texture_id);
        f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        return texture_id;
    }

    ~PlayerSurfaceRenderer()
    {
        qDebug("PlayerSurfaceRenderer :: Destructor");
        f->glDeleteTextures(1, &texture_y);
        f->glDeleteTextures(1, &texture_u);
        f->glDeleteTextures(1, &texture_v);
        f->glDeleteBuffers(1, &vbo_id);
        f->glDeleteVertexArrays(1, &vao_id);

        //delete f;
        //f = nullptr;
    }

    void doUpdate() { update(); }

    void synchronize(QQuickFramebufferObject *obj) override {
        qDebug() << Q_FUNC_INFO;
        player = dynamic_cast<Player*> (obj);

        //! TODO: ms_assert
        if (!player) {
            qDebug() << "Critical error";
        }
    }

    void render() override {

        if(!player) return;

        int item_width  = player->width();
        int item_height = player->height();
        if (item_width <= 0 || item_height <= 0) return;

        player->lock();

        auto handle = player->getCurrentFrame();

        if (handle.frame == nullptr) {
            player->unlock();
            update();
            return;
        }

        currentFrame = handle.frame;

        if (!textures_initialized) {
            texture_y = single_channel_texture(currentFrame->width, currentFrame->height);
            texture_u = single_channel_texture(currentFrame->linesize[1], (currentFrame->linesize[1] * currentFrame->height) / currentFrame->width);
            texture_v = single_channel_texture(currentFrame->linesize[2], (currentFrame->linesize[2] * currentFrame->height) / currentFrame->width);
            textures_initialized = true;
        }

#ifdef DUMP_FRAME
        static int frameName = 0;
        QImage image(currentFrame->width, currentFrame->height, QImage::Format_Indexed8);
        for (int y = 0;y < currentFrame->height; y++)
            memcpy(image.scanLine(y), currentFrame->data[0] + y * currentFrame->linesize[0], currentFrame->width);

        QVector<QRgb> colorTable(256);
        for(int i = 0; i < 256; i++)
            colorTable[i] = qRgb(i, i, i);
        image.setColorTable(colorTable);

        image.save("luma" + QString::number(frameName) + ".jpg");
        ++frameName;
#endif

        f->glActiveTexture(GL_TEXTURE0);
        f->glBindTexture(GL_TEXTURE_2D, this->framebufferObject()->texture());          // without this bind nothing works after FBO resize
        f->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        f->glClear(GL_COLOR_BUFFER_BIT);

        int video_width  = currentFrame->width;                                         // Set viewport to preserve video aspect
        int video_height = currentFrame->height;

        double video_aspect = double(video_width) / video_height;
        double item_aspect  = double(item_width)  / item_height;

        GLint x, y;
        GLsizei w, h;

        if (video_aspect >= item_aspect)
        {
            x = 0;
            h = item_width / video_aspect;
            y = (item_height - h) / 2;
            w = item_width;
        }
        else
        {
            y = 0;
            w = item_height * video_aspect;
            x = (item_width - w) / 2;
            h = item_height;
        }

        f->glViewport(x, y, w, h);

        // Set up luminance texture data
        f->glActiveTexture(GL_TEXTURE1);
        f->glBindTexture(GL_TEXTURE_2D, texture_y);
        f->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, video_width, video_height, GL_RED, GL_UNSIGNED_BYTE, currentFrame->data[0]);

        // Set up chrominance blue texture data
        f->glActiveTexture(GL_TEXTURE2);
        f->glBindTexture(GL_TEXTURE_2D, texture_u);
        f->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, currentFrame->linesize[1], (currentFrame->linesize[1] * video_height) / video_width,
                GL_RED, GL_UNSIGNED_BYTE, currentFrame->data[1]);

        // Set up chrominance red texture data
        f->glActiveTexture(GL_TEXTURE3);
        f->glBindTexture(GL_TEXTURE_2D, texture_v);
        f->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, currentFrame->linesize[2], (currentFrame->linesize[2] * video_height) / video_width,
                GL_RED, GL_UNSIGNED_BYTE, currentFrame->data[2]);

        yuv420p_converter.enable();

        f->glBindVertexArray(vao_id);
        f->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        yuv420p_converter.disable();

        f->glBindBuffer(GL_ARRAY_BUFFER, 0);
        f->glBindVertexArray(0);

        player->window()->resetOpenGLState();
        player->unlock();
    }

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override {
        QOpenGLFramebufferObjectFormat format;
        qDebug() << "PlayerSurfaceRenderer::createFramebufferObject called :: size = " << size;

        //player->setScaleSize(size.height(), size.width());
        return new QOpenGLFramebufferObject(size, format);
    }

    AVFrame *currentFrame { nullptr };
    Player *player { nullptr };
    //QOpenGLFunctions_3_3_Core *f { nullptr };

    GLSLProgram yuv420p_converter;
    bool textures_initialized;
    GLuint texture_y, texture_u, texture_v;
    GLuint vao_id, vbo_id;
};

Player::Player()
    : QQuickFramebufferObject()
{
    //decoder.init("C:/cvnar2.avi", this);
    //decoder.init("C:/qnx_orig.avi", this);
    //decoder.init("C:/raw.avi", this);
    //decoder.init("C:/raw_coded.avi", this);

    connect(&decoder, &PlayerControl::frameChanged,      this, [this]() { if(m_renderer) m_renderer->doUpdate();});
    connect(&decoder, &PlayerControl::frameChanged,      this, &Player::framePositionChanged, Qt::QueuedConnection);
    connect(&decoder, &PlayerControl::stateChanged,      this, &Player::stateChanged, Qt::QueuedConnection);
    connect(&decoder, &PlayerControl::frameCountChanged, this, &Player::frameCountChanged, Qt::QueuedConnection);
    connect(&decoder, &PlayerControl::sourceChanged,     this, &Player::sourceChanged, Qt::QueuedConnection);
    connect(&decoder, &PlayerControl::durationChanged,   this, &Player::durationChanged, Qt::QueuedConnection);
}

Player::~Player()
{
    qDebug() << "Player::~Player is being destroied";
}

void Player::play()
{
    decoder.play();
}

void Player::pause()
{
    qDebug() << "[ Player ] pause()";
    decoder.pause();
}

void Player::setCurrentFrame(unsigned int frameIndex)
{
    decoder.setCurrentFrame(frameIndex);
}

void Player::nextFrame()
{
    decoder.nextFrame();
}

FrameHandle Player::getCurrentFrame()
{
    return decoder.currentFrame();
}

QQuickFramebufferObject::Renderer *Player::createRenderer() const
{
    m_renderer = new PlayerSurfaceRenderer();

    static int totalRendererCount = 0;  ++totalRendererCount;
    qDebug() << "[ Player ] new renderer has been created, total renderer count : " << totalRendererCount;
    return m_renderer;
}

void Player::setSource(const QString &value)
{
    qDebug() << Q_FUNC_INFO;
    if (value == decoder.source())
        return;

    qDebug() << "[ Player ] current state = " << decoder.state();
    if (decoder.state() != StateHolder::PlaybackState::NotLoaded) {
        qDebug() << "[ Player ] deinit called";
        decoder.deinit();
    }

    decoder.init(value, this);
}
