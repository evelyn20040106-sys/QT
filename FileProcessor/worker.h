#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QThread>
#include <QDebug>

class worker : public QObject
{
    Q_OBJECT
public:

    explicit worker(QObject *parent = nullptr);

public slots:

    void dowork(int fileCount);

signals:

    void progressUpdated(int percent);

    void statusUpdated(const QString &status);

    void workFinished();

private:

    void processFile(int fileIndex);
};



#endif // WORKER_H
