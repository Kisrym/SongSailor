#include "songsailor.h"
#include "./ui_songsailor.h"
#include "src/audioinfo.h"
#include "QProcess"

#include <QDir>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QFileDialog>
#include <ctime>
#include <algorithm>

songsailor::songsailor(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::songsailor)
    , tocador(new QMediaPlayer)
    , audioOutput(new QAudioOutput)
{
    ui->setupUi(this);

    ui->pause->installEventFilter(watcher);
    ui->avancar->installEventFilter(watcher);       // hover in the player buttons
    ui->retroceder->installEventFilter(watcher);
    ui->random->installEventFilter(watcher);
    ui->loop->installEventFilter(watcher);
    ui->playlist->installEventFilter(watcher);
    ui->sound->installEventFilter(watcher);

    ui->playlist_opt->hide(); // hide the playlist options (offset, amount)

    // adicionando os modelos
    ui->lista_musicas_player->setModel(model);

    tocador->setAudioOutput(audioOutput);
    this->add_music_in_list();

    connect(ui->random, &QCheckBox::clicked, this, [=](bool checked){if (checked){ui->random->setIcon(QIcon(":/image/icons/random_hover.png"));}});

    // connections
    connect(ui->musica, &QLineEdit::textChanged, this, &songsailor::musica_text_changed);
    connect(ui->instalar, &QPushButton::clicked, this, &songsailor::install);

    connect(ui->musicasChange, &QPushButton::clicked, this, [=]{ui->paginas->setCurrentWidget(ui->musicas_pag);ui->terminal->clear();}); // clears the terminal too
    connect(ui->installChange, &QPushButton::clicked, this, [=]{ui->paginas->setCurrentWidget(ui->instalar_pag);});

    connect(ui->add_directory, &QPushButton::clicked, this, &songsailor::add_directory);

    //Player
    connect(ui->lista_musicas_player, &QListView::doubleClicked, this, [=]{reproduction_list.clear();});
    connect(ui->lista_musicas_player, &QListView::doubleClicked, this, &songsailor::play);
    connect(ui->lista_musicas_player, &QListView::doubleClicked, this, &songsailor::createPlaylist);

    connect(tocador, &QMediaPlayer::durationChanged, this, [=](qint64 v){ui->slider->setMaximum(v / 1000);});
    connect(tocador, &QMediaPlayer::positionChanged, this, [=](qint64 v){ui->slider->setValue(v / 1000);});

    connect(ui->slider, &QSlider::sliderPressed, this, [=]{tocador->pause();});
    connect(ui->slider, &QSlider::sliderReleased, this, [=]{tocador->setPosition(ui->slider->value() * 1000);tocador->play();}); // change the music position

    connect(ui->pause, &QPushButton::clicked, this, [=]{(tocador->isPlaying() ? tocador->pause() : tocador->play());});
    connect(tocador, &QMediaPlayer::playbackStateChanged, this, &songsailor::changeStateIcon);
    connect(tocador, &QMediaPlayer::playbackStateChanged, this, &songsailor::musicEnded);

    connect(ui->volume, &QSlider::valueChanged, this, [=](double v){audioOutput->setVolume(v / 1000);});
    connect(ui->sound, &QCheckBox::clicked, this, [=](bool checked){audioOutput->setMuted(checked); ui->volume->setEnabled(!checked);});

    connect(ui->loop, &QCheckBox::clicked, this, &songsailor::loop);
    connect(ui->random, &QCheckBox::clicked, this, &songsailor::createPlaylist);
    connect(ui->avancar, &QPushButton::clicked, this, [=]{tocador->stop();});
    connect(ui->retroceder, &QPushButton::clicked, this, &songsailor::retroceder);

}

songsailor::~songsailor()
{
    delete ui;
    delete model;
    delete watcher;
    delete tocador;
    delete audioOutput;
}

