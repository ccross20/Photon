#ifndef DECO_OPENGLSHADER_H
#define DECO_OPENGLSHADER_H
#include <QString>
#include <QOpenGLFunctions>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT OpenGLShader
{
public:

    class UniformTexture
    {
    public:
        UniformTexture(const QByteArray &_name, int _loc, int _loc_idx, int _handle, int _target):name(_name),
            loc(_loc),loc_idx(_loc_idx),handle(_handle),target(_target){}
        QByteArray name;
        int loc = -1;
        int loc_idx = -1;
        int handle = -1;
        int target = -1;
    };

    enum ColorFormat
    {
        ModeBGRA,
        ModeRGBA
    };


    OpenGLShader(){}
    OpenGLShader(QOpenGLContext *context, const QString &vertex, const QString &fragment);
    ~OpenGLShader();

    unsigned int id() const;
    void bind(QOpenGLContext *context = nullptr);
    void unbind(QOpenGLContext *context = nullptr);

    void setBool(const QByteArray &name, bool value);
    void setInt(const QByteArray &name, int value);
    void setFloat(const QByteArray &name, float value);
    void setFloat2(const QByteArray &name, float a, float b);
    void setFloat3(const QByteArray &name, float a, float b, float c);
    void setInt2(const QByteArray &name, int a, int b);
    void setInt3(const QByteArray &name, int a, int b, int c);
    void setFloat4(const QByteArray &name, float a, float b, float c, float d);
    void setColor(const QByteArray &name, const QColor &c, ColorFormat format = ModeRGBA);
    void setMatrix(const QByteArray &name, const QMatrix4x4 &matrix);
    int setTexture(const QByteArray &name, int value, int target = GL_TEXTURE_2D);

    void setBool(const GLint &loc, bool value);
    void setInt(const GLint &loc, int value);
    void setFloat(const GLint &loc, float value);
    void setFloat2(const GLint &loc, float a, float b);
    void setFloat3(const GLint &loc, float a, float b, float c);
    void setFloat4(const GLint &loc, float a, float b, float c, float d);
    void setInt2(const GLint &loc, int a, int b);
    void setMatrix(const GLint &loc, const QMatrix4x4 &matrix);

    void clearTextures();

    GLint uniformLocation(const QByteArray &name);

private:
    QVector<UniformTexture> m_textures;
    unsigned int m_id;
    QOpenGLContext *m_context;
};

} // namespace exo

#endif // DECO_OPENGLSHADER_H
