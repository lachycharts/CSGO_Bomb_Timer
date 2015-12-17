#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->show();

    //Get options.
    options = new Dialog;
    connect(options,SIGNAL(optionsChanged()), this, SLOT(optionsChanged()));
    connect(options, SIGNAL(sendToLog(QString)), this, SLOT(updateLog(QString)));
    options->importOptions();
    this->getOptions();
    status = states::initial;
}

MainWindow::~MainWindow()
{
    //Save options on close.
    options->saveOptions();
    delete ui;
}

void MainWindow::on_start_button_clicked()
{
    //Check if port is correct.
    if(port > 0)
    {
        //Set to idle.
        status = states::idle;
        updateStatus();

        //Create listening server in own thread.
        listen = new server;
        listen->setPort(port);
        serverThread = new QThread;
        listen->moveToThread(serverThread);
        connect(serverThread, SIGNAL(started()), listen, SLOT(startListen()));

        //What I want it to do.
        connect(listen, SIGNAL(sendCSGOOutput(QString)), this, SLOT(parseOutput(QString)));
        connect(listen, SIGNAL(listeningStarted()), this, SLOT(listeningStarted()));
        connect(listen, SIGNAL(listeningError(QString)), this, SLOT(listeningError(QString)));

        //Cleaning up after it's done.
        connect(this, SIGNAL(stopListening()), serverThread, SLOT(quit()));
        connect(this, SIGNAL(stopListening()), listen, SLOT(deleteLater()));
        connect(serverThread, SIGNAL(finished()), serverThread, SLOT(deleteLater()));

        //Start it bitch.
        serverThread->start();
    }
}

void MainWindow::on_stop_button_clicked()
{
    //Have to delete listen after otherwise port is being used, and I don't know how to stop it otherwise.
    emit stopListening();
    delete listen;
    listening = 0;

    //Stop timer if counting. Reset counting so if you press start soon after the timer reset. Innacurate, but continuity is maintained in a sense :)
    if(counting == 1)
    {
        emit stopTimer();
        counting = 0;
        bombTime = bombTimePreset;
    }

    //Enable the buttons and gui, etc.
    ui->start_button->setEnabled(true);
    ui->stop_button->setEnabled(false);
    ui->progressBar->setFormat("PRESS START TO LISTEN");
    status = states::initial;
    updateStatus();
    updateLog("Stopped listening");
}


void MainWindow::on_settings_button_clicked()
{
    //Show options window.
    options->setModal(true);
    const QPoint global = this->mapToGlobal(this->window()->rect().center());
    options->move(global.x() - options->width()/2, global.y() - options->height()/2);
    options->show();
}

void MainWindow::getOptions()
{
    //Get all options.
    port = options->getPort();
    sound = options->soundEnabled();
    soundVolume = options->getSoundVolume();
    soundTimes = options->getSoundTimes();
    soundPath = options->getSound();
}

void MainWindow::optionsChanged()
{
    //Check options.
    if(port != options->getPort())
    {
        port = options->getPort();

        //If we are listening, stop then start with new port.
        if(listening)
        {
            on_stop_button_clicked();
            on_start_button_clicked();
        }
    }

    //If sound options have changed.
    if(sound != options->soundEnabled())
        sound = options->soundEnabled();

    //Always update sound options if we are using sound.
    if(sound)
    {
        soundVolume = options->getSoundVolume();
        soundTimes = options->getSoundTimes();
        soundPath = options->getSound();
    }
}


void MainWindow::listeningStarted()
{
    //Update log, status and toggle buttons.
    updateLog("Started listening");
    listening = 1;
    ui->start_button->setEnabled(false);
    ui->stop_button->setEnabled(true);
}

void MainWindow::listeningError(QString error)
{
    //Output error in log and stop listening.
    updateLog("ERROR " + error);
    on_stop_button_clicked();
}

