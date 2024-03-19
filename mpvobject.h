#ifndef MPVOBJECT_H
#define MPVOBJECT_H

#include <QtQuick/QQuickFramebufferObject>
#include <memory>

class MpvObjectPrivate;

class MpvObject : public QQuickFramebufferObject {
    Q_OBJECT
public:
    explicit MpvObject(QQuickItem* parent = nullptr);
    virtual ~MpvObject();

    // Override from QQuickFramebufferObject
    virtual Renderer* createRenderer() const override;

    Q_INVOKABLE void command(const QVariant& kParams);
    Q_INVOKABLE void setProperty(const QString& kName, const QVariant& kValue);
    Q_INVOKABLE void open(const QUrl& kFileUrl);

private:
    std::unique_ptr<MpvObjectPrivate> mpPrivate;
};

#endif // MPVOBJECT_H
