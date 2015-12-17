#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    this->setWindowTitle("Settings");
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::importOptions()
{
    QFile file("bombtimer.cfg");
    bool portOk = 1, soundOk = 1, soundIndexOk = 1, soundVolumeOk = 1, soundTimesOk = 1;

    //If can't load file. Load defaults.
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        emit sendToLog("Could not open \"bombtimer.cfg\" in program directory. Loading default values");
        port = defaultPort;
        sound = defaultSound;
        soundIndex = defaultSoundIndex;
        soundPath = defaultSoundPath;
        soundVolume = defaultSoundVolume;
        soundTimes << 7 << 12 << 15;

        //Set port to make the file be able to be created.
        ui->port_field->setText(QString::number(port));
        this->setModal(true);
        this->show();
        this->on_gamestateFile_browse_clicked();
        this->on_reloadCFG_button_clicked();
    }
    else
    {
        //Error checking!
        bool error = 0;
        QString errorString = "One or more settings from \" bombtimer.cfg\" was invalid, not found or had wrong format. Default value(s) used for: ";

        QStringList lineList, dummyTimes;
        QString line, option, value;
        bool gotPort = 0, gotSound = 0, gotSoundIndex = 0, gotSoundPath = 0, gotSoundVolume = 0, gotSoundTimes = 0;
        while(!file.atEnd())
        {
            //Get line.
            line = file.readLine();
            lineList = line.split(QRegExp("\t"));

            //Get the option string.
            if(lineList.size() == 2)
            {
                option = lineList.at(0);
                value = lineList.at(1);
                value = value.remove(QRegExp("[\n\r]"));

                //Check which option to read in.
                if(option == "port")
                {
                    port = value.toInt(&portOk);
                    gotPort = 1;
                }
                else if(option == "sound")
                {
                    sound = value.toInt();
                    gotSound = 1;
                }
                else if(option == "soundIndex")
                {
                    soundIndex = value.toInt(&soundIndexOk);
                    gotSoundIndex = 1;
                }
                else if(option == "soundPath")
                {
                    soundPath = value;
                    gotSoundPath = 1;
                }
                else if(option == "soundVolume")
                {
                    soundVolume = value.toInt(&soundVolumeOk);
                    gotSoundVolume = 1;
                }
                else if(option == "soundTimes")
                {
                    dummyTimes = value.split(",");
                    for(int i = 0; i < dummyTimes.size(); i++)
                        soundTimes << dummyTimes.at(i).toInt();
                    gotSoundTimes = 1;
                }
                else if(option == "gamestateFile")
                    gamestateFile = value;
            }
        }

        file.close();

//        qDebug() << "port: " << port
//                 << "Sound: " << sound
//                 << "soundIndex: " << soundIndex
//                 << "soudnPath: " << soundPath
//                 << "soundVolume: " << soundVolume
//                 << "soundTimes: " << soundTimes
//                 << "gamestateFile: " << gamestateFile;

        //Take care of the case when one item is not in file.
        if(!gotPort|| !gotSound || !gotSoundIndex || !gotSoundPath || !gotSoundVolume || !gotSoundTimes)
            error = 1;

        //Port number.
        if(!portOk || !gotPort)
        {
            this->setDefault("port");
            errorString += "\nport (3000)";
            error = 1;
        }
        //Sound checkbox.
        if(!soundOk || !gotSound)
        {
            this->setDefault("sound");
            errorString += "\nsound (0)";
            error = 1;
        }
        //Sound combobox.
        if((!soundIndexOk && soundIndex > 4) || !gotSoundIndex)
        {
            this->setDefault("soundIndex");
            errorString += "\nsoundIndex (0)";
            error = 1;
        }
        //Sound volume.
        if(!soundVolumeOk || !gotSoundVolume)
        {
            this->setDefault("soundVolume");
            errorString += "\nsoundVolume (75)";
            error = 1;
        }
        //Custom sound (does it exist if custom sound selected).
        if((soundIndex == soundFiles::custom && !QFile::exists(soundPath)) || !gotSoundPath)
        {
            this->setDefault("soundIndex");
            this->setDefault("soundPath");
            errorString += "\nsoundPath";
            error = 1;
        }

        //Sound times (comma seperated positive integers).
        for(int i = 0; i < soundTimes.size(); i++)
        {
            if(soundTimes.at(i) < 0)
            {
                error = 1;
                break;
            }
        }

        if(soundTimesOk == 0 || !gotSoundTimes)
        {
            this->setDefault("soundTimes");
            errorString += "\nsoundTimes (7,12,15,20)";
            error = 1;
        }

        //Don't need an error for location of gamestate file. Prompt user to select cfg folder.
        if(gamestateFile.isEmpty())
        {
            //Set port to make the file be able to be created.
            ui->port_field->setText(QString::number(port));
            this->setModal(true);
            this->show();
            this->on_gamestateFile_browse_clicked();
            this->on_reloadCFG_button_clicked();
        }

        //Update log with success or failure.
        if(error == 1)
            emit sendToLog(errorString);
        else
            emit sendToLog("Settings loaded successfully");

    }

    setUIValues();
}


