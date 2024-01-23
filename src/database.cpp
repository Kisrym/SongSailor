#include "database.h"

#include <QSqlError>

Database::Database()
{
    db = QSqlDatabase::addDatabase("QMYSQL");

    db.setDatabaseName("songsailor");
    db.setHostName("127.0.0.1");
    db.setPort(3306);
    db.setUserName("root");
    db.setPassword("kaio1234");

    if (db.open()) {
        QSqlQuery query;
        query.exec("CREATE TABLE IF NOT EXISTS dados (id INT AUTO_INCREMENT PRIMARY KEY, nome VARCHAR(30), descricao VARCHAR(30), musicas VARCHAR(300), imagem LONGBLOB)");
    }
    else {
        qDebug() << "Erro ao conectar ao banco de dados:";
        qDebug() << db.lastError().text();
    }

    this->deleteMusicPlaylistsDb("Benzim");
}

Database::~Database(){
    db.close();
}

void Database::createPlaylistDb(QString nome, QByteArray imagem, QString descricao) const{
    QSqlQuery query;
    query.prepare("INSERT INTO dados (nome, descricao, imagem, musicas) VALUES (?, ?, ?, ?)");
    query.addBindValue(nome);
    query.addBindValue(descricao);
    query.addBindValue(imagem);
    query.addBindValue("");

    query.exec();
}

QList<Database::Playlist> Database::loadPlaylistDb() const{
    QList<Playlist> listaPlaylist;
    Playlist playlist;
    QSqlQuery query;

    query.exec("SELECT * FROM dados");
    while (query.next()){
        playlist.id = query.value("id").toInt();
        playlist.nome = query.value("nome").toString();
        playlist.descricao = query.value("descricao").toString();
        playlist.musicas = query.value("musicas").toString().split(";");
        playlist.imagem = query.value("imagem").toByteArray();

        listaPlaylist.append(playlist);
    }

    return listaPlaylist;
}

void Database::alterPlaylistDb(QStringList playlist, QString novoNome, QString novaDescricao) const{
    QSqlQuery query;
    QString musicas = playlist.join(";") + ";";

    query.prepare("UPDATE dados SET descricao = ?, nome = ? WHERE musicas = ?");
    query.addBindValue(novaDescricao);
    query.addBindValue(novoNome);
    query.addBindValue(musicas);

    query.exec();
}

void Database::deletePlaylistDb(QString nome) const{
    QSqlQuery query;
    query.prepare("DELETE FROM dados WHERE nome = ?");
    query.addBindValue(nome);

    query.exec();
}

Database::Playlist Database::locatePlaylistDb(QString nome) const{
    QSqlQuery query;
    Playlist playlist;

    query.prepare("SELECT * FROM dados WHERE nome = ?");
    query.addBindValue(nome);

    query.exec();
    while (query.next()){
        playlist.id = query.value("id").toInt();
        playlist.nome = query.value("nome").toString();
        playlist.descricao = query.value("descricao").toString();
        playlist.musicas = query.value("musicas").toString().split(";");
        playlist.imagem = query.value("imagem").toByteArray();
    }

    return playlist;
}

void Database::addMusicPlaylistDb(QString nomePlaylist, QString nomeMusica) const{
    QSqlQuery query;
    nomeMusica = nomeMusica + ";";

    query.prepare("UPDATE dados SET musicas = CONCAT(musicas, ?) WHERE nome = ?");
    query.addBindValue(nomeMusica);
    query.addBindValue(nomePlaylist);

    query.exec();
};

void Database::deleteMusicPlaylistsDb(QString nomeMusica) const{
    QSqlQuery query;
    QSqlQuery query2;
    nomeMusica = nomeMusica + ";";

    query.exec("SELECT * FROM dados");
    while (query.next()){
        if (query.value("musicas").toString().contains(nomeMusica)){
            QString novasMusicas = query.value("musicas").toString().replace(nomeMusica, "");
            qDebug() << novasMusicas;
            query2.prepare("UPDATE dados SET musicas = ? WHERE nome = ?");
            query2.addBindValue(novasMusicas);
            query2.addBindValue(query.value("nome").toString());
            query2.exec();
        }
    }
}
