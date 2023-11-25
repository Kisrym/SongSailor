#include "database.h"

#include <QSqlError>

Database::Database()
{
    db = QSqlDatabase::addDatabase("QMYSQL");

    db.setDatabaseName("sql10663443");
    db.setHostName("sql10.freemysqlhosting.net");
    db.setPort(3306);
    db.setUserName("sql10663443");
    db.setPassword("qpLGVtahwv");

    if (db.open()) {
        QSqlQuery query;
        query.exec("CREATE TABLE IF NOT EXISTS dados (id INT AUTO_INCREMENT PRIMARY KEY, nome VARCHAR(30), descricao VARCHAR(30), musicas VARCHAR(300))");
    }
    else {
        qDebug() << "Erro ao conectar ao banco de dados:";
        qDebug() << db.lastError().text();
    }
}

Database::~Database(){
    db.close();
}

void Database::createPlaylistDb(QString nome, QString descricao) const{
    QSqlQuery query;
    query.prepare("INSERT INTO dados (nome, descricao, musicas) VALUES (?, ?, ?)");
    query.addBindValue(nome);
    query.addBindValue(descricao);
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

        listaPlaylist.append(playlist);
    }

    return listaPlaylist;
}

void Database::alterPlaylistDb(QString novoNome, QString novaDescricao, Editing editando) const{
    QSqlQuery query;

    if (editando == Description){
        query.prepare("UPDATE dados SET descricao = ? WHERE nome = ?");
        query.addBindValue(novaDescricao);
        query.addBindValue(novoNome);
    }
    if (editando == Name){
        query.prepare("UPDATE dados SET nome = ? WHERE nome = ?");
        query.addBindValue(novoNome);
    }

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
    qDebug() << db.lastError().text();
};