void Dialog::setDefault(QString setting)
{
    if(setting == "port")
        port = defaultPort;
    else if(setting == "sound")
        sound = defaultSound;
    else if(setting == "soundIndex")
        soundIndex = defaultSoundIndex;
    else if(setting == "soundVolume")
        soundVolume = defaultSoundVolume;
    else if(setting == "soundPath")
        soundPath = defaultSoundPath;
    else if(setting == "soundTimes")
    {
        soundTimes.clear();
        soundTimes << 7 << 12 << 15 << 20;
    }
    else if(setting == "gamestateFile")
        gamestateFile = defaultGamestateFile;
}

void Dialog::saveOptions()
{
    QFile file("bombtimer.cfg");
    QTextStream output(&file);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        output << "port\t" << port << "\n"
               << "sound\t" << sound << "\n"
               << "soundIndex\t" << soundIndex << "\n"
               << "soundPath\t" << soundPath << "\n"
               << "soundVolume\t" << soundVolume << "\n"
               << "soundTimes\t";

        for(int i = 0; i < soundTimes.size(); i++)
        {
            output << QString::number(soundTimes.at(i));
            if(soundTimes.size() - i > 1)
                output << ",";
            else
                output <<"\n";
        }

        output << "gamestateFile\t" << gamestateFile;

        file.close();
    }
    else
        emit sendToLog("Could not save to \"bombtimer.cfg\" in program directory. Check permissions");
}

void Dialog::setUIValues()
{
    ui->port_field->setText(QString::number(port));
    ui->sound_checkbox->setChecked(sound);
    ui->sound_combobox->setCurrentIndex(soundIndex);
    ui->soundBrowse_field->setText(soundPath);
    ui->sound_slider->setValue(soundVolume);
    ui->soundVolume_label->setText(QString::number(soundVolume) + "%");

    QString soundTimesString;
    for(int i = 0; i < soundTimes.size(); i++)
    {
        soundTimesString += QString::number(soundTimes.at(i));
        if(soundTimes.size() - i > 1)
            soundTimesString += ",";
    }
    ui->soundTimes_field->setText(soundTimesString);
    QFile file(gamestateFile);
    QFileInfo fileInfo(file);
    ui->gamestateFile_field->setText(fileInfo.absoluteDir().path());
}

QString Dialog::getSound()
{
    QString soundToPlay;
    switch(soundIndex)
    {
    case soundFiles::ding:
        soundToPlay = "qrc:/Sounds/ding.wav";
        break;
    case soundFiles::dingding:
        soundToPlay = "qrc:/Sounds/dingding.wav";
        break;
    case soundFiles::beep:
        soundToPlay = "qrc:/Sounds/beep.wav";
        break;
    case soundFiles::beep2:
        soundToPlay = "qrc:/Sounds/beep2.wav";
        break;
    case soundFiles::loseYourself:
        soundToPlay = "qrc:/Sounds/loseYourself.wav";
        break;
    case soundFiles::custom:
        soundToPlay = ui->soundBrowse_field->text();
        soundPath = ui->soundBrowse_field->text();
        break;
    default:
        soundToPlay = "qrc:/Sounds/ding.wav";
        break;
    }

    return soundToPlay;
}

