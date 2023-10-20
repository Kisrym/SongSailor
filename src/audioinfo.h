#ifndef AUDIOINFO_H
#define AUDIOINFO_H

#include <QString>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include <fstream>
#include <QDir>
#include <QDebug>
#include <QByteArray>

class AudioInfo
{
private:
    TagLib::MPEG::File file;

public:
    AudioInfo(QString path);

    QString getTitle() const;
    QString getAuthor() const;
    QByteArray saveImage();
};

#endif // AUDIOINFO_H
