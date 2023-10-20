#include "musiclistmodel.h"

MusicListModel::MusicListModel(QObject *parent)
    : QAbstractListModel(parent)
{}

int MusicListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_items.size();
}

QVariant MusicListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const MusicItem &item = m_items[index.row()];

    if (role == Qt::DisplayRole)
        return item.title() + "\n" + item.artist();
    else if (role == Qt::DecorationRole)
    {
        QPixmap albumCover;
        albumCover.loadFromData(item.albumCover());
        return albumCover.scaled(QSize(70, 70), Qt::IgnoreAspectRatio);
    }
    return QVariant();
}

void MusicListModel::addItem(const MusicItem &item){
    int newRow = rowCount();

    beginInsertRows(QModelIndex(), newRow, newRow); // updates the model
    if (this->itemExists(item) == false){
        m_items.append(item);
    }
    endInsertRows();
}

bool MusicListModel::itemExists(const MusicItem &item) const{
    for (const MusicItem &otherItem : m_items){
        if (otherItem.title() == item.title()){
            return true;
        }
    }
    return false;
}
