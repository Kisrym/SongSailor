#ifndef MUSICITEM_H
#define MUSICITEM_H

#include <QString>
#include <QPixmap>
#include <QByteArray>

class MusicItem
{
public:
    MusicItem(const QString &title, const QString &artist, const QByteArray &albumCoverData);

    QString title() const;
    QString artist() const;
    QByteArray albumCover() const;

private:
    QString m_title;
    QString m_artist;
    QByteArray m_albumCover;

};

#endif // MUSICITEM_H
