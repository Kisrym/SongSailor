#ifndef SONGSAILOR_H
#define SONGSAILOR_H

#include <QMainWindow>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QAudioOutput>

#include "src/musiclistmodel.h"
#include "src/buttonhoverwatcher.h"
#include "src/database.h"
#include "ui_songsailor.h"
#include "config.h"

QT_BEGIN_NAMESPACE
namespace Ui { class songsailor; }
QT_END_NAMESPACE

class songsailor : public QMainWindow
{
    Q_OBJECT

public:
    songsailor(QWidget *parent = nullptr);
    ~songsailor();
    void instalarMusicas(QString audio, QString type);
    QString removerAcentos(const QString &texto);

private slots:
    void musica_text_changed(const QString musica);
    void install();
    void add_music_in_list(); // vai mexer nos sinais internos do programa, deve ser declarado aqui (private slots)
    void change_config();
    void add_directory();
    void currentListViewChanged();

    //Player
    void play();
    void changeStateIcon(QMediaPlayer::PlaybackState estado);
    void musicEnded(QMediaPlayer::PlaybackState estado);
    void createPlaylist();
    void loop();
    void retroceder();

    //Playlist
    void playlistPag();
    void nameDescriptionChanged();
    void loadPlaylists();

    //context menu
    void showContextMenuMusicasPlayer(const QPoint &pos);
    void showContextMenuPlaylists(const QPoint &pos);
    void createPlaylistAct(); // nome da música vinda da database

    // config slot
    void config_slot(QString client_id, QString secret_id, QString refresh_token);

private:
    Ui::songsailor *ui;
    MusicListModel *model = new MusicListModel(this);
    MusicListModel *playlistModel = new MusicListModel(this);
    QAbstractItemModel *currentModel;
    ButtonHoverWatcher *watcher = new ButtonHoverWatcher(this);
    Database db;

    QMap<QString, QString> musicasPlayer; // nome : link_url
    QList<QString> reproduction_list; // qual música será tocada
    QStringList playlist; // playlist inteira
    QString nomeMusicaAtual;

    QMediaPlayer *tocador;
    QAudioOutput *audioOutput;
    QListView *currentListView; // lista de músicas atual

    bool isPlaylist = false;

    config *janela = new config;
};
#endif // SONGSAILOR_H
