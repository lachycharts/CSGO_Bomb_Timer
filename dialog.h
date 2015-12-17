#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QSound>
#include <QMediaPlayer>
#include <QSettings>
#include <QFileDialog>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();
    void importOptions();
    int getPort();
    int getSoundVolume();
    QString getSound();
    QVector<int> getSoundTimes();
    bool soundEnabled();
    QString gamestateFileFound();
    void saveOptions();

signals:
    void optionsChanged();
    void listeningStarted();
    void listeningError();
    void sendToLog(QString);

private slots:
    void on_sound_combobox_currentIndexChanged(int index);
    void on_save_button_clicked();
    bool on_reloadCFG_button_clicked();
    void on_soundBrowse_button_clicked();
    void on_sound_checkbox_clicked(bool checked);
    void on_gamestateFile_browse_clicked();
    void on_cancel_button_clicked();
    void on_defaults_button_clicked();

    void on_sound_slider_valueChanged(int value);

private:
    enum soundFiles{ding, dingding, beep, beep2, loseYourself, custom};
    int port, defaultPort = 3000;
    bool sound, defaultSound = 1;
    int soundIndex, defaultSoundIndex = 1, soundVolume, defaultSoundVolume = 75;
    QVector<int> soundTimes, defaultSoundTimes;
    QString soundPath, defaultSoundPath = "", gamestateFile, defaultGamestateFile = "";

    void setUIValues();
    void setDefault(QString setting);
    QVector<int> sortQVector(QVector<int> vector);

    Ui::Dialog *ui;
};

#endif // DIALOG_H
