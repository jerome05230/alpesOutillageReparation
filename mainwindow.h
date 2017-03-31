#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QDateEdit"
#include <QListWidgetItem>
#include <QSqlRelationalTableModel>
#include <QCompleter>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
signals:
    void clientSelectionne(QString);
public:
    explicit MainWindow(QWidget *parent = 0);
    QString numeroClientSelectionne,nomClient,prenomClient,telClient,emailCLient,adresseClient,cpClient,villeClient;
    QString nomMachineSelectionnee,marqueMachine,panneMachine,refMachine,etatMachine,typeMachine,clientMachine,devisMachine,techMachine,tempsPasse;
    QDate dateArrivee,dateSortie;
    QString idReparationSelectionnee;
    ~MainWindow();  
    void chargerLesTechniciens();
    void chargerLesEtatsReparation();
    void chargerLesEtatsDevis();
    void viderChampsMachine();

    void effacerTousLesClients();
    QSqlQueryModel modelCp;
    QCompleter completerCP;
    QSqlQueryModel modelVille;
    QCompleter completerVille;
    QSqlQueryModel modelPrenoms;
    QCompleter completerPrenoms;  
    QCompleter* completerPanne;

    void imprimeMachine();
    void selectionneMachineDansLaTableDesMachines(QString sonId);
    void chargerVueEclatee();
public slots:
    void actDesactBoutonAjouterClient();
    void actDesactBoutonsAjouterMachine();
private slots:
    void majListeRechercheMachine(QString);
    void on_focusChanged(QWidget *, QWidget*);
    void on_pushButtonAjouterClient_clicked();

    void on_pushButtonSupprimerClient_clicked();

    void on_tableWidgetClient_cellClicked(int row, int column);

    //void on_pushButtonDeselectionnerClient_clicked();

    void on_pushButtonModifierClient_clicked();

    void on_pushButtonRechercherClient_clicked();

    void on_pushButtonAjouterMachine_clicked();


    void on_pushButtonDeselectionner_2_clicked();

    void on_listWidgetResultatRecherche_itemActivated(QListWidgetItem *item);

    void on_action_Fermer_triggered();

    void on_pushButtonAddTechnicien_clicked();

    void on_tableWidgetMachine_cellClicked(int row, int column);

    void on_pushButtonModifierMachine_clicked();

    void on_pushButtonSupprMachine_clicked();

    void on_lineEditRechercheClient_textChanged(const QString &arg1);

    void on_pushButtonAjouterMachineClient_clicked();
    void afficherClientSelectionne(QString sonNumero);

    void on_pushButtonVoirClient_clicked();

    void on_pushButtonVoirMachinesClient_clicked();

    void on_pushButtonRechercherMachine_clicked();

    void on_pushButtonAnnulerRechercheMachine_clicked();

    void on_resetFormClient_clicked();

    void on_pushButtonDeselectionner_clicked();

    void on_pushButtonToutVoirClient_clicked();

    void on_lineEditRechercheClient_textEdited(const QString &arg1);

    void on_lineEditRechercheMachine_textEdited(const QString &arg1);

    void on_lineEditCP_editingFinished();
    void on_pushButtonAjouterPiece_clicked();
    void on_pushButtonImprimerMachine_clicked();
    void on_tableWidgetMachineClient_cellClicked(int row, int column);

    void closeEvent(QCloseEvent* event);

    void on_pushButtonModele_clicked();

private:
    Ui::MainWindow *ui;
    void chargerLesClients();

    int chercherIdClient();

    void viderLesChampsClient();

    void chargerLesMachines(QString filtre="");
    QSqlRelationalTableModel * tableModelTechnicien;

};

#endif // MAINWINDOW_H
