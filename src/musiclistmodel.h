#ifndef MUSICLISTMODEL_H
#define MUSICLISTMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QMap>
#include "musicitem.h"

class MusicListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    MusicListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void addItem(const MusicItem &item);
    bool itemExists(const MusicItem &item) const;
    void clear();

private:
    QList<MusicItem> m_items;
    QMap<QString, QPixmap> m_imageCache;

    QPixmap loadAndCacheImage(const MusicItem &item) const;
};

#endif // MUSICLISTMODEL_H
