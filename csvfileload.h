#ifndef CSVFILELOAD_H
#define CSVFILELOAD_H

#include <QStringList>
#include <QFile>
#include <QDebug>

template<typename T, class U=QHash<QString, T>>
class CSVFileLoad
{
public:
    virtual ~CSVFileLoad() = default;
    bool Load(const QString &sFilename, const int fieldCount, char separator=',');
    void interruptLoad();
    bool hasLoaded() const;
    void resetHasLoaded();
    QString getFilename() const;

    virtual void onLine(int lineNo, const QStringList& args) = 0;

    const T& operator[](QString id) const;

protected:
    U _data;

private:
    bool _bInterruptLoad = false;
    volatile bool _bHasLoaded = false;
    QString _filename;
};

template<typename T, class U>
bool CSVFileLoad<T,U>::Load(const QString& sFilename, const int fieldCount, char separator)
{
    _bHasLoaded = false;
    _filename = sFilename;

    QFile inFile(sFilename);
    if(!inFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed to load : " << sFilename;
        return false;
    }

    int iLineNo = 0;
    std::vector<std::string> dataLine;
    dataLine.resize(fieldCount);

    while (!inFile.atEnd())
    {
        if( _bInterruptLoad )
            return false;

        QString line = inFile.readLine();
        line = line.trimmed();

        if( line.isEmpty())
            continue;

        line.remove('\n');
        line.remove('"');

        onLine(iLineNo, line.split(separator));
        iLineNo++;
    }

    _bHasLoaded = true;

    return true;
}

template<typename T,class U>
void CSVFileLoad<T,U>::interruptLoad()
{
    _bInterruptLoad = true;
}

template<typename T,class U>
bool CSVFileLoad<T,U>::hasLoaded() const
{
    return _bHasLoaded;
}

template<typename T,class U>
void CSVFileLoad<T,U>::resetHasLoaded()
{
    _bHasLoaded = false;
}

template<typename T,class U>
QString CSVFileLoad<T,U>::getFilename() const
{
    return _filename;
}

template<typename T,class U>
const T &CSVFileLoad<T,U>::operator[](QString id) const
{
    auto it = _data.find(id);
    if( it == _data.end())
    {
        static T empty = {};
        return empty;
    }

    return *it;
}



#endif // CSVFILELOAD_H
