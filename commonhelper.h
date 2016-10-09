#ifndef COMMONHELPER_H
#define COMMONHELPER_H

#include <QObject>
#include <QHostAddress>

class CommonHelper : public QObject
{
    Q_OBJECT
public:
    explicit CommonHelper(QObject *parent = 0);
    QHostAddress getLocalHostIP();

signals:

public slots:

};

#endif // COMMONHELPER_H
