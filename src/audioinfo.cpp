#include "audioinfo.h"

AudioInfo::AudioInfo(QString path)
    : file(path.toLocal8Bit().constData()) // pega acentos
{}

QString AudioInfo::getAuthor() const{
    if (file.tag()->artist().isEmpty()){
        qDebug() << "Autor não encontrado";
        return "Artista Desconhecido";
    }
    return QString::fromStdString(file.tag()->artist().toCString(true));
}

QString AudioInfo::getTitle() const {
    if (file.tag()->title().isEmpty()){
        qDebug() << "Título não encontrado";
        return "Música Desconhecida";
    }
    return QString::fromStdString(file.tag()->title().toCString(true));
}

QByteArray AudioInfo::saveImage(){
    if (file.ID3v2Tag()->frameListMap().contains("APIC")){
        TagLib::ID3v2::AttachedPictureFrame *frame = static_cast<TagLib::ID3v2::AttachedPictureFrame *>(file.ID3v2Tag()->frameListMap()["APIC"].front());
        if (frame){
            TagLib::ByteVector image = frame->picture();
            QByteArray byteArray(reinterpret_cast<const char*>(image.data()), image.size());

            return byteArray;
        }
        else {
            qDebug() << "Imagem não encontrada";

            QFile file(R"(:\image\icons\music_not_found.png)"); // default song album cover
            file.open(QIODevice::ReadOnly);
            QByteArray byteArray = file.readAll();
            file.close();

            return byteArray;
        }
    }
    return QByteArray();
}
