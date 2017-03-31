#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSqlDriver>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QMessageBox>
#include <QSqlRecord>
#include <QSqlRelationalDelegate>
#include <QTextDocument>
#include <QtPrintSupport/QPrintDialog>
#include <QPrinter>
#include "dialogajoutpieceareparation.h"
#include <QCloseEvent>
#include <QWebView>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    chargerLesClients();
    chargerLesMachines();
    chargerLesTechniciens();
    chargerLesEtatsReparation();
    chargerLesEtatsDevis();
    connect(qApp,SIGNAL(focusChanged(QWidget*,QWidget*)),this,SLOT(on_focusChanged(QWidget*,QWidget*)));
    connect(this, SIGNAL(clientSelectionne(QString)),this,SLOT(afficherClientSelectionne(QString)));
    connect(ui->lineEditNom, SIGNAL(textEdited(QString)),this, SLOT(actDesactBoutonAjouterClient()));
    connect(ui->lineEditPrenom, SIGNAL(textEdited(QString)),this, SLOT(actDesactBoutonAjouterClient()));
    connect(ui->lineEditTelephone, SIGNAL(textEdited(QString)),this, SLOT(actDesactBoutonAjouterClient()));
    connect(ui->lineEditAdresse, SIGNAL(textEdited(QString)),this, SLOT(actDesactBoutonAjouterClient()));
    connect(ui->lineEditMarque,SIGNAL(textEdited(QString)),this, SLOT(actDesactBoutonsAjouterMachine()));
    connect(ui->lineEditReference,SIGNAL(textEdited(QString)),this, SLOT(actDesactBoutonsAjouterMachine()));
    connect(ui->lineEditNomMachine,SIGNAL(textEdited(QString)),this, SLOT(actDesactBoutonsAjouterMachine()));
    //j'échange le mécano avec la panne dans la table des réparations
    ui->tableWidgetMachine->horizontalHeader()->swapSections(5,10);
    //et je dis à la panne de prendre l'espace restant
    ui->tableWidgetMachine->horizontalHeader()->stretchLastSection();
    //les completers
    modelCp.setQuery("select distinct(codePostal) from localite" );
    completerCP.setModel(&modelCp);
    ui->lineEditCP->setCompleter(&completerCP);
    //pour les prénoms
    modelPrenoms.setQuery("select distinct(prenomClient) from Client" );
    completerPrenoms.setModel(&modelPrenoms);
    completerPrenoms.setCaseSensitivity(Qt::CaseInsensitive);
    ui->lineEditPrenom->setCompleter(&completerPrenoms);
    //pour les pannes
    QStringList listeDesPannes;
    QSqlQuery queryTtePannes("select panneReparation from Reparation");
    while(queryTtePannes.next())
    {
        QStringList listPanneASplitter=queryTtePannes.value(0).toString().split("&&");
        foreach (QString laPanne, listPanneASplitter) {
            if(!(listeDesPannes.contains(laPanne)))
            {
                listeDesPannes.append(laPanne);
            }
        }
    }
    listeDesPannes.sort(Qt::CaseInsensitive);
    completerPanne=new QCompleter(listeDesPannes);

    completerPanne->setCaseSensitivity(Qt::CaseInsensitive);
    ui->lineEditPanne1->setCompleter(completerPanne);
    ui->lineEditPanne2->setCompleter(completerPanne);
    ui->lineEditPanne3->setCompleter(completerPanne);
    ui->lineEditPanne4->setCompleter(completerPanne);
}
void MainWindow::chargerLesEtatsReparation()
{
    qDebug()<<"void MainWindow::chargerLesEtatsReparation()";
    ui->comboBoxTaf->clear();   
    QSqlQuery reqTousLesEtats("select idEtat,libelleEtat from Etat_Reparation");
    while(reqTousLesEtats.next())
    {
        QString idEtat=reqTousLesEtats.value("idEtat").toString();
        QString libelleEtat=reqTousLesEtats.value("libelleEtat").toString();
        ui->comboBoxTaf->addItem(libelleEtat,idEtat);
    }
}
void MainWindow::chargerLesEtatsDevis()
{
    qDebug()<<"void MainWindow::chargerLesEtatsDevis()";
    ui->comboBoxDevis->clear();    
    QSqlQuery reqTousLesEtatsDevis("select  idDevis,etat from Devis_Reparation");
    while(reqTousLesEtatsDevis.next())
    {
        QString idEtat=reqTousLesEtatsDevis.value("idDevis").toString();
        QString libelleEtat=reqTousLesEtatsDevis.value("etat").toString();
        ui->comboBoxDevis->addItem(libelleEtat,idEtat);
    }
}
void MainWindow::chargerLesTechniciens()
{
    qDebug()<<"void MainWindow::chargerLesTechniciens()";
    ui->comboBoxMecano->clear();
    tableModelTechnicien=new QSqlRelationalTableModel(this);
    tableModelTechnicien->setTable("Utilisateur");
    tableModelTechnicien->setRelation(2,QSqlRelation("typeUtilisateur","idType","libelleType"));
    ui->tableViewTechniciens->setModel(tableModelTechnicien);
    tableModelTechnicien->select();
    ui->tableViewTechniciens->setItemDelegate(new QSqlRelationalDelegate(ui->tableViewTechniciens));
    //remplir la combo des mécanos
    QSqlQuery reqTousLesMecanos("select idUtilisateur,nomUtilisateur from Utilisateur where not supprime and typeUtilisateur!=5");
    while(reqTousLesMecanos.next())
    {
        QString idMecano=reqTousLesMecanos.value("idUtilisateur").toString();
        QString nomUtilisateur=reqTousLesMecanos.value("nomUtilisateur").toString();
        ui->comboBoxMecano->addItem(nomUtilisateur,idMecano);
    }
    //mecano non affecté
    ui->comboBoxMecano->addItem("","NULL");
}

MainWindow::~MainWindow()
{
    delete ui;
    delete completerPanne;
}

