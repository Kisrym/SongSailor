#ifndef SONGSAILOR_H
#define SONGSAILOR_H

#include <QMainWindow>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QAudioOutput>

#include "src/musiclistmodel.h"
#include "src/buttonhoverwatcher.h"
#include "ui_songsailor.h"

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
    void add_directory();

    //Player
    void play();
    void changeStateIcon(QMediaPlayer::PlaybackState estado);
    void musicEnded(QMediaPlayer::PlaybackState estado);
    void createPlaylist();
    void loop();
    void retroceder();

private:
    Ui::songsailor *ui;
    MusicListModel *model = new MusicListModel(this);
    ButtonHoverWatcher *watcher = new ButtonHoverWatcher(this);

    QMap<QString, QString> musicasPlayer;
    QList<QString> reproduction_list;
    QList<QString> playlist;
    QString nomeMusicaAtual;

    QMediaPlayer *tocador;
    QAudioOutput *audioOutput;

    bool isPlaylist = false;
};
#endif // SONGSAILOR_H
