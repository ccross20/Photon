#include <QDebug>
#include <QFile>
#include <QColor>
#include <QMatrix4x4>
#include "openglshader.h"

namespace photon {

OpenGLShader::OpenGLShader(QOpenGLContext *context, const QString &vertex, const QString &fragment) : m_context(context)
{

    QFile vert(vertex);
    if(!vert.open(QFile::ReadOnly))
        qDebug() << "Error opening vertex shader";


    QFile frag(fragment);
    if(!frag.open(QFile::ReadOnly))
        qDebug() << "Error opening fragment shader";

    auto f = context->functions();


    uint v_id, f_id;
    int success;
    char infoLog[512];

    QByteArray vertSource = vert.readAll();
    //qDebug() << vertSource;

    const char *vertexSource = vertSource.constData();
    v_id = f->glCreateShader(GL_VERTEX_SHADER);
    f->glShaderSource(v_id, 1, &vertexSource, NULL);
    f->glCompileShader(v_id);


    f->glGetShaderiv(v_id, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        f->glGetShaderInfoLog(v_id, 512, NULL, infoLog);
        qDebug() << "Vertex Shader Error: " << infoLog;
    }



    QByteArray fragSource = frag.readAll();
    const char *fragmentSource = fragSource.constData();
    f_id = f->glCreateShader(GL_FRAGMENT_SHADER);
    f->glShaderSource(f_id, 1, &fragmentSource, NULL);
    f->glCompileShader(f_id);

    f->glGetShaderiv(f_id, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        f->glGetShaderInfoLog(f_id, 512, NULL, infoLog);
        qDebug() << fragment << "  Fragment Shader Error: " << infoLog;
    }


    m_id = f->glCreateProgram();
    f->glAttachShader(m_id, v_id);
    f->glAttachShader(m_id, f_id);
    f->glLinkProgram(m_id);

    f->glGetProgramiv(m_id, GL_LINK_STATUS, &success);

    if(!success)
    {
        f->glGetProgramInfoLog(m_id, 512, NULL, infoLog);
        qDebug() << fragment << " Link Error: " << infoLog;
    }

    f->glDeleteShader(v_id);
    f->glDeleteShader(f_id);

}

OpenGLShader::~OpenGLShader()
{
    m_context->functions()->glDeleteShader(m_id);
}

unsigned int OpenGLShader::id() const
{
    return m_id;
}

void OpenGLShader::bind(QOpenGLContext *context)
{
    if(context)
        m_context = context;
    else if(!m_context)
        m_context = QOpenGLContext::currentContext();
    m_context->functions()->glUseProgram(m_id);
}

void OpenGLShader::unbind(QOpenGLContext *context)
{
    if(context)
        m_context = context;
    else if(!m_context)
        m_context = QOpenGLContext::currentContext();
    m_context->functions()->glUseProgram(0);
}


GLint OpenGLShader::uniformLocation(const QByteArray &name)
{
    return m_context->functions()->glGetUniformLocation(m_id, name.constData());
}

void OpenGLShader::setBool(const QByteArray &name, bool value)
{
    auto f = m_context->functions();
    f->glUniform1i(f->glGetUniformLocation(m_id, name.toStdString().c_str()), (int)value);
}

void OpenGLShader::setInt(const QByteArray &name, int value)
{
    auto f = m_context->functions();
    f->glUniform1i(f->glGetUniformLocation(m_id, name.toStdString().c_str()), value);
}

void OpenGLShader::setFloat(const QByteArray &name, float value)
{
    auto f = m_context->functions();
    f->glUniform1f(f->glGetUniformLocation(m_id, name.toStdString().c_str()), value);
}

void OpenGLShader::setFloat2(const QByteArray &name, float a, float b)
{
    auto f = m_context->functions();
    f->glUniform2f(f->glGetUniformLocation(m_id, name.toStdString().c_str()),a,b);
}

void OpenGLShader::setFloat3(const QByteArray &name, float a, float b, float c)
{
    auto f = m_context->functions();
    f->glUniform3f(f->glGetUniformLocation(m_id, name.toStdString().c_str()),a,b,c);
}

void OpenGLShader::setInt2(const QByteArray &name, int a, int b)
{
    auto f = m_context->functions();
    f->glUniform2i(f->glGetUniformLocation(m_id, name.toStdString().c_str()),a,b);
}

void OpenGLShader::setInt3(const QByteArray &name, int a, int b, int c)
{
    auto f = m_context->functions();
    f->glUniform3i(f->glGetUniformLocation(m_id, name.toStdString().c_str()),a,b, c);
}

void OpenGLShader::setFloat4(const QByteArray &name, float a, float b, float c, float d)
{
    auto f = m_context->functions();
    f->glUniform4f(f->glGetUniformLocation(m_id, name.toStdString().c_str()),a,b,c,d);
}

void OpenGLShader::setColor(const QByteArray &name, const QColor &c, ColorFormat format)
{
    auto f = m_context->functions();
#if defined(Q_OS_WIN)
    if(format == ModeBGRA)
        f->glUniform4f(f->glGetUniformLocation(m_id, name.toStdString().c_str()),c.blueF(),c.greenF(),c.redF(),c.alphaF());
    else
        f->glUniform4f(f->glGetUniformLocation(m_id, name.toStdString().c_str()),c.redF(),c.greenF(),c.blueF(),c.alphaF());
#elif defined(Q_OS_MAC)
    f->glUniform4f(f->glGetUniformLocation(m_id, name.toStdString().c_str()),c.redF(),c.greenF(),c.blueF(),c.alphaF());
#endif
}

void OpenGLShader::setMatrix(const QByteArray &name, const QMatrix4x4 &matrix)
{
    auto f = m_context->functions();
    f->glUniformMatrix4fv(f->glGetUniformLocation(m_id, name.toStdString().c_str()), 1, GL_FALSE, matrix.data());
}

int OpenGLShader::setTexture(const QByteArray &name, int value, int target)
{
    auto f = m_context->functions();
    GLint loc = f->glGetUniformLocation(m_id, name.toStdString().c_str());

    if(loc != -1){

        for(UniformTexture &tex : m_textures)
        {
            if(tex.loc == loc)
            {
                tex.handle = value;
                f->glUniform1i(loc, tex.loc_idx);
                f->glActiveTexture(GL_TEXTURE0 + tex.loc_idx);
                f->glBindTexture(tex.target, tex.handle);
                return tex.loc_idx;
            }
        }

          UniformTexture untex = UniformTexture(name, loc, m_textures.length(), value, target);
          m_textures.append(untex);

          f->glUniform1i(loc, untex.loc_idx);
          f->glActiveTexture(GL_TEXTURE0 + untex.loc_idx);
          f->glBindTexture(untex.target, untex.handle);
        }

    return m_textures.length();
}

void OpenGLShader::clearTextures()
{
    //int counter =
    auto f = m_context->functions();
    for(UniformTexture &tex : m_textures)
    {
        f->glActiveTexture(GL_TEXTURE0 + tex.loc_idx);
        f->glBindTexture(tex.target, 0);
    }
    m_textures.clear();
}


void OpenGLShader::setBool(const GLint &loc, bool value)
{
    auto f = m_context->functions();
    f->glUniform1i(loc, (int)value);
}

void OpenGLShader::setInt(const GLint &loc, int value)
{
    auto f = m_context->functions();
    f->glUniform1i(loc, value);
}

void OpenGLShader::setFloat(const GLint &loc, float value)
{
    auto f = m_context->functions();
    f->glUniform1f(loc, value);
}

void OpenGLShader::setFloat2(const GLint &loc, float a, float b)
{
    auto f = m_context->functions();
    f->glUniform2f(loc,a,b);
}

void OpenGLShader::setFloat3(const GLint &loc, float a, float b, float c)
{
    auto f = m_context->functions();
    f->glUniform3f(loc,a,b,c);
}

void OpenGLShader::setFloat4(const GLint &loc, float a, float b, float c, float d)
{
    auto f = m_context->functions();
    f->glUniform4f(loc,a,b,c,d);
}

void OpenGLShader::setInt2(const GLint &loc, int a, int b)
{
    auto f = m_context->functions();
    f->glUniform2i(loc,a,b);
}

void OpenGLShader::setMatrix(const GLint &loc, const QMatrix4x4 &matrix)
{
    auto f = m_context->functions();
    f->glUniformMatrix4fv(loc, 1, GL_FALSE, matrix.data());
}

} // namespace exo
