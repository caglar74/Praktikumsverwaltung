#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->firma->setPlaceholderText("Betrieb");
    ui->ort->setPlaceholderText("Ort");

    ui->zeit->setPlaceholderText("01.01.2025 - 01.07.2027");

    connect(ui->add, &QPushButton::clicked,
            this, &MainWindow::addPraktikum);

    connect(ui->del, &QPushButton::clicked,
            this, &MainWindow::deletePraktikum);

    loadFromFile();
    updateStatistik();
}

MainWindow::~MainWindow()
{
    delete ui;
}

QDate MainWindow::berechneOstersonntag(int jahr)
{
    int a = jahr % 19;
    int b = jahr / 100;
    int c = jahr % 100;
    int d = b / 4;
    int e = b % 4;
    int f = (b + 8) / 25;
    int g = (b - f + 1) / 3;
    int h = (19 * a + b - d - g + 15) % 30;
    int i = c / 4;
    int k = c % 4;
    int l = (32 + 2 * e + 2 * i - h - k) % 7;
    int m = (a + 11 * h + 22 * l) / 451;

    int monat = (h + l - 7 * m + 114) / 31;
    int tag = ((h + l - 7 * m + 114) % 31) + 1;

    return QDate(jahr, monat, tag);
}

bool MainWindow::istFeiertag(QDate datum)
{
    int jahr = datum.year();

    QDate ostern = berechneOstersonntag(jahr);

    QList<QDate> feiertage =
        {
            QDate(jahr, 1, 1),    // Neujahr
            QDate(jahr, 5, 1),    // Tag der Arbeit
            QDate(jahr, 10, 3),   // Deutsche Einheit
            QDate(jahr, 12, 25),  // Weihnachten
            QDate(jahr, 12, 26),  // 2. Weihnachtstag
            QDate(jahr, 11, 1),

            ostern.addDays(-2),   // Karfreitag
            ostern.addDays(1),    // Ostermontag
            ostern.addDays(39),   // Christi Himmelfahrt
            ostern.addDays(50),   // Pfingstmontag
            ostern.addDays(60)    // Fronleichnam
        };

    return feiertage.contains(datum);
}

int MainWindow::berechneTage(QString zeitraum)
{
    QStringList teile = zeitraum.split("-");

    if (teile.size() != 2)
        return 0;

    QString startText = teile[0].trimmed();
    QString endeText = teile[1].trimmed();

    QDate start = QDate::fromString(startText, "dd.MM.yyyy");
    QDate ende = QDate::fromString(endeText, "dd.MM.yyyy");

    if (!start.isValid() || !ende.isValid())
        return 0;

    int arbeitstage = 0;

    for (QDate datum = start;
         datum <= ende;
         datum = datum.addDays(1))
    {
        int wochentag = datum.dayOfWeek();

        bool wochenende =
            (wochentag == 6 || wochentag == 7);

        if (!wochenende && !istFeiertag(datum))
        {
            arbeitstage++;
        }
    }

    return arbeitstage;
}

void MainWindow::addPraktikum()
{
    Praktikum p;

    p.firma = ui->firma->text();
    p.ort = ui->ort->text();
    p.zeitraum = ui->zeit->text();

    p.tage = berechneTage(p.zeitraum);

    if (p.firma.isEmpty())
        return;

    praktika[anzahl] = p;
    anzahl++;

    double wochen = p.tage / 5.0;

    QString text =
        p.firma + " | " +
        p.ort + " | " +
        p.zeitraum +
        " | " +
        QString::number(p.tage) +
        " Tage (" +
        QString::number(wochen, 'f', 1) +
        " Wochen)";

    ui->listWidget->addItem(text);

    ui->firma->clear();
    ui->ort->clear();
    ui->zeit->clear();

    saveToFile();
    updateStatistik();
}

void MainWindow::deletePraktikum()
{
    int row = ui->listWidget->currentRow();

    if (row < 0)
        return;

    delete ui->listWidget->takeItem(row);

    for (int i = row; i < anzahl - 1; i++)
    {
        praktika[i] = praktika[i + 1];
    }

    anzahl--;

    saveToFile();
    updateStatistik();
}

void MainWindow::updateStatistik()
{
    int gesamtTage = 0;

    for (int i = 0; i < anzahl; i++)
    {
        gesamtTage += praktika[i].tage;
    }

    double gesamtWochen = gesamtTage / 5.0;

    QString info;

    info += "Gesamt: ";
    info += QString::number(gesamtTage) +
            " Tage (" +
            QString::number(gesamtWochen, 'f', 1) +
            " Wochen)   ";

    int sommer = 50 - gesamtTage;
    int pruefung = 60 - gesamtTage;
    int klasse12 = 80 - gesamtTage;

    if (sommer <= 0)
    {
        info += "Sommerferien: Erledigt   ";
    }
    else
    {
        info += "Sommerferien: ";
        info += QString::number(sommer) +
                " Tage offen (" +
                QString::number(sommer /5.0, 'f', 1) +
                " Wochen)   ";
    }

    if (pruefung <= 0)
    {
        info += "Abschlussprüfungen: Erledigt   ";
    }
    else
    {
        info += "Abschlussprüfungen: ";
        info += QString::number(pruefung) +
                " Tage offen (" +
                QString::number(pruefung / 5.0, 'f', 1) +
                " Wochen)   ";
    }

    if (klasse12 <= 0)
    {
        info += "12. Klasse: Erledigt";
    }
    else
    {
        info += "12. Klasse: ";
        info += QString::number(klasse12) +
                " Tage offen (" +
                QString::number(klasse12 / 5.0, 'f', 1) +
                " Wochen)";
    }

    statusBar()->showMessage(info);
}

void MainWindow::saveToFile()
{
    QJsonArray array;

    for (int i = 0; i < anzahl; i++)
    {
        QJsonObject obj;

        obj["firma"] = praktika[i].firma;
        obj["ort"] = praktika[i].ort;
        obj["zeitraum"] = praktika[i].zeitraum;
        obj["tage"] = praktika[i].tage;

        array.append(obj);
    }

    QFile file("praktika.json");

    if (file.open(QIODevice::WriteOnly))
    {
        QJsonDocument doc(array);

        file.write(doc.toJson());

        file.close();
    }
}

void MainWindow::loadFromFile()
{
    QFile file("praktika.json");

    if (!file.open(QIODevice::ReadOnly))
         return;

    QJsonDocument doc =
        QJsonDocument::fromJson(file.readAll());

    QJsonArray array = doc.array();

    for (int i = 0; i < array.size(); i++)
    {
        QJsonObject obj = array[i].toObject();

        Praktikum p;

        p.firma = obj["firma"].toString();
        p.ort = obj["ort"].toString();
        p.zeitraum = obj["zeitraum"].toString();
        p.tage = obj["tage"].toInt();

        praktika[anzahl] = p;
        anzahl++;

        double wochen = p.tage / 5.0;

        QString text =
            p.firma + " | " +
            p.ort + " | " +
            p.zeitraum +
            " | " +
            QString::number(p.tage) +
            " Tage (" +
            QString::number(wochen, 'f', 1) +
            " Wochen)";

        ui->listWidget->addItem(text);
    }

    file.close();
}