void Dialog::on_sound_slider_valueChanged(int value)
{
    ui->soundVolume_label->setText(QString::number(value) + "%");
}


void Dialog::on_sound_combobox_currentIndexChanged(int index)
{
    if(index == soundFiles::custom)
    {
        ui->soundBrowse_field->setEnabled(true);
        ui->soundBrowse_button->setEnabled(true);
    }
    else
    {
        ui->soundBrowse_field->setEnabled(false);
        ui->soundBrowse_button->setEnabled(false);
    }
}

void Dialog::on_save_button_clicked()
{
    bool errorPort = 0, errorTimes = 0, errorSound = 0;

    //Port.
    if(ui->port_field->text().toInt() <= 0)
    {
        errorPort = 1;
        ui->port_field->setStyleSheet("color: rgb(255, 0, 0); background-color: rgb(0, 0, 0); border: 1px solid; border-color: rgb(100, 100, 100);");
    }
    else
    {
        port = ui->port_field->text().toInt();
        ui->port_field->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb(0, 0, 0); border: 1px solid; border-color: rgb(100, 100, 100);");
    }

    //Sound stuff.
    sound = ui->sound_checkbox->isChecked();
    soundIndex = ui->sound_combobox->currentIndex();
    soundVolume = ui->sound_slider->value();

    //Custom file - for when we select custom as an option, we care about errors, otherwise we just grab it.
    QFile file(ui->soundBrowse_field->text());
    if(soundIndex == soundFiles::custom)
    {
        if(file.exists())
        {
            soundPath = ui->soundBrowse_field->text();
            ui->soundBrowse_field->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb(0, 0, 0); border: 1px solid; border-color: rgb(100, 100, 100);");
        }
        else
        {
            ui->soundBrowse_field->setStyleSheet("color: rgb(255, 0, 0); background-color: rgb(0, 0, 0); border: 1px solid; border-color: rgb(100, 100, 100);");
            ui->soundBrowse_field->setText("Failed to import file");
            errorSound = 1;
        }
    }
    else
        soundPath = ui->soundBrowse_field->text();

    //Reminder times. Go through with split comma, check each converts to int.
    if(sound)
    {
        QStringList times = ui->soundTimes_field->text().split(",");
        QVector<int> soundTimesTemp;
        for(int i = 0; i < times.size(); i++)
        {
            soundTimesTemp << times.at(i).toInt();
            if(times.at(i).toInt() < 0)
            {
                errorTimes = 1;
                break;
            }
        }

        if(errorTimes)
            ui->soundTimes_field->setStyleSheet("color: rgb(255, 0, 0); background-color: rgb(0, 0, 0); border: 1px solid; border-color: rgb(100, 100, 100);");
        else
        {
            soundTimes = this->sortQVector(soundTimesTemp);
            ui->soundTimes_field->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb(0, 0, 0); border: 1px solid; border-color: rgb(100, 100, 100);");
        }
    }

    if(errorTimes == 0 && errorPort == 0 && errorSound == 0)
    {
        ui->reloadCFG_button->setText("Reload gamestate .cfg");
        emit optionsChanged();
        saveOptions();
        this->hide();
    }
}

QVector<int> Dialog::sortQVector(QVector<int> vector)
{
    int temp = 0;
    for(int i = 0; i < vector.size(); i++)
    {
        for(int j = 0; j < vector.size() - 1; j++)
        {
            if(i != j)
            {
                if(vector.at(i) > vector.at(j))
                {
                    temp = vector.at(i);
                    vector[i] = vector.at(j);
                    vector[j] = temp;
                }
            }
        }
    }

    return vector;
}

