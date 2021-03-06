/*
 * Shadowsocks Process Class
 *
 * Used to interact with the backend.
 *
 * Copyright 2014 William Wong <librehat@outlook.com>
 */
#ifndef SS_PROCESS_H
#define SS_PROCESS_H
#include <QObject>
#include <QString>
#include <QProcess>
#include "ssprofile.h"

class SS_Process : public QObject
{
    Q_OBJECT

public:
    SS_Process(QObject *parent = 0);
    ~SS_Process();
    void start(SSProfile * const, bool debug = false);
    void stop();
    bool isRunning();

signals:
    void readReadyProcess(const QByteArray &o);
    void sigstart();
    void sigstop();

private:
    bool running;
    int backendTypeID;
    QString app_path;
    QProcess proc;

    void start(const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, bool debug = false, bool tfo = false);
    void start(QString &args);

private slots:
    void autoemitreadReadyProcess();
    void started();
    void exited(int);
};

#endif // SS_PROCESS_H
