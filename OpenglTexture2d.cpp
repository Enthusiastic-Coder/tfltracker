#include <iostream>
#include <QString>
#include "OpenGLTexture2D.h"
#include <QDebug>

OpenGLTexture2D::OpenGLTexture2D()
{
}

OpenGLTexture2D::~OpenGLTexture2D()
{
    clear();
}

void OpenGLTexture2D::clear()
{
    if( _texture_id != -1)
    {
        glDeleteTextures(1, &_texture_id);
        _texture_id = -1;
    }
}

void OpenGLTexture2D::setMinMag(int minn, int magg)
{
    _minification = minn;
    _magnification = magg;

    if( _texture_id != -1)
    {
        bind();
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,_magnification);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,_minification);

    }
}

void OpenGLTexture2D::setWrapMode(int i)
{
    _wrapMode = i;
}

bool OpenGLTexture2D::generate(int width, int height, bool isDepth)
{
    if(_texture_id != 0)
        glDeleteTextures(1, &_texture_id);

    _texture_id = createTexture(width, height, isDepth, _minification, _magnification, _wrapMode);

    _width = width;
    _height = height;

    return glGetError() == GL_NO_ERROR;
}

unsigned int OpenGLTexture2D::loadTexture(const QString& filename,int minTex, int maxTex, int wrap, int* width, int* height)
{
    unsigned int num = -1;
    num = texture_loadTexture(filename, minTex, maxTex, wrap, width, height);
    if( minTex != GL_LINEAR && minTex != GL_NEAREST)
        glGenerateMipmap(GL_TEXTURE_2D);

    return num;
}

unsigned int OpenGLTexture2D::createTexture(int w,int h,bool isDepth, int minTex, int maxTex, int wrap)
{
   unsigned int textureId;
    glGenTextures(1,&textureId);
    glBindTexture(GL_TEXTURE_2D,textureId);

    if ( isDepth)
    {
#ifdef ANDROID
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

#else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,maxTex);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,minTex);
#endif
    }
    else
    {
#ifdef ANDROID
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
#else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_FLOAT, 0);
#endif
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,maxTex);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,minTex);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLenum i = glGetError();
    if(i!=0)
    {
        printf( "(%s,%d) [%d]- Error happened while loading the texture: ", __FUNCTION__, __LINE__, i );
    }
    glBindTexture(GL_TEXTURE_2D,0);
    return textureId;
}

bool OpenGLTexture2D::Load(const QImage &img)
{
    QOpenGLFunctions::initializeOpenGLFunctions();
    clear();
    _texture_id = texture_loadTexture(img, _minification,_magnification, _wrapMode, &_width, &_height);
    return _texture_id !=0;
}

bool OpenGLTexture2D::Load(const QString &sfilename)
{
    QOpenGLFunctions::initializeOpenGLFunctions();
    clear();
    _texture_id = loadTexture(sfilename, _minification,_magnification, _wrapMode, &_width, &_height);
    return _texture_id !=0;
}

GLuint OpenGLTexture2D::getId() const
{
    return _texture_id;
}

void OpenGLTexture2D::bind()
{
    glBindTexture(GL_TEXTURE_2D, _texture_id);
}

int OpenGLTexture2D::width() const
{
    return _width;
}

int OpenGLTexture2D::height() const
{
    return _height;
}

OpenGLTexture2D::operator bool()
{
    return _texture_id != 0;
}

OpenGLTexture2D::operator GLuint()
{
    return _texture_id;
}

//////////////////////


unsigned int OpenGLTexture2D::texture_loadTexture(const QString& filename, int minTex, int maxTex, int wrap, int* width, int* height)
{
    QImage img = QImage(filename);

    if( img.isNull())
    {
        qDebug() << filename << " failed to load.";
        return -1;
    }

    return texture_loadTexture(img, minTex, maxTex, wrap, width, height);

}

unsigned int OpenGLTexture2D::texture_loadTexture(const QImage& origImg, int minTex, int maxTex, int wrap, int* width, int* height)
{
    QImage img = origImg.mirrored().convertToFormat(QImage::Format_RGBA8888);

    unsigned int num = -1;
    glGenTextures(1,&num);

    glBindTexture(GL_TEXTURE_2D,num);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,maxTex);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,minTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, img.width(), img.height(),
                    0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());

    *width = img.width();
    *height = img.height();
    return num;
}