void MainWindow::majListeRechercheMachine(QString leTexte)
{
    qDebug()<<"void MainWindow::majListeRechercheMachine(QString leTexte)";
    //s'exécute quand il faut raffraichir la liste de recherche
    //sender() est l'émetteur du signal donc un des trois lineEdit auquel le slot est attaché
    if(sender()==ui->lineEditReference)
    {
        QString txtReq="select idModel,nature,codeModel,libelleMarque from Modele inner join Marque on Modele.marque=Marque.idMarque where upper(codeModel) like upper('%"+ui->lineEditReference->text()+"%')";
        qDebug()<<txtReq;
        QSqlQuery maReq(txtReq);
        //effacement de la liste
        ui->listWidgetResultatRecherche->clear();
        while(maReq.next())
        {
            QListWidgetItem*  nouvelElement=new QListWidgetItem(maAjouterReq.value("codeModel").toString()+" "+maReq.value("libelleMarque").toString()+" "+maReq.value("nature").toString());
            nouvelElement->setData(32,maReq.value("idModel").toString());
            nouvelElement->setData(33,maReq.value("codeModel").toString());
            nouvelElement->setData(34,maReq.value("libelleMarque").toString());
            nouvelElement->setData(35,maReq.value("nature").toString());
            ui->listWidgetResultatRecherche->insertItem(0,nouvelElement);
        }
    }
    else if(sender()==ui->lineEditMarque)
    {
        QString txtReq="select idModel,nature,codeModel,libelleMarque from Modele inner join Marque on Modele.marque=Marque.idMarque where upper(libelleMarque) like upper('%"+ui->lineEditMarque->text()+"%')";
        qDebug()<<txtReq;
        QSqlQuery maReq(txtReq);
        //effacement de la liste
        ui->listWidgetResultatRecherche->clear();
        while(maReq.next())
        {
            QListWidgetItem*  nouvelElement=new QListWidgetItem(maReq.value("codeModel").toString()+" "+maReq.value("libelleMarque").toString()+" "+maReq.value("nature").toString());
            nouvelElement->setData(32,maReq.value("idModel").toString());
            nouvelElement->setData(33,maReq.value("codeModel").toString());
            nouvelElement->setData(34,maReq.value("libelleMarque").toString());
            nouvelElement->setData(35,maReq.value("nature").toString());
            ui->listWidgetResultatRecherche->insertItem(0,nouvelElement);
        }
    }
    else if(sender()==ui->lineEditNomMachine)
    {
        QString txtReq="select idModel,nature,codeModel,libelleMarque from Modele inner join Marque on Modele.marque=Marque.idMarque where upper(nature) like upper('%"+ui->lineEditNomMachine->text()+"%')";
        qDebug()<<txtReq;
        QSqlQuery maReq(txtReq);
        //effacement de la liste
        ui->listWidgetResultatRecherche->clear();
        while(maReq.next())
        {
            QListWidgetItem*  nouvelElement=new QListWidgetItem(maReq.value("codeModel").toString()+" "+maReq.value("libelleMarque").toString()+" "+maReq.value("nature").toString());
            nouvelElement->setData(32,maReq.value("idModel").toString());
            nouvelElement->setData(33,maReq.value("codeModel").toString());
            nouvelElement->setData(34,maReq.value("libelleMarque").toString());
            nouvelElement->setData(35,maReq.value("nature").toString());
            ui->listWidgetResultatRecherche->insertItem(0,nouvelElement);
        }
    }
}

void MainWindow::on_focusChanged(QWidget* old, QWidget* nouveau)
{
    qDebug()<<"void MainWindow::on_focusChanged(QWidget* old, QWidget* nouveau)";
    if(nouveau==ui->lineEditReference)
    {
        disconnect(ui->lineEditMarque,SIGNAL(textEdited(QString)),this, SLOT(majListeRechercheMachine(QString)));
        disconnect(ui->lineEditNomMachine,SIGNAL(textEdited(QString)),this, SLOT(majListeRechercheMachine(QString)));
        //il est dans le modele on affiche les références
        //attacher l'evenement textChanged à la maj de la zone de droite
        connect(ui->lineEditReference,SIGNAL(textEdited(QString)),this, SLOT(majListeRechercheMachine(QString)));
    }
    else if(nouveau==ui->lineEditNomMachine)
    {
        disconnect(ui->lineEditReference,SIGNAL(textEdited(QString)),this, SLOT(majListeRechercheMachine(QString)));
        disconnect(ui->lineEditMarque,SIGNAL(textEdited(QString)),this, SLOT(majListeRechercheMachine(QString)));
        //il est dans le modele on affiche les références
        //attacher l'evenement textChanged à la maj de la zone de droite
        connect(ui->lineEditNomMachine,SIGNAL(textEdited(QString)),this, SLOT(majListeRechercheMachine(QString)));

    }
    else if(nouveau==ui->lineEditMarque)
    {
        disconnect(ui->lineEditReference,SIGNAL(textEdited(QString)),this, SLOT(majListeRechercheMachine(QString)));
        disconnect(ui->lineEditNomMachine,SIGNAL(textEdited(QString)),this, SLOT(majListeRechercheMachine(QString)));
        //il est dans le modele on affiche les références
        //attacher l'evenement textChanged à la maj de la zone de droite
        connect(ui->lineEditMarque,SIGNAL(textEdited(QString)),this, SLOT(majListeRechercheMachine(QString)));
    }
}
void MainWindow::effacerTousLesClients()
{
    qDebug()<<"void MainWindow::effacerTousLesClients()";
    ui->tableWidgetClient->clearContents();
    int nbLigne=ui->tableWidgetClient->rowCount();
    for(int noLigne=0;noLigne<nbLigne;noLigne++) ui->tableWidgetClient->removeRow(0);
}

void MainWindow::chargerLesClients()
{
    qDebug()<<"void MainWindow::chargerLesClients()";
   QSqlQuery reqChargerClient;
   int ligneActu=0;
   //on efface tout
   effacerTousLesClients();
   /*CREATE TABLE `Client`(`idClient` int(11),`nomClient` varchar(45),`prenomClient` varchar(45),`telephoneClient` varchar(45),`emailClient` varchar(45),`adresseClient` varchar(45),'cpClient' varchar(6),'villeClient' varchar(45),primary key(`idClient`));
*/
   reqChargerClient.prepare("select nomClient,prenomClient,telephoneClient,emailClient,adresseClient,cpClient,villeClient,idClient from Client where not supprime order by nomClient");
   if(reqChargerClient.exec())
   {
       ui->tableWidgetClient->setRowCount(reqChargerClient.size());
       ui->tableWidgetClient->horizontalHeader()->show();
       //ui->tableWidgetClient->setRowCount(reqChargerClient.size());
       while(reqChargerClient.next())//pour chaque client
       {

        //pour chaque champ à afficher
         for(int noCol=0;noCol<7;noCol++)
         {
            ui->tableWidgetClient->setItem(ligneActu,noCol,new QTableWidgetItem(reqChargerClient.value(noCol).toString()));
         }
         //chargement de l'identifiant du client en data32 de la première colonne
         ui->tableWidgetClient->item(ligneActu,0)->setData(32,reqChargerClient.value("idClient").toString());
          ligneActu++;//on passe à la ligne suivante

       }//fin du pour chaque client
   }//fin du if
   else
   {
       qDebug()<<"pb req";
   }

}//fin de chargerLesClients

