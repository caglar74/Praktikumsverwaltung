#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QDate>
#include <QList>

struct Praktikum
{
    QString firma;
    QString ort;
    QString zeitraum;
    int tage;
};

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    Praktikum praktika[100];
    int anzahl = 0;

    void addPraktikum();
    void deletePraktikum();

    void saveToFile();
    void loadFromFile();

    int berechneTage(QString zeitraum);

    bool istFeiertag(QDate datum);

    QDate berechneOstersonntag(int jahr);

    void updateStatistik();
};

#endif
