#include <QtDebug>
#include <QFileInfo>
#include <QDir>
#include "ss_process.h"

SS_Process::SS_Process(QObject *parent) :
    QObject(parent)
{
    proc.setReadChannelMode(QProcess::MergedChannels);
    connect(&proc, &QProcess::readyRead, this, &SS_Process::autoemitreadReadyProcess);
    connect(&proc, &QProcess::started, this, &SS_Process::started);
    connect(&proc, static_cast<void (QProcess::*)(int)>(&QProcess::finished), this, &SS_Process::exited);
}

SS_Process::~SS_Process()
{}

bool SS_Process::isRunning()
{
    return running;
}

void SS_Process::start(SSProfile * const p, bool debug)
{
    app_path = p->backend;
    backendTypeID = p->getBackendTypeID();
    start(p->server, p->password, p->server_port, p->local_addr, p->local_port, p->method, p->timeout, p->custom_arg, debug, p->fast_open);
}

void SS_Process::start(QString &args)
{
    stop();
#ifdef Q_OS_WIN
    QString sslocalbin = QFileInfo(app_path).dir().canonicalPath();
    sslocalbin.append("/node_modules/shadowsocks/bin/sslocal");
    sslocalbin = QString("\"") + QDir::toNativeSeparators(sslocalbin) + QString("\"");
    switch (backendTypeID) {
    case 0://libev
        args.append(" -u");
    case 2://go
        proc.setProgram(app_path);
        break;
    case 1://nodejs
        proc.setProgram("node");
        args.prepend(sslocalbin);
        break;
    case 3://python
        proc.setProgram("python");
        args.prepend(QString("\""));
        args.prepend(app_path);
        args.prepend(QString("\""));
        break;
    default:
        qWarning() << tr("Aborted: Invalid Backend Type.") << backendTypeID;
        return;
    }
    proc.setNativeArguments(args);
    proc.start();
#else
    proc.start(app_path + QString(" ") + args);
#endif
    qDebug() << tr("Backend arguments are ") << args;
    proc.waitForStarted(1000);//wait for at most 1 second
}

void SS_Process::start(const QString &server, const QString &pwd, const QString &s_port, const QString &l_addr, const QString &l_port, const QString &method, const QString &timeout, const QString &custom_arg, bool debug, bool tfo)
{
    QString args;
    args.append(QString(" -s ") + server);
    args.append(QString(" -p ") + s_port);
    args.append(QString(" -b ") + l_addr);
    args.append(QString(" -l ") + l_port);
    args.append(QString(" -k \"") + pwd + QString("\""));
    args.append(QString(" -m ") + method.toLower());
    if (backendTypeID != 2) {//go port doesn't support this argument
        args.append(QString(" -t ") + timeout);
    }

    if (debug) {
        if(backendTypeID == 2) {
            args.append("-d=true");//shadowsocks-go
        }
        else {
            args.append(" -v");
        }
    }

#ifdef Q_OS_LINUX
    if ((backendTypeID == 3 || backendTypeID == 0) && tfo) {//only python and libev ports support tfo
        args.append(" --fast-open");
    }
#else
    Q_UNUSED(tfo);
#endif

    args.append(" ").append(custom_arg);
    start(args);
}

void SS_Process::stop()
{
    if (proc.isOpen()) {
        proc.close();
    }
}

void SS_Process::autoemitreadReadyProcess()
{
    emit readReadyProcess(proc.readAll());
}

void SS_Process::started()
{
    running = true;
    qDebug() << tr("Backend started. PID: ") <<proc.pid();
    emit sigstart();
}

void SS_Process::exited(int e)
{
    qDebug() << tr("Backend exited. Exit Code: ") << e;
    running = false;
    emit sigstop();
}