void MainWindow::chargerLesMachines(QString filtre)
{
    qDebug()<<"void MainWindow::chargerLesMachines(QString filtre)";
    QString txtReq="select nature,libelleMarque,codeModel,typeMoteur,concat(nomClient,concat(' ',prenomClient)), panneReparation, libelleEtat, etat,dateArrivee,dateFinalisation, nomUtilisateur, idReparation, Client.idClient "
                   "from Modele inner join Reparation on Reparation.outilRef=Modele.idModel "
                   "inner join Marque on Marque.idMarque=Modele.marque "
                   "inner join Client on Reparation.idClient=Client.idClient "
                   "inner join Etat_Reparation on Etat_Reparation.idEtat=Reparation.idEtat "
                   "left outer join Utilisateur on Reparation.idUtilisateur=Utilisateur.idUtilisateur "
                   "left outer join Devis_Reparation on Reparation.idDevis=Devis_Reparation.idDevis where not libelleEtat like 'Supprimer' ";
    if(!filtre.isEmpty())txtReq+=" and telephoneClient like '%"+filtre+"%' or nature like '%"+filtre+"%' or codeModel like '%"+filtre+"%'";
    qDebug()<<txtReq;
    QSqlQuery reqChargerMachine;
    reqChargerMachine.prepare(txtReq);
    if(reqChargerMachine.exec())
    {
        int ligneActu=0;
        ui->tableWidgetMachine->setRowCount(reqChargerMachine.size());
        if(reqChargerMachine.size()>0)
        {
            while(reqChargerMachine.next())
            {
                QString laPanne=reqChargerMachine.value(5).toString();
                QString laPanneEnForme=laPanne.replace("&&","\n");
                for(int noCol=0;noCol<11;noCol++)
                {
                    ui->tableWidgetMachine->setItem(ligneActu,noCol,new QTableWidgetItem(reqChargerMachine.value(noCol).toString()));
                }
                /*je range l'identifiant de la réparation dans l'item colonne0*/
                QString idRep=reqChargerMachine.value("idReparation").toString();
                qDebug()<<"reparation:"<<idRep;
                ui->tableWidgetMachine->item(ligneActu,0)->setData(32,idRep);
                //je range l'identifiant du client dans l'item colonne4 client
                QString idClient=reqChargerMachine.value("idClient").toString();
                qDebug()<<"client:"<<idClient;
                ui->tableWidgetMachine->item(ligneActu,4)->setData(32,idClient);
                //la panne entière en data 32 de l'item colonne 5
                ui->tableWidgetMachine->item(ligneActu,5)->setData(32,laPanne);
                //affichage du type de moteur
                int idType=reqChargerMachine.value(3).toInt();
                if(idType==1)
                {
                    typeMachine="Thermique";
                }
                else
                {
                    typeMachine="Electrique";
                }
                ui->tableWidgetMachine->setItem(ligneActu,3,new QTableWidgetItem(typeMachine));
                //affichage correct des pannes
                ui->tableWidgetMachine->setItem(ligneActu,5,new QTableWidgetItem(laPanneEnForme));
                ui->tableWidgetMachine->verticalHeader()->resizeSections(QHeaderView::ResizeToContents);
                ligneActu++;
            }
        }//fin de requête ok
        else//la requête renvoie 0 ligne
        {
            //il n'y a pas de machine
            qDebug()<<"pas de machine pour l'instant";
        }
    }
    else//erreur lors du select des machines
    {
        qDebug()<<reqChargerMachine.lastError().text();
    }

    ui->pushButtonSupprMachine->setEnabled(false);
}
int MainWindow::chercherIdClient()
{
    qDebug()<<"int MainWindow::chercherIdClient()";
    QSqlQuery reqIdClient;
    QString tReq="select idClient from Client where nomClient='"+nomClient+"' and telephoneClient='"+telClient+"'";
    qDebug()<<tReq;
    reqIdClient.prepare(tReq);
    reqIdClient.exec();
    if(reqIdClient.first())
    {
        int numClient=reqIdClient.value(0).toInt();
        return numClient;
    }
    else
    {
        qDebug()<<"Client non trouvé";
        return -1;
    }
}

void MainWindow::viderLesChampsClient()
{
    qDebug()<<"void MainWindow::viderLesChampsClient()";
    nomClient="";
    prenomClient="";
    telClient="";
    emailCLient="";
    adresseClient="";
    cpClient="";
    villeClient="";

    ui->labelNomClient->setText(nomClient);
    ui->labelPrenomClient->setText(prenomClient);
    ui->labelTelClient->setText(telClient);
    ui->labelEmailClient->setText(emailCLient);
    ui->labelCPClient->setText(cpClient);
    ui->labelAdresseClient->setText(adresseClient);
    ui->labelVilleClient->setText(villeClient);

    ui->lineEditNom->setText(nomClient);
    ui->lineEditPrenom->setText(prenomClient);
    ui->lineEditTelephone->setText(telClient);
    ui->lineEditEmail->setText(emailCLient);
    ui->lineEditAdresse->setText(adresseClient);
    ui->lineEditCP->setText(cpClient);
    ui->lineEditVille->setText(villeClient);

    ui->tableWidgetMachineClient->setRowCount(0);

    ui->pushButtonSupprimerClient->setDisabled(true);
    ui->pushButtonAjouterMachine->setDisabled(true);
    ui->pushButtonModifierClient->setDisabled(true);
    ui->pushButtonAjouterClient->setDisabled(true);

}
void MainWindow::viderChampsMachine()
{
    qDebug()<<"void MainWindow::viderChampsMachine()";
    ui->lineEditNomMachine->setText("");
    ui->lineEditMarque->setText("");
    ui->lineEditReference->setText("");
    ui->lineEditPanne1->setText("");
    ui->lineEditPanne2->setText("");
    ui->lineEditPanne3->setText("");
    ui->lineEditPanne4->setText("");
    ui->comboBoxMecano->setCurrentText("");
    ui->comboBoxDevis->setCurrentText("a faire");
}

void MainWindow::on_pushButtonAjouterClient_clicked()
{
    qDebug()<<"void MainWindow::on_pushButtonAjouterClient_clicked()";
   nomClient=ui->lineEditNom->text().replace("'","\\'");
   prenomClient=ui->lineEditPrenom->text();
   telClient=ui->lineEditTelephone->text();
   emailCLient=ui->lineEditEmail->text();
   adresseClient=ui->lineEditAdresse->text().replace("'","\\'");
   cpClient=ui->lineEditCP->text();
   villeClient=ui->lineEditVille->text().replace("'","\\'");

   QSqlQuery reqMaxId;
   reqMaxId.prepare("select max(idClient) from Client");
   reqMaxId.exec();
   reqMaxId.first();


   int maxId=reqMaxId.value(0).toInt();
   maxId++;
   QString nbClient=QString::number(maxId);
   QSqlQuery reqAjouterClient;

   QString reqAjoutClient="insert into Client(idClient,nomClient,prenomClient,telephoneClient,emailClient,villeClient,adresseClient,cpClient) values("+nbClient+",'"+nomClient+"','"+prenomClient+"','"+telClient+"','"+emailCLient+"','"+villeClient+"','"+adresseClient+"','"+cpClient+"')";
   qDebug()<<reqAjoutClient;
   reqAjouterClient.prepare(reqAjoutClient);
   reqAjouterClient.exec();

   viderLesChampsClient();
   chargerLesClients();
}

void MainWindow::on_pushButtonSupprimerClient_clicked()
{
    qDebug()<<"void MainWindow::on_pushButtonSupprimerClient_clicked()";
   if(QMessageBox::warning(this,this->windowTitle(),"Etes-vous bien certain de vouloir supprimer ce client?",QMessageBox::Yes|QMessageBox::No,QMessageBox::No)==QMessageBox::Yes)
   {
       int numClient=0;
       numClient=chercherIdClient();
       QString numClientSt=QString::number(numClient);

       QString reqSuppr="update Client set supprime=true where idClient="+numClientSt+"";
       qDebug()<<reqSuppr;

       QSqlQuery reqSupprClient;
       reqSupprClient.prepare(reqSuppr);
       reqSupprClient.exec();

       viderLesChampsClient();
       chargerLesClients();
   }
}
/**
 * @brief MainWindow::on_tableWidgetClient_cellClicked
 * @param row
 * @param column
 * Selection du client sur lequel on souhaite travailler
 */