void Dialog::on_gamestateFile_browse_clicked()
{
    //Get steam path.
    QString value = "SteamPath", path, folder;
    QSettings settings("HKEY_CURRENT_USER\\Software\\Valve\\Steam", QSettings::NativeFormat);
    path = settings.value(value).toString() + "";

    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setWindowTitle("Select the cfg folder in the CSGO installation directory:");
    dialog.setDirectory(path);
    if(dialog.exec())
        folder = dialog.directory().path();

    if(!folder.isEmpty())
        ui->gamestateFile_field->setText(folder);
}


bool Dialog::on_reloadCFG_button_clicked()
{
    QString folder = ui->gamestateFile_field->text();
    bool fileError = 0, error = 0;

    //Check if port is all g.
    if(ui->port_field->text().toInt() <= 0)
    {
        error = 1;
        port = defaultPort;
    }
    else
    {
        port = ui->port_field->text().toInt();
        ui->port_field->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb(0, 0, 0); border: 1px solid; border-color: rgb(100, 100, 100);");

        //Create output variables.
        QFile file(folder + "/gamestate_integration_bombtimer.cfg");
        QTextStream output(&file);

        if(file.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Truncate))
        {

            //Get filename.
            gamestateFile = file.fileName();

            //Write file based on port.
            QString text = "\"Console Sample v.1\"\n"
                           "{\n"
                           "\"uri\" \"http://127.0.0.1:" +
                           QString::number(port) + "\"\n"
                           "\"timeout\" \"5.0\"\n"
                           "\"buffer\"  \"0.1\"\n"
                           "\"throttle\" \"0.5\"\n"
                           "\"heartbeat\" \"60.0\"\n"
                           "\"data\"\n"
                           "{\n"
                           "\"round\" \"1\"\n"
                           "}\n"
                           "}";
            output << text;

            //Close file and say all g in button.
            file.close();
            ui->reloadCFG_button->setText(".cfg file created");
            sendToLog("Gamestate integration file created in " + folder);
        }
        else
            fileError = 1;
    }

    if(fileError)
    {
        ui->reloadCFG_button->setText("Could not create file");
        sendToLog("Gamestate integration file could not be created. Check folder permissions and try again");
        error = 1;
    }

    return error;
}

void Dialog::on_soundBrowse_button_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, "Select custom sound file to import: ", "", "Sound Files (*.mp3 *.m4a *.ogg *.wav *.wma)");

    if(!filename.isEmpty())
    {
        ui->soundBrowse_field->setText(filename);
        ui->soundBrowse_field->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb(0, 0, 0); border: 1px solid; border-color: rgb(100, 100, 100);");
    }
}

void Dialog::on_sound_checkbox_clicked(bool checked)
{
    ui->sound_combobox->setEnabled(checked);
    ui->sound_slider->setEnabled(checked);
    ui->soundBrowse_button->setEnabled(checked);
    ui->soundBrowse_field->setEnabled(checked);
    ui->soundTimes_field->setEnabled(checked);
}

int Dialog::getPort()
{
    return port;
}

int Dialog::getSoundVolume()
{
    return soundVolume;
}

QVector<int> Dialog::getSoundTimes()
{
    return soundTimes;
}

bool Dialog::soundEnabled()
{
    return sound;
}


QString Dialog::gamestateFileFound()
{
    return gamestateFile;
}

void Dialog::on_cancel_button_clicked()
{
    //Set fields back to what they were.
    setUIValues();
    this->hide();
}

void Dialog::on_defaults_button_clicked()
{
    //Set all fields to default, don't save until
    ui->port_field->setText(QString::number(defaultPort));
    ui->sound_checkbox->setChecked(defaultSound);
    ui->sound_combobox->setCurrentIndex(defaultSoundIndex);
    ui->soundTimes_field->setText("7,12,15,20");
    ui->soundBrowse_field->clear();
}

