#ifndef BLACKGETCOUNT_H
#define BLACKGETCOUNT_H


#include <QObject>
#include"globaldata.h"

class blackGetCount : public QObject
{
    Q_OBJECT
public:
    explicit blackGetCount(QObject *parent = 0);

    bool write_IIC_slot(QString reg ,QString value);
    QString read_IIC_slot(QString reg);

    QString saveFilePath;

    int fileIndex;


    QString getCountStr[4800];
    QString savedString;


signals:

    void sendSavedFrame_signal(int);
public slots:
    void start_blackGetCount_slot(QString filePath,QString InteTime,int saveFrame,QStringList rowAndCol_list);

    void writeTXT(QString text, int index);

};

#endif // BLACKGETCOUNT_H
