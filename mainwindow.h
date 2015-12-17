#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "server.h"
#include "dialog.h"
#include <QMainWindow>
#include <QJsonArray>
#include <QTime>
#include <QGraphicsTextItem>
#include <QFont>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void stopTimer();
    void stopListening();
    void closePort();

public slots:
    void parseOutput(QString);
    void updateBombTime();
    void optionsChanged();
    void listeningStarted();
    void listeningError(QString);
    void updateLog(QString);
private slots:
    void on_start_button_clicked();
    void on_stop_button_clicked();
    void on_settings_button_clicked();

private:
    //Thread and timers.
    QThread* serverThread;
    server *listen;
    QTimer *updateTimer;
    int timerInterval = 10, bombTimePreset = 40000, bombTime = bombTimePreset, soundTimesIndex = 0;
    bool listening = 0, counting = 0, stopResidualTiming = 0, stateChanged;
    enum states{live, ctWinner, tWinner, planted, freeze, idle, initial};
    int status = states::initial;
    QGraphicsRectItem rect;

    void startCountdown();
    QString getInfo(QString parsed, QString key);
    void updateStatus();
    void updateStylesheet(QColor colour, QColor borderColour, QColor fontColour);
    QString getBombTimeText();
    void resetTimingParameters();

    //Style.
    int style;
    enum styleType{styleLive, styleCtWinner, styleTWinner, stylePlantedBlue, stylePlantedOrange, stylePlantedRed, styleFreeze, styleIdle, styleInitial};

    //Options.
    Dialog *options;
    int port, soundVolume;
    bool sound;
    QString soundPath;
    QVector<int> soundTimes;

    void getOptions();


    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