void MainWindow::on_tableWidgetClient_cellClicked(int row, int column)
{
    qDebug()<<"void MainWindow::on_tableWidgetClient_cellClicked(int row, int column";
    QString idClientS=ui->tableWidgetClient->item(row,0)->data(32).toString();
    emit clientSelectionne(idClientS);
}



void MainWindow::on_pushButtonModifierClient_clicked()
{
    qDebug()<<"void MainWindow::on_pushButtonModifierClient_clicked()";
    int numClient=chercherIdClient();
    QString nbClient=QString::number(numClient);

    nomClient=ui->lineEditNom->text().replace("'","\\'");
    prenomClient=ui->lineEditPrenom->text();
    telClient=ui->lineEditTelephone->text();
    emailCLient=ui->lineEditEmail->text();
    adresseClient=ui->lineEditAdresse->text().replace("'","\\'");
    cpClient=ui->lineEditCP->text();
    villeClient=ui->lineEditVille->text().replace("'","\\'");


    QSqlQuery reqAjouterClient;
    QString reqModif="update Client set nomClient='"+nomClient+"', prenomClient='"+prenomClient+"', telephoneClient='"+telClient+"',emailClient='"+emailCLient+"',adresseClient='"+adresseClient+"',cpClient='"+cpClient+"',villeClient='"+villeClient+"' where idClient="+nbClient+"";
    qDebug()<<reqModif;
    reqAjouterClient.prepare(reqModif);
    reqAjouterClient.exec();
    viderLesChampsClient();
    chargerLesClients();
}


void MainWindow::on_pushButtonRechercherClient_clicked()
{
    qDebug()<<"void MainWindow::on_pushButtonRechercherClient_clicked()";
    int ligneActu=0;
    QString nomRecherche=ui->lineEditRechercheClient->text();
    if(!nomRecherche.isEmpty())
    {
    QSqlQuery reqRecherche;
    reqRecherche.prepare("select * from Client where upper(nomClient) like upper('%"+nomRecherche+"%') OR upper(prenomClient) like upper('%"+nomRecherche+"%') or telephoneClient like '%"+nomRecherche+"%'");
    if(reqRecherche.exec())
    {

        effacerTousLesClients();
        while(reqRecherche.next())//pour chaque client
        {
         ui->tableWidgetClient->setRowCount(ligneActu+1);
         //pour chaque champ à afficher
          for(int noCol=0;noCol<7;noCol++)
          {
             ui->tableWidgetClient->setItem(ligneActu,noCol,new QTableWidgetItem(reqRecherche.value(noCol+1).toString()));
          }
          ui->tableWidgetClient->item(ligneActu,0)->setData(32,reqRecherche.value("idClient").toString());
           ligneActu++;//on passe à la ligne suivante

        }//fin du pour chaque client
    }//fin du if
    else
    {
        qDebug()<<"pb req";
    }
    }
    else
    {
        ui->pushButtonRechercherClient->setEnabled(false);
    }

}

void MainWindow::on_pushButtonAjouterMachine_clicked()
{
    qDebug()<<"void MainWindow::on_pushButtonAjouterMachine_clicked()";
    int typeMachineInt;
    int idMaxMachine;
    QString maxId;
    QString idClientAct;
    QSqlQuery reqMaxIdMach;
    reqMaxIdMach.prepare("select ifnull(max(idReparation),100) from Reparation");
    if(reqMaxIdMach.exec())
    {
        reqMaxIdMach.first();
        idMaxMachine=reqMaxIdMach.value(0).toInt();
        idMaxMachine++;
        maxId=QString::number(idMaxMachine);
    }
    dateArrivee=dateArrivee.currentDate();
    QString dateARentrer=dateArrivee.toString("yyyy/MM/dd");
    nomMachineSelectionnee=ui->lineEditNomMachine->text().replace("'","\\'");
    marqueMachine=ui->lineEditMarque->text().replace("'","\\'");
    refMachine=ui->lineEditReference->text();
    //refMachine=ui->lineEditReference->property("numeroModele").toString();
    panneMachine=ui->lineEditPanne1->text();
    panneMachine=panneMachine+"&&"+ui->lineEditPanne2->text().replace("'","\\'");
    panneMachine=panneMachine+"&&"+ui->lineEditPanne3->text().replace("'","\\'");
    panneMachine=panneMachine+"&&"+ui->lineEditPanne4->text().replace("'","\\'");
    //panneMachine=panneMachine+"&&"+ui->lineEditPanne5->text().replace("'","\\'");
    QString tempsPasse=QString::number(ui->lineEditHPasse->text().toInt()*60+ui->lineEditMinPasse->text().toInt());

    if(ui->radioButtonElectrique->isChecked())
    {
        typeMachineInt=0;
    }
    if(ui->radioButtonThermique->isChecked())
    {
        typeMachineInt=1;
    }
    idClientAct=QString::number(chercherIdClient());
    typeMachine=QString::number(typeMachineInt);
    //s'il y a besoin créer la marque
    QString txtReqIdMarque="select idMarque from Marque where upper(libelleMarque)=upper('"+marqueMachine+"')";
    qDebug()<<txtReqIdMarque;
    QSqlQuery reqIdMarque(txtReqIdMarque);
    reqIdMarque.exec();
    if(reqIdMarque.size()==0)
    {
        QSqlQuery reqObtientIdMarque("select ifnull(max(idMarque),0)+1 from Marque");
        reqObtientIdMarque.first();
        QString sonId=reqObtientIdMarque.value(0).toString();
        QString txtReqInsMarque= "insert into Marque values("+sonId+",'"+marqueMachine+"')";
        qDebug()<<txtReqInsMarque;
        QSqlQuery reqInsertMarque;
        reqInsertMarque.exec(txtReqInsMarque);
        qDebug()<<reqInsertMarque.lastError().text();
    }
    //obtention de l'identifiant de la marque
    reqIdMarque.exec();
    reqIdMarque.first();
    QString idMarque=reqIdMarque.value(0).toString();
    //recherche du modèle
    QString textReaq="select idModel from Modele where upper(codeModel)=upper('"+refMachine+"') and marque ="+idMarque;
    qDebug()<<textReaq;
    QSqlQuery reqIdModele(textReaq);
    reqIdModele.exec();
    qDebug()<<reqIdModele.lastError().text();
    //s'il y a besoin créer le modèle
    if(reqIdModele.size()==0)
    {
        QSqlQuery reqObtientIdModele("select ifnull(max(idModel),1)+1 from Modele");
        reqObtientIdModele.first();
        QString idDuModele=reqObtientIdModele.value(0).toString();
        qDebug()<<"creation du modele"<<idDuModele;
        QString txtRaq="insert into Modele(idModel,codeModel,marque,nature,typeMoteur) values("+idDuModele+",'"+refMachine+"',"+idMarque+",'"+nomMachineSelectionnee+"',"+typeMachine+")";
        qDebug()<<txtRaq;
        QSqlQuery reqInsertModele;
        reqInsertModele.exec(txtRaq);
        qDebug()<<reqInsertModele.lastError().text();
    }
    reqIdModele.exec();
    reqIdModele.first();
    QString idModele=reqIdModele.value(0).toString();
    //créer la réparation
    QSqlQuery insertMachine;
/*CREATE TABLE `Reparation`(`idReparation` int(11),`outilNom` varchar(45),`outilType` tinyint(2),`panneReparation` varchar(255),`outilRef` integer not null references Modele(idModel),`dateArrivee` DATE,`tempsPasse` int(11),`dateFinalisation` date,`idClient` int(11) NOT NULL,`idDevis` int(11) NOT NULL,`idEtat` int(11) NOT NULL,`refProduit` varchar(45) ,`idUtilisateur` int(11) NOT NULL, foreign key (`idClient`) references Client(`idClient`), foreign key (`idDevis`) references Devis_Reparation(`idDevis`), foreign key (`idEtat`) references Etat_Reparation(`idEtat`), foreign key (`refProduit`) references Produit(`refProduit`), foreign key (`idUtilisateur`) references Utilisateur(`idUtilisateur`),primary key(`idReparation`));
 */
    QString txtReq="insert into Reparation (idReparation,panneReparation,outilRef,dateArrivee,idClient,idEtat,idDevis,idUtilisateur,tempsPasse) "
                   "values("+maxId+",'"
                   +panneMachine+"',"
                   +idModele+",'"
                   +dateARentrer+"',"
                   +idClientAct+",1,NULL,NULL,"+tempsPasse+")";//plannifier,pas de devis, pas de mecano affecté
    insertMachine.prepare(txtReq);
    qDebug()<<txtReq;
    insertMachine.exec();
    qDebug()<<insertMachine.lastError();

//raz des zones de saisie
    ui->lineEditNomMachine->setText("");
    ui->lineEditReference->setText("");
    ui->lineEditMarque->setText("");
    ui->lineEditPanne1->setText("");
    ui->lineEditPanne2->setText("");
    ui->lineEditPanne3->setText("");
    ui->lineEditPanne4->setText("");
    //ui->lineEditPanne5->setText("");
    //impression de l'étiquette
    imprimeMachine();
    chargerLesMachines();
}



