#include "mpvobject.h"

#include <mpv/client.h>
#include <mpv/render_gl.h>
#include <qthelper.hpp>

#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QQuickOpenGLUtils>
#include <QQuickWindow>

class MpvObjectPrivate : public QObject {
    Q_OBJECT
public:
    MpvObjectPrivate(MpvObject* pParent);
    ~MpvObjectPrivate();

    mpv_handle* mpMpvHandle;
    mpv_render_context* mpMpvGlContext;

    MpvObject* mpParent;

    static void handleMpvUpdate(void* ctx);
    static void handleMpvEvent(void* ctx);

public slots:
    void handleUpdate();

signals:
    void update();
};

MpvObjectPrivate::MpvObjectPrivate(MpvObject* pParent)
    : QObject(nullptr)
    , mpParent(pParent)
    , mpMpvHandle(mpv_create())
    , mpMpvGlContext(nullptr)
{

    if (!mpMpvHandle) {
        throw std::runtime_error("could not create mpv context");
    }

    // Uncomment it to show more messages
    // mpv_set_option_string(mpMpvHandle, "terminal", "yes");
    // mpv_set_option_string(mpMpvHandle, "msg-level", "all=v");

    if (mpv_initialize(mpMpvHandle) < 0) {
        throw std::runtime_error("could not initialize mpv context");
    }

    mpv::qt::set_option_variant(mpMpvHandle, "vo", "libmpv"); // This option must be set otherwise mpv will open a new window

    connect(this, &MpvObjectPrivate::update, this, &MpvObjectPrivate::handleUpdate, Qt::QueuedConnection);
}

MpvObjectPrivate::~MpvObjectPrivate()
{
    if (mpMpvGlContext) // Only initialized if something got drawn
    {
        mpv_render_context_free(mpMpvGlContext);
    }

    mpv_terminate_destroy(mpMpvHandle);
}

void MpvObjectPrivate::handleMpvUpdate(void* ctx)
{
    MpvObjectPrivate* self = (MpvObjectPrivate*)ctx;
    emit self->update();
}

void MpvObjectPrivate::handleMpvEvent(void* ctx)
{
    Q_UNUSED(ctx)
}

void MpvObjectPrivate::handleUpdate()
{
    emit mpParent->update();
}

MpvObject::MpvObject(QQuickItem* parent)
    : QQuickFramebufferObject(parent)
    , mpPrivate(std::make_unique<MpvObjectPrivate>(this))
{
}

MpvObject::~MpvObject()
{
}

class MpvRenderer : public QQuickFramebufferObject::Renderer {

public:
    MpvRenderer(MpvObjectPrivate* pPrivate)
        : mpPrivate(pPrivate)
    {
        mpv_set_wakeup_callback(mpPrivate->mpMpvHandle, MpvObjectPrivate::handleMpvEvent, nullptr);
    }

    virtual ~MpvRenderer()
    {
    }

    // This function is called when a new FrameBufferObject is needed.
    // This happens on the initial frame.
    QOpenGLFramebufferObject* createFramebufferObject(const QSize& size)
    {
        // Initialize mpv_gl:
        if (!mpPrivate->mpMpvGlContext) {
            mpv_opengl_init_params openGLInitParams
            {
                [](void*, const char* name) -> void* {
                    QOpenGLContext* glctx = QOpenGLContext::currentContext();
                    return glctx ? reinterpret_cast<void*>(glctx->getProcAddress(QByteArray(name))) : nullptr;
                }
#if MPV_CLIENT_API_VERSION < MPV_MAKE_VERSION(2, 0)
                ,
                    nullptr, nullptr
#endif
            };
            mpv_render_param params[] {
                { MPV_RENDER_PARAM_API_TYPE, const_cast<char*>(MPV_RENDER_API_TYPE_OPENGL) },
                { MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &openGLInitParams },
                { MPV_RENDER_PARAM_INVALID, nullptr }
            };

            if (mpv_render_context_create(&mpPrivate->mpMpvGlContext, mpPrivate->mpMpvHandle, params) < 0) {
                throw std::runtime_error("failed to initialize mpv GL context");
            }
            mpv_render_context_set_update_callback(mpPrivate->mpMpvGlContext, MpvObjectPrivate::handleMpvUpdate, mpPrivate);
        }

        return QQuickFramebufferObject::Renderer::createFramebufferObject(size);
    }

    void render()
    {
        QQuickOpenGLUtils::resetOpenGLState();
        QOpenGLFramebufferObject* fbo = framebufferObject();
        mpv_opengl_fbo mpfbo { static_cast<int>(fbo->handle()), fbo->width(), fbo->height(), 0 };
        int flip_y { 0 };

        mpv_render_param params[] = {
            // Specify the default framebuffer (0) as target. This will
            // render onto the entire screen. If you want to show the video
            // in a smaller rectangle or apply fancy transformations, you'll
            // need to render into a separate FBO and draw it manually.
            { MPV_RENDER_PARAM_OPENGL_FBO, &mpfbo },
            // Flip rendering (needed due to flipped GL coordinate system).
            { MPV_RENDER_PARAM_FLIP_Y, &flip_y },
            { MPV_RENDER_PARAM_INVALID, nullptr }
        };
        // See render_gl.h on what OpenGL environment mpv expects, and
        // other API details.
        mpv_render_context_render(mpPrivate->mpMpvGlContext, params);

        QQuickOpenGLUtils::resetOpenGLState();
    }

private:
    MpvObjectPrivate* mpPrivate;
};

QQuickFramebufferObject::Renderer* MpvObject::createRenderer() const
{
    window()->setPersistentGraphics(true);
    window()->setPersistentSceneGraph(true);
    return new MpvRenderer(mpPrivate.get());
}

void MpvObject::command(const QVariant& kParams)
{
    mpv::qt::command_variant(mpPrivate->mpMpvHandle, kParams);
}

void MpvObject::setProperty(const QString& kName, const QVariant& kValue)
{
    mpv::qt::set_property_variant(mpPrivate->mpMpvHandle, kName, kValue);
}

void MpvObject::open(const QUrl& kFileUrl)
{
    if (!kFileUrl.isLocalFile()) {
        return;
    }

    command(QVariantList { "loadfile", kFileUrl.toLocalFile() });
}

#include "mpvobject.moc"
