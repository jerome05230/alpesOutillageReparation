#ifndef DIALOGAJOUTPIECEAREPARATION_H
#define DIALOGAJOUTPIECEAREPARATION_H

#include <QDialog>

namespace Ui {
class DialogAjoutPieceAReparation;
}

class DialogAjoutPieceAReparation : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAjoutPieceAReparation(QWidget *parent = 0);
    ~DialogAjoutPieceAReparation();
    QString getReference();
    QString getLibelle();
    QString getQuantite();
    QString getSeuilAlerte();
    bool getStocke();

private slots:
    void on_pushButtonNewPiece_clicked();

    void on_checkBoxStocke_stateChanged(int arg1);

private:
    Ui::DialogAjoutPieceAReparation *ui;
    QString chargerPDF();
};

#endif // DIALOGAJOUTPIECEAREPARATION_H
