#ifndef MUSICLISTMODEL_H
#define MUSICLISTMODEL_H

#include <QObject>
#include <QAbstractListModel>

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

private:
    QList<MusicItem> m_items;
};

#endif // MUSICLISTMODEL_H
