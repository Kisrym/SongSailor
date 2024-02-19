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
#include <QMenu>

songsailor::songsailor(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::songsailor)
    , tocador(new QMediaPlayer)
    , audioOutput(new QAudioOutput)
{
    ui->setupUi(this);

    connect(ui->lista_musicas_player, &QListView::customContextMenuRequested, this, &songsailor::showContextMenuMusicasPlayer);

    ui->pause->installEventFilter(watcher);
    ui->avancar->installEventFilter(watcher);       // hover in the player buttons
    ui->retroceder->installEventFilter(watcher);
    ui->random->installEventFilter(watcher);
    ui->loop->installEventFilter(watcher);
    ui->playlist->installEventFilter(watcher);
    ui->sound->installEventFilter(watcher);

    ui->playlist_opt->hide(); // hide the playlist options (offset, amount)

    // applying the models
    ui->lista_musicas_player->setModel(model);
    ui->lista_musicas_playlist->setModel(playlistModel);

    tocador->setAudioOutput(audioOutput);
    this->add_music_in_list();
    this->currentListViewChanged(); // colocando a listView atual ao iniciar o programada
    this->loadPlaylists(); // carregando as playlists da database

    connect(ui->random, &QCheckBox::clicked, this, [=](bool checked){if (checked){ui->random->setIcon(QIcon(":/image/icons/random_hover.png"));}});
    connect(janela, &config::config_signal, this, &songsailor::config_slot);
    connect(ui->paginas, &QStackedWidget::currentChanged, this, &songsailor::currentListViewChanged);

    // Install
    connect(ui->musica, &QLineEdit::textChanged, this, &songsailor::musica_text_changed);
    connect(ui->instalar, &QPushButton::clicked, this, &songsailor::install);

    // Menu
    connect(ui->musicasChange, &QPushButton::clicked, this, [=]{ui->paginas->setCurrentWidget(ui->musicas_pag);ui->terminal->clear();}); // clears the terminal too
    connect(ui->installChange, &QPushButton::clicked, this, [=]{ui->paginas->setCurrentWidget(ui->instalar_pag);ui->terminal->setText("Aguardando instalação...");});

    //Player
    connect(ui->add_directory, &QPushButton::clicked, this, &songsailor::add_directory);
    connect(ui->lista_musicas_player, &QListView::doubleClicked, this, [=]{
        reproduction_list.clear();
        this->play();
        this->createPlaylist();
    });
    connect(ui->lista_musicas_playlist, &QListView::doubleClicked, this, [=]{
        reproduction_list.clear();
        this->play();
        this->createPlaylist();
    });
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

    //Playlist
    connect(ui->playlistNome, &QLineEdit::editingFinished, this, &songsailor::nameDescriptionChanged);
    connect(ui->playlistDescription, &QLineEdit::editingFinished, this, &songsailor::nameDescriptionChanged);
}

songsailor::~songsailor()
{
    delete ui;
    delete model;
    delete playlistModel;
    delete watcher;
    delete tocador;
    delete audioOutput;
    delete janela;
}

void songsailor::instalarMusicas(QString audio, QString type){
    QStringList arguments;
    QString path = QFileDialog::getExistingDirectory(this, "Diretório para Instalação", QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    QDir::setCurrent("/home/kaio/Desktop/Storage/projetos_qt/SongSailor/");
    arguments << "src/music_downloader.py" << "install" << "-type" << type << "-audio" << audio << "--path" << path;

    if (this->isPlaylist){
        arguments << "--offset" << ui->offset->text() << "--amount" << ui->amount->text();
    }

    QFuture<void> future = QtConcurrent::run([this, arguments](){ // resumidamente, isso sincroniza as threads com o resultado no final, deixando "assíncrono"
        QProcess process;

        process.setProgram("python3");
        process.setArguments(arguments);
        /*
        process.setReadChannel(QProcess::StandardError);
        QObject::connect(&process, &QProcess::readyReadStandardError, [&]() {
            qDebug() << process.readAllStandardError();
        });*/

        QObject::connect(&process, &QProcess::readyReadStandardOutput, this, [&]() {
            QTextStream in(&process);
            while (!in.atEnd()) {
                QString line = in.readLine();
                ui->terminal->setText(ui->terminal->text() + "\n" + line);
            }
        });

        process.start();
        process.waitForFinished(-1); // -1 deixa a espera infinita

        if (process.exitCode() == 0){
            ui->terminal->setText(ui->terminal->text() + "\nInstalação Finalizada");
            QMetaObject::invokeMethod(this, "add_music_in_list", Qt::QueuedConnection); // equivalente a "this->add_music_in_list", só que no future
            process.deleteLater();
        }
        else if (process.exitCode() == 2){
            QMetaObject::invokeMethod(this, "change_config", Qt::QueuedConnection);
            process.deleteLater();
        }
        else {
            qDebug() << "Erro na instalação da(s) música(s)";
            qDebug() << process.exitCode();
            process.deleteLater();
        }
    });
}

void songsailor::change_config(){
    janela->show();
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
    if (!musicasPlayer.isEmpty()){
        musicasPlayer.clear();
        model->clear();
    }
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
            stream << Qt::endl << path << Qt::endl;
        }

    }
    this->add_music_in_list();
    file.close();
}