void MainWindow::on_pushButtonDeselectionner_2_clicked()
{
     viderLesChampsClient();
}



void MainWindow::on_listWidgetResultatRecherche_itemActivated(QListWidgetItem *item)
{
    qDebug()<<"void MainWindow::on_listWidgetResultatRecherche_itemActivated(QListWidgetItem *item)";
    //en 32 l'identifiant, 33 le modèle, 34 la marque, 35 la nature
    ui->lineEditReference->setProperty("numeroModele",item->data(32).toString());
    ui->lineEditReference->setText(item->data(33).toString());   
    ui->lineEditMarque->setText(item->data(34).toString());
    ui->lineEditNomMachine->setText(item->data(35).toString());
    //activation du bouton ajouter machine
    ui->pushButtonAjouterMachine->setEnabled(true);
}

void MainWindow::on_action_Fermer_triggered()
{
    close();
}
void MainWindow::imprimeMachine()
{
    qDebug()<<"void MainWindow::imprimeMachine()";
    //impression de l'étiquette
    QTextDocument etiquette;
    QStringList listeDesLignes;
    listeDesLignes<<"<body><center><h1>A.O.R</h1></center>";
    listeDesLignes<<"<div>";
    listeDesLignes<<"<table border=\"1\">";
    listeDesLignes<<"<tr><td>Nature:</td><td>"+nomMachineSelectionnee+"</td></tr>";
    listeDesLignes<<"<tr><td>Modèle:</td><td>"+refMachine+"</td></tr>";
    listeDesLignes<<"<tr><td>Client:</td><td>"+nomClient+" "+ prenomClient+"</td></tr>";
    listeDesLignes<<"<tr><td>tel:</td><td>"+telClient+"</td></tr>";
    listeDesLignes<<"<tr><th colspan=\"2\">Panne(s):</th></tr><tr><td colspan=\"2\"><ul>";
    QStringList listPanne=panneMachine.replace("&&","\n").split("\n",QString::SkipEmptyParts);
    for (int noPanne=0;noPanne<listPanne.count();noPanne++)
    {
        listeDesLignes<<"<li>"+listPanne[noPanne]+"</li>";
    }
    listeDesLignes<<"</ul></td></tr>";
    QString dateARentrer=dateArrivee.toString("dd/MM/yyyy");
    listeDesLignes<<"<tr><td>Arrivée:</td><td>"+dateARentrer+"</td></tr>";
    listeDesLignes<<"</table></div></body>";

    etiquette.setHtml(listeDesLignes.join("\r\n"));
    QPrinter printer;
    printer.setPageSize(QPagedPaintDevice::A6);
    QPrintDialog printDialog(&printer);
    if(printDialog.exec()==QDialog::Accepted)
     {
        etiquette.print(&printer);
    }
}

void MainWindow::on_pushButtonAddTechnicien_clicked()
{
  qDebug()<<"void MainWindow::on_pushButtonAddTechnicien_clicked()";
  //ajout d'un technicien
  tableModelTechnicien->setEditStrategy(QSqlRelationalTableModel::OnRowChange);
   QSqlRecord nouveauTechnicien=tableModelTechnicien->record();
   QSqlQuery reqProchainIdTech("select ifnull(max(idUtilisateur),0)+1 from Utilisateur");
   reqProchainIdTech.first();
   QString sonId=reqProchainIdTech.value(0).toString();
   QSqlQuery reqInsertTech;
   reqInsertTech.exec("insert into Utilisateur values("+sonId+",'sonNom',1,false)");

   /*nouveauTechnicien.setValue("idUtilisateur",sonId);
   nouveauTechnicien.setValue("nomUtilisateur","son nom");
   nouveauTechnicien.setValue("typeUtilisateur",1);
   nouveauTechnicien.setValue("supprime",false);
   //tableModelTechnicien->insertRow(tableModelTechnicien->rowCount());
   tableModelTechnicien->insertRecord(tableModelTechnicien->rowCount(),nouveauTechnicien);*/
   //tableModelTechnicien->setRecord(tableModelTechnicien->rowCount(),nouveauTechnicien);
   tableModelTechnicien->select();
   }

