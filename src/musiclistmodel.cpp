#include "musiclistmodel.h"

MusicListModel::MusicListModel(QObject *parent)
    : QAbstractListModel(parent)
{}

int MusicListModel::rowCount(const QModelIndex &parent) const{
    if (parent.isValid())
        return 0;

    return m_items.size();
}

QVariant MusicListModel::data(const QModelIndex &index, int role) const{
    if (!index.isValid())
        return QVariant();

    const MusicItem &item = m_items[index.row()];

    if (role == Qt::DisplayRole)
        return item.title() + "\n" + item.artist();
    else if (role == Qt::DecorationRole)
    {
        if (m_imageCache.contains(item.title()))
            return m_imageCache[item.title()]; // Recuperar do cache

        QPixmap albumCover = loadAndCacheImage(item);
        return albumCover.scaled(QSize(70, 70), Qt::IgnoreAspectRatio);
    }
    return QVariant();
}

void MusicListModel::addItem(const MusicItem &item){
    int newRow = rowCount();

    beginInsertRows(QModelIndex(), newRow, newRow);
    if (!itemExists(item))
        m_items.append(item);
    endInsertRows();
}

bool MusicListModel::itemExists(const MusicItem &item) const{
    for (const MusicItem &otherItem : m_items)
    {
        if (otherItem.title() == item.title())
            return true;
    }
    return false;
}

void MusicListModel::clear(){
    if (m_items.isEmpty())
        return;

    beginResetModel();
    m_items.clear();
    endResetModel();
}

QPixmap MusicListModel::loadAndCacheImage(const MusicItem &item) const{
    // carregar a imagem
    QPixmap albumCover;
    albumCover.loadFromData(item.albumCover());

    // redimensiona a imagem
    QSize iconSize(70, 70);
    QPixmap scaledCover = albumCover.scaled(iconSize, Qt::KeepAspectRatio);

    // adiciona ao cache
    const_cast<MusicListModel*>(this)->m_imageCache.insert(item.title(), scaledCover); // o const_cast desabilita temporariamente o "const" do item, para poder modificá-lo

    return scaledCover;
}

