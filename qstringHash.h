#ifndef QSTRINGHASH_H
#define QSTRINGHASH_H

#include <QObject>
#if QT_VERSION <= QT_VERSION_CHECK(5,13,2)
namespace std
{
  template<>
    struct hash<QString>
    {
      size_t operator()(const QString & s) const
      {
        return qHash(s);
      }
    };
}
#else
#include <QHashFunctions>
#endif


#endif // QSTRINGHASH_H
