#include "DatapackChecksum.h"

#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QString>
#include <QSet>
#include <QDebug>

#include "../../general/base/GeneralVariable.h"
#include "../../general/base/FacilityLib.h"

using namespace CatchChallenger;

QThread DatapackChecksum::thread;

DatapackChecksum::DatapackChecksum()
{
    if(!thread.isRunning())
        thread.start();
    moveToThread(&thread);
}

DatapackChecksum::~DatapackChecksum()
{
}

QByteArray DatapackChecksum::doChecksum(const QString &datapackPath, bool onlyFull)
{
    QCryptographicHash hash(QCryptographicHash::Sha224);

    const QSet<QString> &extensionAllowed=QString(CATCHCHALLENGER_EXTENSION_ALLOWED).split(";").toSet();
    QRegularExpression datapack_rightFileName=QRegularExpression(DATAPACK_FILE_REGEX);
    QStringList returnList=CatchChallenger::FacilityLib::listFolder(datapackPath);
    returnList.sort();
    int index=0;
    const int &size=returnList.size();
    while(index<size)
    {
        const QString &fileName=returnList.at(index);
        if(fileName.contains(datapack_rightFileName))
        {
            if(!QFileInfo(fileName).suffix().isEmpty() && extensionAllowed.contains(QFileInfo(fileName).suffix()))
            {
                QFile file(datapackPath+returnList.at(index));
                if(file.size()<=8*1024*1024)
                {
                    if(file.open(QIODevice::ReadOnly))
                    {
                        const QByteArray &data=file.readAll();
                        if(!onlyFull)
                        {
                            QCryptographicHash hashFile(QCryptographicHash::Sha224);
                            hashFile.addData(data);
                        }
                        hash.addData(data);
                        file.close();
                    }
                    else
                    {
                        qDebug() << QStringLiteral("Unable to open the file to do the checksum: %1").arg(file.fileName());
                        return QByteArray();
                    }
                }
            }
        }
        index++;
    }
    return hash.result();
}

void DatapackChecksum::doDifferedChecksum(const QString &datapackPath)
{
    QList<quint32> partialHashList;
    QStringList datapackFilesList;

    //do the by file partial hash
    {
        const QSet<QString> &extensionAllowed=QString(CATCHCHALLENGER_EXTENSION_ALLOWED).split(";").toSet();
        QRegularExpression datapack_rightFileName=QRegularExpression(DATAPACK_FILE_REGEX);
        QStringList returnList=CatchChallenger::FacilityLib::listFolder(datapackPath);
        returnList.sort();
        int index=0;
        const int &size=returnList.size();
        while(index<size)
        {
            const QString &fileName=returnList.at(index);
            if(fileName.contains(datapack_rightFileName))
            {
                if(!QFileInfo(fileName).suffix().isEmpty() && extensionAllowed.contains(QFileInfo(fileName).suffix()))
                {
                    QFile file(datapackPath+returnList.at(index));
                    if(file.size()<=8*1024*1024)
                    {
                        if(file.open(QIODevice::ReadOnly))
                        {
                            datapackFilesList << returnList.at(index);

                            QCryptographicHash hash(QCryptographicHash::Sha224);
                            const QByteArray &data=file.readAll();
                            {
                                QCryptographicHash hashFile(QCryptographicHash::Sha224);
                                hashFile.addData(data);
                            }
                            hash.addData(data);
                            partialHashList << *reinterpret_cast<const int *>(hash.result().constData());
                            file.close();
                        }
                        else
                        {
                            partialHashList << 0;
                            qDebug() << QStringLiteral("Unable to open the file to do the checksum: %1").arg(file.fileName());
                        }
                    }
                }
            }
            index++;
        }
    }

    emit datapackChecksumDone(datapackFilesList,doChecksum(datapackPath),partialHashList);
}