void MainWindow::updateLog(QString string)
{
    //Output when state changes, or given a string.
    if(string.isEmpty())
    {
        switch(status)
        {
        case states::live:
            string = "LIVE";
            break;
        case states::planted:
            string = "BOMB HAS BEEN PLANTED";
            break;
        case states::ctWinner:
            string = "COUNTER TERRORISTS WIN";
            break;
        case states::tWinner:
            string = "TERRORISTS WIN";
            break;
        case states::freeze:
            string = "FREEZE TIME";
            break;
        case states::initial:
            string = "INITIALISED";
            break;
        default:
            string = "IDLE STATE";
            break;
        }
    }

    QTime time = QTime::currentTime();
    ui->output_field->append(time.toString("hh:mm:ss")+ ": " + string);
}

void MainWindow::parseOutput(QString array)
{
    int previousStatus = status;

    //If error message, output log, otherwise sort it to get status.
    QString phase = getInfo(array, "phase");
    QString bomb = getInfo(array, "bomb");
    QString winningTeam;

    if(phase == "live")
    {
        status = states::live;

        if(bomb == "planted")
        {
            status = states::planted;

            if(counting == 0)
                startCountdown();
        }
    }
    else if(phase == "over")
    {
        //Stop counter if bomb was planted. We don't reset anything just yet, since this timer will stop befroe
        if(counting == 1)
        {
            emit stopTimer();
            resetTimingParameters();
        }

        //Get winning team.
        winningTeam = getInfo(array, "win_team");
        if(winningTeam == "T")
            status = states::tWinner;
        else if(winningTeam == "CT")
            status = states::ctWinner;
    }
    else if(phase == "freezetime")
        status = states::freeze;
    else
        status = states::idle;

    //Update the log if a new state change has occurred.
    if(previousStatus != status)
    {
        updateLog("");
        updateStatus();
    }
}

void MainWindow::updateStatus()
{
    //Determine styles, text, values to be displayed.
    QColor colour, borderColour, fontColour = QColor(255,255,255);
    QString text;
    double percent = double(bombTime)/bombTimePreset;

    //Determine if we have to change colour/style.
    int previousStyle = style;

    switch(status)
    {
    case states::live:
        style = styleType::styleLive;
        colour = QColor(0,120,0);
        borderColour = QColor(0,100,0);
        text = "LIVE";
        break;
    case states::planted:
        if(bombTime > 6000)
        {
            style = styleType::stylePlantedBlue;
            colour = QColor(50,130,230);
            borderColour = QColor(30,110,210);
        }
        else if(bombTime > 5000)
        {
            style = styleType::stylePlantedOrange;
            colour = QColor(240,170,10);
            borderColour = QColor(210,140,0);
        }
        else
        {
            style = styleType::stylePlantedRed;
            colour = QColor(250,30,30);
            borderColour = QColor(220,0,0);
        }

        //Update the bomb timer if bombtime is multiple of 0.1s (100ms).
        if((bombTime/10)%10 == 0)
        {
            ui->progressBar->setFormat(getBombTimeText());
            if(bombTime%400 == 0)
                ui->progressBar->setValue(percent * ui->progressBar->maximum());
        }
        break;
    case states::ctWinner:
        style = styleType::styleCtWinner;
        colour = QColor(74,117,181);
        borderColour = QColor(50,90,160);
        text = "COUNTER TERRORISTS WIN";
        break;
    case states::tWinner:
        style = styleType::styleTWinner;
        colour = QColor(196,174,111);
        borderColour = QColor(170,150,90);
        text = "TERRORISTS WIN";
        break;
    case states::freeze:
        style = styleType::styleFreeze;
        colour = QColor(50,50,50);
        borderColour = QColor(20,20,20);
        text = "FREEZE TIME";
        break;
    case states::idle:
        style = styleType::styleIdle;
        colour = QColor(30,30,30);
        borderColour = QColor(20,20,20);
        text = "IDLE STATE";
        break;
    case states::initial:
        style = styleType::styleInitial;
        colour = QColor(30,30,30);
        borderColour = QColor(20,20,20);
        text = "PRESS START TO LISTEN";
        break;
    default:
        style = styleType::styleIdle;
        colour = QColor(30,30,30);
        borderColour = QColor(20,20,20);
        text = "IDLE STATE";
        break;
    }

    //Apply stylesheet, update text and value;
    if(previousStyle != style)
    {
        updateStylesheet(colour, borderColour, fontColour);
        //If we update this while planted, it will do it twice, and f
        if(status != states::planted)
        {
            ui->progressBar->setValue(percent * ui->progressBar->maximum());
            ui->progressBar->setFormat(text);
        }
    }

}

