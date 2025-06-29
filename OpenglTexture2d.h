#ifndef OPENGLTEXTURE2D_H
#define OPENGLTEXTURE2D_H

#include <QString>
#include <QImage>
#include <QOpenGLFunctions>

class OpenGLTexture2D : private QOpenGLFunctions
{
public:

    OpenGLTexture2D();
    ~OpenGLTexture2D(void);

    OpenGLTexture2D(const OpenGLTexture2D& rhs) = delete;
    OpenGLTexture2D& operator=(const OpenGLTexture2D& rhs) = delete;

    void clear();
    void setMinMag(int minn, int magg);
    void setWrapMode(int i);
    bool generate(int width, int height, bool isDepth);
    bool Load(const QImage& img);
    bool Load(const QString& sfilename);
    GLuint getId() const;
    operator GLuint();
    operator bool();
    void bind();
    int width() const;
    int height() const;

private:
    unsigned int loadTexture(const QString &filename, int minTex, int maxTex, int wrap, int *width, int *height);
    unsigned int createTexture(int w, int h, bool isDepth, int minTex, int maxTex, int wrap);

    unsigned int texture_loadTexture(const QImage& origImg, int minTex, int maxTex, int wrap, int* width, int* height);
    unsigned int texture_loadTexture(const QString& filename, int minTex, int maxTex, int wrap, int* width, int* height);

private:
    GLuint _texture_id = -1;
    int _wrapMode = GL_REPEAT;
    int _minification = GL_LINEAR;
    int _magnification = GL_LINEAR;
    int _width = 0;
    int _height = 0;
};


#endif // OPENGLTEXTURE2D_H
