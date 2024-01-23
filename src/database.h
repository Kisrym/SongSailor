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
        QByteArray imagem;
    };

    void createPlaylistDb(QString nome, QByteArray imagem, QString descricao = "Nova Playlist") const;
    QList<Playlist> loadPlaylistDb() const;
    void alterPlaylistDb(QStringList playlist, QString novoNome, QString novaDescricao) const;
    void deletePlaylistDb(QString nome) const;

    Playlist locatePlaylistDb(QString nome) const;
    void addMusicPlaylistDb(QString nomePlaylist, QString nomeMusica) const;
    void deleteMusicPlaylistsDb(QString nomeMusica) const;

private:
    QSqlDatabase db;
};

#endif // DATABASE_H