void songsailor::play(){
    nomeMusicaAtual = currentListView->currentIndex().data(Qt::DisplayRole).toString().split("\n")[0];
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
    currentModel = (currentListView->objectName() == "lista_musicas_player") ? model : playlistModel;

    if (!ui->random->isChecked()){
        QModelIndex currentIndex = currentListView->currentIndex(); //!

        for (int c = currentIndex.row(); c < currentModel->rowCount(); c++){
            QModelIndex index = currentModel->index(c, currentIndex.column());
            AudioInfo musica(musicasPlayer.value(currentModel->data(index, Qt::DisplayRole).toString().split("\n")[0]));

            reproduction_list.append(musica.getTitle());
        }
        playlist = reproduction_list;
        reproduction_list.removeFirst(); // removing the current music
    }
    else {
        for (int c = 0; c < currentModel->rowCount(); c++){
            QModelIndex index = currentModel->index(c, 0);

            AudioInfo musica(musicasPlayer.value(currentModel->data(index, Qt::DisplayRole).toString().split("\n")[0]));

            if (currentListView->currentIndex().data(Qt::DisplayRole).toString().split("\n")[0] != musica.getTitle()){ // removing the current music from the playlist //!
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
            for (int c = 0; c < currentModel->rowCount(); c++){
                QModelIndex index = currentModel->index(c, 0);

                if (reproduction_list.first() == currentModel->data(index, Qt::DisplayRole).toString().split("\n")[0]){
                    currentListView->setCurrentIndex(index); //!
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

void songsailor::config_slot(QString client_id, QString secret_id, QString refresh_token){;
    QStringList arguments;
    QProcess process;
    arguments << "src/music_downloader.py" << "config" << "-client_id" << client_id << "-secret_id" << secret_id << "-refresh_token" << refresh_token;

    process.setProgram("python");
    process.setArguments(arguments);

    process.setReadChannel(QProcess::StandardError);
    QObject::connect(&process, &QProcess::readyReadStandardError, [&]() {
        qDebug() << process.readAllStandardError();
    });

    process.start();
    process.waitForFinished(-1);

    janela->close();
}

void songsailor::showContextMenuMusicasPlayer(const QPoint &pos){
    QListView *listView = qobject_cast<QListView *>(sender());
    if (!listView){
        return;
    }

    QModelIndex index = listView->indexAt(pos);
    if (index.isValid()){
        QMenu menu;
        QMenu submenu("Adicionar à playlist");
        menu.addMenu(&submenu);

        QAction *createPlaylist = submenu.addAction("Criar playlist");
        submenu.addSeparator();

        for (Database::Playlist &playlist : db.loadPlaylistDb()){
            QAction *playlistAction = submenu.addAction(playlist.nome); // cria cada opção do context menu

            connect(playlistAction, &QAction::triggered, this, [=]{
                db.addMusicPlaylistDb(playlist.nome, ui->lista_musicas_player->currentIndex().data(Qt::DisplayRole).toString().split("\n")[0]); // o botão vai adicionar a musica à playlist
            });
        }

        QAction *deleteAct = menu.addAction("Excluir faixa");
        QAction *selectedAction = menu.exec(listView->viewport()->mapToGlobal(pos));
        if (selectedAction == createPlaylist){
            this->createPlaylistAct();
        }
        if (selectedAction == deleteAct){
            db.deleteMusicPlaylistsDb(ui->lista_musicas_player->currentIndex().data(Qt::DisplayRole).toString().split("\n")[0]);
            qDebug() << QFile::remove(musicasPlayer.value(index.data(Qt::DisplayRole).toString().split("\n")[0]));
            this->add_music_in_list();
        }
    }
}

void songsailor::showContextMenuPlaylists(const QPoint &pos){
    QPushButton *botao = qobject_cast<QPushButton *>(sender());
    QMenu menu;
    QAction *excluirOption = new QAction("Excluir playlist", this);

    menu.addAction(excluirOption);
    QAction *selectedAction = menu.exec(botao->mapToGlobal(pos));

    if (selectedAction == excluirOption){
        db.deletePlaylistDb(botao->text());
        delete botao;
    }
}

void songsailor::createPlaylistAct(){// nome da musica vinda da database
    QString musicaNome = ui->lista_musicas_player->currentIndex().data(Qt::DisplayRole).toString().split("\n")[0]; // se o nome não estiver vazio, é para carregar os dados em vez de criar

    QPushButton *botao = new QPushButton(musicaNome);
    botao->setContextMenuPolicy(Qt::CustomContextMenu);

    AudioInfo musica(musicasPlayer.value(musicaNome));
    QPixmap albumCover;

    albumCover.loadFromData(musica.saveImage());
    botao->setMinimumSize(QSize(0, 50));
    botao->setIcon(QIcon(albumCover));
    botao->setIconSize(QSize(45, 45));

    ui->biblioteca->layout()->addWidget(botao);
    connect(botao, &QPushButton::clicked, this, &songsailor::playlistPag);
    connect(botao, &QPushButton::customContextMenuRequested, this, &songsailor::showContextMenuPlaylists);

    db.createPlaylistDb(musica.getTitle(), musica.saveImage());
    db.addMusicPlaylistDb(musica.getTitle(), musicaNome); // inicialmente é o mesmo nome
}

void songsailor::loadPlaylists(){
    for (Database::Playlist &playlist : db.loadPlaylistDb()){
        QPushButton *botao = new QPushButton(playlist.nome);
        botao->setContextMenuPolicy(Qt::CustomContextMenu);

        QPixmap albumCover;

        albumCover.loadFromData(playlist.imagem);
        botao->setMinimumSize(QSize(0, 50));
        botao->setIcon(QIcon(albumCover));
        botao->setIconSize(QSize(45, 45));

        ui->biblioteca->layout()->addWidget(botao);
        connect(botao, &QPushButton::clicked, this, &songsailor::playlistPag);
        connect(botao, &QPushButton::customContextMenuRequested, this, &songsailor::showContextMenuPlaylists);
    }
}

void songsailor::playlistPag(){
    playlistModel->clear();

    QPushButton *botao = qobject_cast<QPushButton *>(sender()); // botão que emitiu o sinal
    QPixmap albumCover = botao->icon().pixmap(QSize(150, 150));
    Database::Playlist playlist = db.locatePlaylistDb(botao->text());

    ui->playlistNome->setText(playlist.nome);
    ui->albumCover->setPixmap(albumCover);
    ui->playlistDescription->setText(playlist.descricao);

    for (QString &item : playlist.musicas){
        if (!item.isEmpty()){
            AudioInfo musica(musicasPlayer.value(item));
            playlistModel->addItem(MusicItem(musica.getTitle(), musica.getAuthor(), musica.saveImage()));
            ui->lista_musicas_playlist->update();
        }
    }

    ui->paginas->setCurrentWidget(ui->playlist_pag);

    disconnect(ui->playlistNome, &QLineEdit::textEdited, nullptr, nullptr);
    connect(ui->playlistNome, &QLineEdit::textEdited, this, [=](QString novoNome){botao->setText(novoNome);});
}

void songsailor::currentListViewChanged(){
    QWidget *currentWidget = ui->paginas->currentWidget();
    if (currentWidget){
        foreach(QObject *child, currentWidget->children()){
            QListView *listView = qobject_cast<QListView*>(child);
            if (listView){
                currentListView = listView;
            }
        }
    }
}

void songsailor::nameDescriptionChanged(){
    QStringList playlist;
    currentModel = (currentListView->objectName() == "lista_musicas_player") ? model : playlistModel;

    for (int row = 0; row < currentModel->rowCount(); ++row) {
        for (int column = 0; column < currentModel->columnCount(); ++column) {
            QModelIndex index = currentModel->index(row, column);
            AudioInfo musica(musicasPlayer.value(currentModel->data(index, Qt::DisplayRole).toString().split("\n")[0]));

            playlist.append(musica.getTitle());
        }
    }

    db.alterPlaylistDb(playlist, ui->playlistNome->text(), ui->playlistDescription->text());
}
