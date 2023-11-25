#ifndef DATABASE_H
#define DATABASE_H

#include <QSql>
#include <QSqlDatabase>
#include <QDebug>
#include <QSqlQuery>

class Database
{
public:
    Database();
    ~Database();

    struct Playlist {
        int id;
        QString nome;
        QString descricao;
        QStringList musicas;
    };

    enum Editing {Name, Description};

    void createPlaylistDb(QString nome, QString descricao = "Nova Playlist") const;
    QList<Playlist> loadPlaylistDb() const;
    void alterPlaylistDb(QString novoNome, QString novaDescricao, Editing editando) const;

    Playlist locatePlaylistDb(QString nome) const;
    void addMusicPlaylistDb(QString nomePlaylist, QString nomeMusica) const;

private:
    QSqlDatabase db;
};

#endif // DATABASE_H
