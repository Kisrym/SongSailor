#include "musicitem.h"

MusicItem::MusicItem(const QString &title, const QString &artist, const QByteArray &albumCoverData)
    : m_title(title), m_albumCover(albumCoverData), m_artist(artist)
{}

QString MusicItem::title() const {
    return m_title;
}

QString MusicItem::artist() const {
    return m_artist;
}

QByteArray MusicItem::albumCover() const {
    return m_albumCover;
}
