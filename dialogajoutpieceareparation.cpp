#include "dialogajoutpieceareparation.h"
#include "ui_dialogajoutpieceareparation.h"
#include <QSqlQuery>
#include <poppler-qt5.h>


DialogAjoutPieceAReparation::DialogAjoutPieceAReparation(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAjoutPieceAReparation)
{
    ui->setupUi(this);
    ui->groupBox->setVisible(false);
    //je cache la saisie du stock d'alerte
    ui->labelSeuilAlerte->setVisible(false);
    ui->spinBoxSeuilAlerte->setVisible(false);
    chargerPDF();
   // ui->webViewPiece->setContent();
}

DialogAjoutPieceAReparation::~DialogAjoutPieceAReparation()
{
    delete ui;
}

QString DialogAjoutPieceAReparation::getReference()
{
    return ui->lineEditPeferencePiece->text();
}

QString DialogAjoutPieceAReparation::getLibelle()
{
    return ui->lineEditLibellePiece->text();
}

QString DialogAjoutPieceAReparation::getQuantite()
{
    return QString::number(ui->spinBoxQuantiteNecessaire->value());
}

QString DialogAjoutPieceAReparation::getSeuilAlerte()
{
    return QString::number(ui->spinBoxSeuilAlerte->value());
}
bool DialogAjoutPieceAReparation::getStocke()
{
    return ui->checkBoxStocke->isChecked();
}

void DialogAjoutPieceAReparation::on_pushButtonNewPiece_clicked()
{
    ui->groupBox->setVisible(true);
    Poppler::Document *doc = Poppler::Document::load("file:///home/jbaroncampos/T%C3%A9l%C3%A9chargements/ms880.pdf");
    ui->webViewPiece->load(QUrl("file:///home/jbaroncampos/T%C3%A9l%C3%A9chargements/ms880.pdf"));
    ui->webViewPiece->show();
}

void DialogAjoutPieceAReparation::on_checkBoxStocke_stateChanged(int arg1)
{
    //on cache ou pas la saisie du stock d'alerte
    ui->spinBoxSeuilAlerte->setVisible(arg1!=0);
    ui->labelSeuilAlerte->setVisible(arg1!=0);
}
QString DialogAjoutPieceAReparation::chargerPDF()
{
    qDebug()<<"QString DialogAjoutPieceAReparation::chargerPDF()";
    QSqlQuery reqPdf("select nomDuFichier from Eclate E inner join Modele M on E.numeroEclate = M.numeroEclate;");
    reqPdf.first();
    QString pdf=reqPdf.value(0).toString();
    return pdf;
}