QString MainWindow::getBombTimeText()
{
    //Format bomb time text to update progress bar.
    int seconds=  0, milliseconds = 0;
    QString text;
    seconds = bombTime/1000;
    milliseconds = bombTime%1000;
    text = QString::number(seconds) + "." + QString::number(milliseconds/100);
    return text;
}

void MainWindow::updateStylesheet(QColor colour, QColor borderColour, QColor fontColour)
{
    //I know, kill me now.
    QString stylesheet = "QProgressBar {\n"
            "border: 2px solid;\n"
            "color: rgb(" +
            QString::number(fontColour.red()) + "," + QString::number(fontColour.green()) + "," + QString::number(fontColour.blue())
            + ");\n"
            "width: 20px;\n"
            "font: bold 20px;\n"
            "border-color: rgb(" +
            QString::number(colour.red()) + "," + QString::number(colour.green()) + "," + QString::number(colour.blue()) +
            ");\n}\n"
            "QProgressBar::chunk{\n"
            "width: 1px;\n"
            "background-color: rgb(" +
            QString::number(borderColour.red()) + "," + QString::number(borderColour.green()) + "," + QString::number(borderColour.blue()) +
            ");\n}";

    ui->progressBar->setStyleSheet(stylesheet);
}

QString MainWindow::getInfo(QString parsed, QString key)
{
    QStringList newLineSplit = parsed.split("\n");
    QString lineKey, value;

    //Get line from csgo that has what I am looking for.
    for(int i = 0; i < newLineSplit.size(); i++)
    {
        if(newLineSplit.at(i).contains(QRegExp(key)))
            lineKey = newLineSplit.at(i);
    }

    //Remove all bullshit and get the value.
    if(!lineKey.isEmpty())
    {
        value = lineKey.remove(QRegExp("[\t\n,\"]"));
        value = value.split(" ").at(1);
    }

    return value;
}

void MainWindow::startCountdown()
{
    bombTime = bombTimePreset;
    QThread* updateThread = new QThread;
    updateTimer = new QTimer;
    updateTimer->setInterval(timerInterval);
    updateTimer->moveToThread(updateThread);
    disconnect(updateTimer, SIGNAL(timeout()), this, SLOT(updateBombTime()));       //What happens when the timer goes off.
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateBombTime()));
    connect(this, SIGNAL(stopTimer()), updateThread, SLOT(quit()));                 //Set it up for when it's finished.
    connect(this, SIGNAL(stopTimer()), updateTimer, SLOT(deleteLater()));
    connect(updateThread, SIGNAL(finished()), updateThread, SLOT(deleteLater()));
    connect(updateThread, SIGNAL(started()), updateTimer, SLOT(start()));           //Starting thread.
    updateThread->start();

    //Update flags.
    counting = 1;
    soundTimesIndex = 0;
}

void MainWindow::resetTimingParameters()
{
    bombTime = bombTimePreset;
    counting = 0;
    soundTimesIndex = 0;
}

void MainWindow::updateBombTime()
{
    if(bombTime > 0)
    {
        //Update bomb time.
        bombTime -= timerInterval;

        //Check if we have to play sound.
        if(sound && !soundTimes.isEmpty())
        {
            if(double(bombTime)/1000.0 == soundTimes.at(soundTimesIndex) && bombTime != 0)
            {
                QMediaPlayer *player = new QMediaPlayer;
                player->setMedia(QUrl(options->getSound()));
                player->setVolume(soundVolume);
                player->play();

                if(soundTimesIndex < soundTimes.size() - 1)
                    soundTimesIndex++;
            }
        }
    }

    updateStatus();
}

//void MainWindow::on_pushButton_clicked()
//{
//    emit startCountdown();
//    status = states::planted;
//    counting = 1;
//}

//void MainWindow::on_pushButton_2_clicked()
//{
//    QMediaPlayer *player = new QMediaPlayer;
//    player->setMedia(QUrl(options->getSound()));
//    player->setVolume(soundVolume);
//    player->play();
//}
