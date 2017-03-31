#include "mainwindow.h"
#include <QApplication>
#include <QSqlDatabase>
#include <QtSql>
#include <QFileDialog>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));

     QSqlDatabase maBase = QSqlDatabase::addDatabase("QMYSQL");
     maBase.setDatabaseName("alpes_outillage_reparation");
     maBase.setHostName("localhost");
     maBase.setUserName("adminAOR");
     maBase.setPassword("123456789");
     bool baseOuverte = maBase.open();
     if(baseOuverte)
     {

         MainWindow w;
         w.showMaximized();
         return a.exec();
     }
     else//pb sur la base
     {
         qDebug()<<"La base n'a pas pû être ouverte";
         qDebug()<<"Verifiez que le serveur est lançé et que Mysql est fonctionnel";
         exit(2);
     }
}