void songsailor::instalarMusicas(QString audio, QString type){
    QStringList arguments;
    QString path = QFileDialog::getExistingDirectory(this, "Diretório para Instalação", QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    arguments << "music_downloader.py" << "-type" << type << "-audio" << audio << "--path" << path;

    if (this->isPlaylist){
        arguments << "--offset" << ui->offset->text() << "--amount" << ui->amount->text();
    }

    QFuture<void> future = QtConcurrent::run([this, arguments](){ // resumidamente, isso sincroniza as threads com o resultado no final, deixando "assíncrono"
        QProcess process;

        process.setProgram("python");
        process.setArguments(arguments);

        /*
        process.setReadChannel(QProcess::StandardError);
        QObject::connect(&process, &QProcess::readyReadStandardError, [&]() {
            qDebug() << process.readAllStandardError();
        });
        */
        QObject::connect(&process, &QProcess::readyReadStandardOutput, this, [&]() {
            QTextStream in(&process);
            while (!in.atEnd()) {
                QString line = in.readLine();
                ui->terminal->setText(ui->terminal->text() + "\n" + line);
            }
        });

        process.start();
        process.waitForFinished(-1);

        if (process.exitCode() == 0){ // -1 deixa a espera infinita
            ui->terminal->setText(ui->terminal->text() + "\nInstalação Finalizada");
            QMetaObject::invokeMethod(this, "add_music_in_list", Qt::QueuedConnection); // equivalente a "this->add_music_in_list", só que no future
            process.deleteLater();
        }
        else {
            qDebug() << "Erro na instalação da(s) música(s)";
            process.deleteLater();
        }
    });
}

void songsailor::musica_text_changed(const QString musica){
    if (musica.contains("playlist")){
        ui->playlist_opt->show();
        this->isPlaylist = true;
    }
    else {
        ui->playlist_opt->hide();
        this->isPlaylist = false;
    }
}

void songsailor::install(){
    if (!ui->musica->text().isEmpty()){
        QString type = (ui->spotify_radio->isChecked()) ? "spotify" : "youtube";
        QString audio = ui->musica->text();

        this->instalarMusicas(audio, type);
    }
    else {
        return;
    }
}

void songsailor::add_music_in_list(){
    QFile file("paths.txt");
    if (file.open(QIODevice::ReadOnly)){
        QTextStream stream(&file);
        while (!stream.atEnd()){
            QDir directory(stream.readLine());
            QStringList musicas = directory.entryList(QStringList() << "*.mp3", QDir::Files);

            for (QString &m : musicas){
                AudioInfo musica(directory.absoluteFilePath(m)); // getting the info from each music in the directory
                musicasPlayer[musica.getTitle()] = directory.absoluteFilePath(m);

                model->addItem(MusicItem(musica.getTitle(), musica.getAuthor(), musica.saveImage()));
                ui->lista_musicas_player->update();
            }
        }
    }
}

void songsailor::add_directory(){
    QString path = QFileDialog::getExistingDirectory(this, "Abrir Diretório", "/Documents", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    QFile file("paths.txt");
    if (file.open(QIODevice::ReadWrite)){
        QTextStream stream(&file);
        if (!stream.readAll().contains(path)){
            stream << path << Qt::endl;
        }

    }
    this->add_music_in_list();
    file.close();
}

void songsailor::play(){
    nomeMusicaAtual = ui->lista_musicas_player->currentIndex().data(Qt::DisplayRole).toString().split("\n")[0];
    AudioInfo musica(musicasPlayer.value(nomeMusicaAtual));

    QPixmap albumCover;
    albumCover.loadFromData(musica.saveImage());
    ui->albumLabel->setPixmap(albumCover.scaled(QSize(68, 61), Qt::KeepAspectRatio));
    ui->artistaLabel->setText("<b>" + musica.getTitle() + "</b>" + "<br>" + musica.getAuthor());

    tocador->setSource(QUrl::fromLocalFile(musicasPlayer.value(nomeMusicaAtual)));
    tocador->play();
}

void songsailor::changeStateIcon(QMediaPlayer::PlaybackState estado){
    switch (estado) {
    case QMediaPlayer::PlayingState:
        ui->pause->setIcon(QIcon(":/image/icons/pause.png"));
        ui->pause->setObjectName("pause");
        break;
    case QMediaPlayer::PausedState:
        ui->pause->setIcon(QIcon(":/image/icons/play.png"));
        ui->pause->setObjectName("play");
        break;
    case QMediaPlayer::StoppedState:
        ui->pause->setIcon(QIcon(":/image/icons/play.png"));
    }
}

void songsailor::createPlaylist(){
    reproduction_list.clear();
    playlist.clear();
    if (!ui->random->isChecked()){
        QModelIndex currentIndex = ui->lista_musicas_player->currentIndex();
        for (int c = currentIndex.row(); c < model->rowCount(); c++){
            QModelIndex index = model->index(c, currentIndex.column());

            AudioInfo musica(musicasPlayer.value(model->data(index, Qt::DisplayRole).toString().split("\n")[0]));

            reproduction_list.append(musica.getTitle());
        }
        playlist = reproduction_list;
        reproduction_list.removeFirst(); // removing the current music
    }
    else {
        for (int c = 0; c < model->rowCount(); c++){
            QModelIndex index = model->index(c, 0);

            AudioInfo musica(musicasPlayer.value(model->data(index, Qt::DisplayRole).toString().split("\n")[0]));

            if (ui->lista_musicas_player->currentIndex().data(Qt::DisplayRole).toString().split("\n")[0] != musica.getTitle()){ // removing the current music from the playlist
                reproduction_list.append(musica.getTitle());
            }
        }
        std::srand(static_cast<unsigned int>(std::time(nullptr))); // random seed
        std::random_shuffle(reproduction_list.begin(), reproduction_list.end());
        playlist = reproduction_list;
    }
}

void songsailor::musicEnded(QMediaPlayer::PlaybackState estado){
    if (estado == QMediaPlayer::StoppedState && !ui->loop->isChecked()){
        if (!reproduction_list.empty()){
            for (int c = 0; c < model->rowCount(); c++){
                QModelIndex index = model->index(c, 0);

                if (reproduction_list.first() == model->data(index, Qt::DisplayRole).toString().split("\n")[0]){
                    ui->lista_musicas_player->setCurrentIndex(index);
                    this->play();
                    reproduction_list.remove(0);
                    break;
                }
            }
        }
    }
    else if (estado == QMediaPlayer::StoppedState && ui->loop->isChecked()){
        tocador->setPosition(0);
        tocador->play();
    }
}

void songsailor::loop(){
    if (ui->loop->isChecked()){
        ui->loop->setIcon(QIcon(":/image/icons/loop_hover.png"));
        tocador->setLoops(-1);
    }
    else {
        ui->loop->setIcon(QIcon(":/image/icons/loop.png"));
        tocador->setLoops(1);
    }
}

void songsailor::retroceder(){
    if (!ui->loop->isChecked()){
        qsizetype index = playlist.indexOf(nomeMusicaAtual);

        if (playlist.indexOf(nomeMusicaAtual) == 0){
            tocador->setPosition(0); // restart the music if it's the first one
        }
        else {
            reproduction_list.prepend(playlist.at(index)); // fixing the reproduction_list again
            reproduction_list.prepend(playlist.at(index - 1));

            tocador->stop();
        }
    }
    else {
        tocador->setPosition(0);
        tocador->play();
    }
}