void MainWindow::on_tableWidgetMachine_cellClicked(int row, int column)
{
    qDebug()<<"void MainWindow::on_tableWidgetMachine_cellClicked(int row, int column)";
    //sélection d'une machine
    //on va ouvrir la modif de la machine    QWebView *view = new QWebView(parent);
    //mémoriser l'identifiant de la réparation en variable globale de la mainwindow
    idReparationSelectionnee=ui->tableWidgetMachine->item(row,0)->data(32).toString();
    qDebug()<<"reparation choisie"<<idReparationSelectionnee;
    //activation du bouton modifier
    ui->pushButtonModifierMachine->setEnabled(true);
    ui->pushButtonSupprMachine->setEnabled(true);
    ui->pushButtonAjouterPiece->setEnabled(true);
    //desactivation du bouton ajouter
    ui->pushButtonAjouterMachine->setEnabled(false);

    //recup des infos:
    QString idClient=ui->tableWidgetMachine->item(row,4)->data(32).toString();
    //mémoriser l'identifiant du client en variable globale de la mainwindow
    numeroClientSelectionne=idClient;
    emit(clientSelectionne(idClient));
    qDebug()<<"retour dans void MainWindow::on_tableWidgetMachine_cellClicked(int row, int column)";
    nomMachineSelectionnee=ui->tableWidgetMachine->item(row,0)->text();
    marqueMachine=ui->tableWidgetMachine->item(row,1)->text();
    refMachine=ui->tableWidgetMachine->item(row,2)->text();
    typeMachine=ui->tableWidgetMachine->item(row,3)->text();
    qDebug()<<ui->tableWidgetMachine->item(row,8)->text();
    dateArrivee=QDate::fromString(ui->tableWidgetMachine->item(row,8)->text(),"yyyy-MM-dd");
    qDebug()<<"date arrivee:"<<dateArrivee;
    dateSortie=QDate::fromString(ui->tableWidgetMachine->item(row,9)->text(),"yyyy-MM-dd");
    techMachine=ui->tableWidgetMachine->item(row,10)->text();
    //il y a le client en colonne 4 et on ne s'en sert pas
    panneMachine=ui->tableWidgetMachine->item(row,5)->text();
    qDebug()<<"Panne:"<<panneMachine;

    QString rEtat=ui->tableWidgetMachine->item(row,6)->text();
    QString rDevis=ui->tableWidgetMachine->item(row,7)->text();
    //en 8 il y a la date d'arrivée et on ne la modifie pas
    QString rDateSortie=ui->tableWidgetMachine->item(row,9)->text();
    QString rTechnicien=ui->tableWidgetMachine->item(row,10)->text();
    //on rempli les zones de saisie avec
    ui->lineEditNomMachine->setText(nomMachineSelectionnee);
    ui->lineEditMarque->setText(marqueMachine);
    ui->lineEditReference->setText(refMachine);
    if(typeMachine=="Electrique")ui->radioButtonElectrique->setChecked(true);
    else ui->radioButtonThermique->setChecked(true);
    //etat devis
    ui->comboBoxDevis->setCurrentText(rDevis);
    //TAF
    ui->comboBoxTaf->setCurrentText(rEtat);
    //Mecano
    ui->comboBoxMecano->setCurrentText(rTechnicien);
    //pannes
    QString rPanne=ui->tableWidgetMachine->item(row, 5)->text();
    QStringList listPannes=rPanne.split("\n");
    //vider les pannes et les afficher
    ui->lineEditPanne1->setText(listPannes[0]);
    ui->lineEditPanne2->setText(listPannes[1]);
    ui->lineEditPanne3->setText(listPannes[2]);
    ui->lineEditPanne4->setText(listPannes[3]);
    //afficher le client de la machine
    //obtention et affichage du temps passé
    QSqlQuery reqTemps ("select ifnull(tempsPasse,0) from Reparation where idreparation="+idReparationSelectionnee);
    reqTemps.first();
    QString heures=QString::number(reqTemps.value(0).toInt()/60);
    QString minutes=QString::number(reqTemps.value(0).toInt()%60);
    ui->lineEditHPasse->setText(heures);
    ui->lineEditMinPasse->setText(minutes);
    //activation des boutons voir client de la machine et imprimer
    ui->pushButtonImprimerMachine->setEnabled(true);
    ui->pushButtonVoirClient->setEnabled(true);
}

void MainWindow::on_pushButtonModifierMachine_clicked()
{
    qDebug()<<"void MainWindow::on_pushButtonModifierMachine_clicked()";
    //recup des infos nécessaires
    //identifiant de la réparation récupéré de la variable globale de la mainWindow
    QString sIdReparation=idReparationSelectionnee;
    //qDebug()<<"current text"<<ui->comboBoxMecano->currentText();

    QString mecano=ui->comboBoxMecano->currentData().toString();
    //qDebug()<<mecano;
    QString taf=ui->comboBoxTaf->currentData().toString();
    QString etatDevis=ui->comboBoxDevis->currentData().toString();
    //la panne
    QString laPanne=ui->lineEditPanne1->text()+"&&"+
            ui->lineEditPanne2->text()+"&&"+
            ui->lineEditPanne3->text()+"&&"+
            ui->lineEditPanne4->text();
    QString tempsPasse=QString::number(ui->lineEditHPasse->text().toInt()*60+ui->lineEditMinPasse->text().toInt());

    QString txtUpdate="update Reparation set idDevis="+etatDevis+" ,"+" idEtat="+taf+", idUtilisateur="+mecano+", panneReparation='"+laPanne+"', tempsPasse="+tempsPasse+" where idReparation="+sIdReparation;
    QSqlQuery reqUpdateMachine;
    if(reqUpdateMachine.exec(txtUpdate))
    {
        chargerLesMachines();
    }
    else
    {
        qDebug()<<reqUpdateMachine.lastError().text();
    }
    qDebug()<<txtUpdate;

}

void MainWindow::on_pushButtonSupprMachine_clicked()
{
    qDebug()<<"void MainWindow::on_pushButtonSupprMachine_clicked()";
    if(QMessageBox::warning(this,this->windowTitle(),"Etes-vous bien certain de vouloir supprimer cette Reparation?",QMessageBox::Yes|QMessageBox::No,QMessageBox::No)==QMessageBox::Yes)
    {
        QString txtSuppr="update Reparation set  idEtat=14 where idReparation="+idReparationSelectionnee;
        QSqlQuery reqSupprMachine;
        if(reqSupprMachine.exec(txtSuppr))
        {
            chargerLesMachines();
        }
        else
        {
            qDebug()<<reqSupprMachine.lastError().text();
        }
        qDebug()<<txtSuppr;
    }
}

void MainWindow::on_lineEditRechercheClient_textChanged(const QString &arg1)
{
    qDebug()<<"void MainWindow::on_lineEditRechercheClient_textChanged(const QString &arg1)";
    if(arg1.length()>2)
    {
        on_pushButtonRechercherClient_clicked();
        ui->labelListeClients->setText(tr("Liste filtrée des clients"));
        ui->labelListeClients->setStyleSheet("background-color:orange;");
    }
    else //critère de recherche insuffisant
    {
        ui->labelListeClients->setText(tr("Liste des clients"));
        ui->labelListeClients->setStyleSheet("");
        chargerLesClients();
    }
}

void MainWindow::on_pushButtonAjouterMachineClient_clicked()
{
    qDebug()<<"void MainWindow::on_pushButtonAjouterMachineClient_clicked()";
    ui->tabWidget->setCurrentIndex(1);
    viderChampsMachine();
    ui->lineEditNomMachine->setFocus();
}

void MainWindow::afficherClientSelectionne(QString sonNumero)
{
    qDebug()<<"void MainWindow::afficherClientSelectionne(QString sonNumero)";
    //affichage dans la zone de droite des infos concernant le client
    QSqlQuery reqChargerClient;
    QString txtReq="select nomClient,prenomClient,telephoneClient,emailClient,adresseClient,cpClient,villeClient,idClient from Client where not supprime and idClient="+sonNumero;
    qDebug()<<txtReq;
    reqChargerClient.prepare(txtReq);
    if(reqChargerClient.exec())
    {
        reqChargerClient.first();
        //affichage des infos dans la zone de modification
        nomClient=reqChargerClient.value("nomClient").toString();
        numeroClientSelectionne=sonNumero;
        prenomClient=reqChargerClient.value("prenomClient").toString();
        telClient=reqChargerClient.value("telephoneClient").toString();
        emailCLient=reqChargerClient.value("emailClient").toString();
        adresseClient=reqChargerClient.value("adresseClient").toString();
        cpClient=reqChargerClient.value("cpClient").toString();
        villeClient=reqChargerClient.value("villeClient").toString();
        //on vide les machines du client
        ui->tableWidgetMachineClient->setRowCount(0);
        //à revoir


        QString txtReq="select concat(nature,concat(' ',concat(libelleMarque,concat(' ',codeModel)))),date_format(dateArrivee,'%d/%m'),libelleEtat,if(dateFinalisation is null,datediff(now(),dateArrivee),'OK'),typeMoteur,concat(nomClient,concat(' ', prenomClient)), panneReparation, idDevis,dateFinalisation, nomUtilisateur,idReparation from Modele inner join Reparation on Reparation.outilRef=Modele.idModel inner join Marque on Marque.idMarque=Modele.marque inner join Client on Reparation.idClient=Client.idClient natural join Etat_Reparation left outer join Utilisateur on Reparation.idUtilisateur=Utilisateur.idUtilisateur where Client.idClient="+sonNumero+" order by Reparation.idEtat asc, dateArrivee desc";
        qDebug()<<txtReq;
        QSqlQuery chercherMachineDuClient;
        chercherMachineDuClient.prepare(txtReq);
        if(chercherMachineDuClient.exec())
        {

            int nbLigne=chercherMachineDuClient.size();
            if(nbLigne>0)
            {

                ui->tableWidgetMachineClient->setRowCount(nbLigne);

                int noLigne=0;
                while(chercherMachineDuClient.next())
                {
                    QSqlRecord enregistrement=chercherMachineDuClient.record();
                    //pour chaque champ à afficher
                    for(int noCol=0;noCol<enregistrement.count();noCol++)
                    {
                        ui->tableWidgetMachineClient->setItem(noLigne,noCol,new QTableWidgetItem(chercherMachineDuClient.value(noCol).toString()));

                    }
                    ui->tableWidgetMachineClient->item(noLigne,0)->setData(32,enregistrement.value("idReparation").toString());
                    noLigne++;//on passe à la ligne suivante
                }
                //il a des machines, on peut les sélectionner
                ui->pushButtonVoirMachinesClient->setEnabled(true);
            }
            else
            {
                qDebug()<<"Ce client n'a pas de machine";
                ui->pushButtonVoirMachinesClient->setEnabled(false);
            }
            //activation du bouton ajouterMachineAuClient
            ui->pushButtonAjouterMachineClient->setEnabled(true);
        }
        else
        {
            qDebug()<<chercherMachineDuClient.lastError().databaseText();
        }

        ui->labelNomClient->setText(nomClient);
        ui->labelPrenomClient->setText(prenomClient);
        ui->labelTelClient->setText(telClient);
        ui->labelEmailClient->setText(emailCLient);
        ui->labelCPClient->setText(cpClient);
        ui->labelAdresseClient->setText(adresseClient);
        ui->labelVilleClient->setText(villeClient);


        ui->lineEditNom->setText(nomClient);
        ui->lineEditPrenom->setText(prenomClient);
        ui->lineEditTelephone->setText(telClient);
        ui->lineEditEmail->setText(emailCLient);
        ui->lineEditAdresse->setText(adresseClient);
        ui->lineEditCP->setText(cpClient);
        ui->lineEditVille->setText(villeClient);

        ui->pushButtonSupprimerClient->setEnabled(true);
        //ui->pushButtonAjouterMachine->setEnabled(true);
        ui->pushButtonModifierClient->setEnabled(true);
        ui->pushButtonDeselectionner->setEnabled(true);

    }

}
void MainWindow::actDesactBoutonsAjouterMachine()
{
    qDebug()<<"void MainWindow::actDesactBoutonsAjouterMachine()";
    bool activerAjoutMachine=true;
    activerAjoutMachine=activerAjoutMachine&& !ui->lineEditNomMachine->text().isEmpty();
    activerAjoutMachine=activerAjoutMachine&& !ui->lineEditMarque->text().isEmpty();
    activerAjoutMachine=activerAjoutMachine&& !ui->lineEditReference->text().isEmpty();
    ui->pushButtonAjouterMachine->setEnabled(activerAjoutMachine);
}

void MainWindow::on_pushButtonVoirClient_clicked()
{
    qDebug()<<"void MainWindow::on_pushButtonVoirClient_clicked()";
    //positionnement sur l'onglet clients
    ui->tabWidget->setCurrentIndex(0);
    //sélection du client
    int row=0;
    int nbClientsAffiches=ui->tableWidgetClient->rowCount();
    while(!(row==nbClientsAffiches || ui->tableWidgetClient->item(row,0)->data(32)== this->numeroClientSelectionne))
    {
        row++;
    }
    if(row<nbClientsAffiches)ui->tableWidgetClient->selectRow(row);
}

void MainWindow::on_pushButtonVoirMachinesClient_clicked()
{
    qDebug()<<"void MainWindow::on_pushButtonVoirMachinesClient_clicked()";
    //on passe sur l'onglet machines
    ui->tabWidget->setCurrentIndex(1);
    //filtrer pour n'afficher que les siennes ou positionner sur la sienne
    ui->lineEditRechercheMachine->setText(telClient);
    //on déclenche la recherche
    on_pushButtonRechercherMachine_clicked();
    //efface la sélection
    ui->tableWidgetMachine->clearSelection();
}
//recherche des machines d'un client
void MainWindow::on_pushButtonRechercherMachine_clicked()
{
    qDebug()<<"void MainWindow::on_pushButtonRechercherMachine_clicked()";
    //s'il a des machines
        ui->label_ListeMachines->setText(tr("Liste filtée des machines"));
        ui->label_ListeMachines->setStyleSheet("background-color:orange;");
        chargerLesMachines(ui->lineEditRechercheMachine->text());
}
//recherche de machine par noTel client ou nature ou modele
void MainWindow::on_pushButtonAnnulerRechercheMachine_clicked()
{
    qDebug()<<"void MainWindow::on_pushButtonAnnulerRechercheMachine_clicked()";
    this->idReparationSelectionnee=-1;
    //raz de la zone de filtrage
    chargerLesMachines();
    //raz du formulaire du bas machine
    viderChampsMachine();
    ui->label_ListeMachines->setText(tr("Liste des machines"));
    ui->label_ListeMachines->setStyleSheet("");
    ui->lineEditRechercheMachine->setText("");
}
void MainWindow::actDesactBoutonAjouterClient()
{
    qDebug()<<"void MainWindow::actDesactBoutonAjouterClient()";
    //contrôler la validité des champs saisis
    bool activer=true;
    activer =activer && !ui->lineEditAdresse->text().isEmpty();
    activer =activer && !ui->lineEditNom->text().isEmpty();
    activer =activer && !ui->lineEditPrenom->text().isEmpty();
    activer =activer && !ui->lineEditVille->text().isEmpty();
    activer =activer && !ui->lineEditCP->text().isEmpty();
    activer =activer && !ui->lineEditTelephone->text().isEmpty()&& ui->lineEditTelephone->text().length()>13;
    ui->pushButtonAjouterClient->setEnabled(activer);
}

void MainWindow::on_resetFormClient_clicked()
{
    qDebug()<<"void MainWindow::on_resetFormClient_clicked()";
    viderLesChampsClient();
}

void MainWindow::on_pushButtonDeselectionner_clicked()
{
    qDebug()<<"void MainWindow::on_pushButtonDeselectionner_clicked()";
    viderLesChampsClient();
    ui->tableWidgetClient->clearSelection();
    ui->pushButtonDeselectionner->setEnabled(false);
    ui->pushButtonVoirMachinesClient->setEnabled(false);
    ui->pushButtonSupprimerClient->setEnabled(false);
}

void MainWindow::on_pushButtonToutVoirClient_clicked()
{
    qDebug()<<"void MainWindow::on_pushButtonToutVoirClient_clicked()";
    ui->lineEditRechercheClient->clear();
    chargerLesClients();
}

void MainWindow::on_lineEditRechercheClient_textEdited(const QString &arg1)
{
    qDebug()<<"void MainWindow::on_lineEditRechercheClient_textEdited(const QString &arg1)";
    ui->pushButtonRechercherClient->setEnabled(!arg1.isEmpty());
}

void MainWindow::on_lineEditRechercheMachine_textEdited(const QString &arg1)
{
    qDebug()<<"void MainWindow::on_lineEditRechercheMachine_textEdited(const QString &arg1)";
    ui->pushButtonRechercherMachine->setEnabled(!arg1.isEmpty());
    if(arg1.length()>2)
        on_pushButtonRechercherMachine_clicked();
}

void MainWindow::on_lineEditCP_editingFinished()
{
    qDebug()<<"void MainWindow::on_lineEditCP_editingFinished()";
    if(! ui->lineEditCP->text().isEmpty())
    {
        modelVille.setQuery("select trim(nom) from localite where codePostal like '"+ui->lineEditCP->text()+"'");
        completerVille.setModel(&modelVille);
        completerVille.setCaseSensitivity(Qt::CaseInsensitive);
        ui->lineEditVille->setCompleter(&completerVille);
    }
}



void MainWindow::on_pushButtonAjouterPiece_clicked()
{
    qDebug()<<"void MainWindow::on_pushButtonAjouterPiece_clicked()";
    //ajouter une pièce nécessaire à la  réparation
    DialogAjoutPieceAReparation monDialog;
    if(monDialog.exec()==QDialog::Accepted)
    {

        //je choppe les infos et j'enregistre tout ça
        QString reference=monDialog.getReference().replace("'","\\'");
         QString libelle=monDialog.getLibelle().replace("'","\\'");
         QString quantiteNecessaire=monDialog.getQuantite();
         bool estStocke=monDialog.getStocke();
         QString stocke="false";
         if(estStocke) stocke="true";
         QString seuilAlerte="0";
         if(estStocke)
         {
             seuilAlerte=monDialog.getSeuilAlerte();
         }
         if(!reference.isEmpty()&&!libelle.isEmpty() && !quantiteNecessaire.isEmpty())
         {
             //c'est une nouvelle pièce à enregistrer
             //chopper le modèle de la machine en cours de reparation
             QString reqModele="select outilRef from Reparation where idReparation="+idReparationSelectionnee;
             qDebug()<<reqModele;
             QSqlQuery sQ(reqModele);
             sQ.first();
             QString idModele=sQ.value(0).toString();
             //je cree la pièce
             QSqlQuery reqMax("select ifnull(max(idProduit),0)+1 from Produit");
             reqMax.first();
             QString idProduit=reqMax.value(0).toString();

             QString txtInsPiece ="insert into Produit(idProduit,libelleProduit,stocke,reference,seuilalerte) values("+idProduit+",'"+libelle+"',"+stocke+",'"+reference+"',"+seuilAlerte+")";
             qDebug()<<txtInsPiece;
             QSqlQuery reqInsPiece(txtInsPiece);
             //rentrer les lignes dans convenir
             QString textReq="insert into convenir(idModel,idProduit) values ("+idModele+","+idProduit+")";
             qDebug()<<textReq;
             QSqlQuery reqInsConvenir(textReq);

             //rentrer la ligne dans necessiter
             QString textReqNec="insert into necessiter(idReparation,idProduit,qteNecessaire) values ("+idReparationSelectionnee+","+idProduit+","+quantiteNecessaire+")";
             qDebug()<<textReqNec;
             QSqlQuery reqInsNecessiter(textReqNec);

         }
    }
}

void MainWindow::on_pushButtonImprimerMachine_clicked()
{
    qDebug()<<"void MainWindow::on_pushButtonImprimerMachine_clicked()";
  imprimeMachine();
}
void MainWindow::selectionneMachineDansLaTableDesMachines(QString sonId)
{
    qDebug()<<"void MainWindow::selectionneMachineDansLaTableDesMachines(QString sonId)";
    int ligne=0;
    while(!(ui->tableWidgetMachine->item(ligne,0)->data(32).toString()==sonId))
    {
        ligne++;
    }
    //sélection de la ligne
     ui->tableWidgetMachine->selectRow(ligne);
    //ui->tableWidgetMachine->itemClicked(ui->tableWidgetMachine->item(ligne,0));
}

void MainWindow::on_tableWidgetMachineClient_cellClicked(int row, int column)
{
    qDebug()<<"void MainWindow::on_tableWidgetMachineClient_cellClicked(int row, int column)";
    //sélection d'une machine dans la table de droite
    //ui->tabWidget->setCurrentIndex(1);
    on_pushButtonVoirMachinesClient_clicked();
    //et selection de la bonne machine
    selectionneMachineDansLaTableDesMachines(ui->tableWidgetMachineClient->item(row,0)->data(32).toString());
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if(QMessageBox::warning(this,"GapReparation","êtes vous sûr de vouloir quitter l'application",QMessageBox::Yes|QMessageBox::No,QMessageBox::No)==QMessageBox::Yes)
    {
        event->accept();
    }
    else
        event->ignore();
}







void MainWindow::chargerVueEclatee()
{
  //  QWebView *view = new QWebView(parent);
  //     view->load(QUrl("http://qt.nokia.com/"));
  //    view->show();
}
void MainWindow::on_pushButtonModele_clicked()
{
  chargerVueEclatee();
}
