#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include <QTimer>

#include "inc/fmod.hpp"
#include "inc/fmod_errors.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void loadIni(bool isLoad);
    //void plotAllWav(char *fileName);

public slots:
    void onTimer();

private slots:
    void on_actionAdd_triggered();
    void on_actionList_triggered();
    void on_actionStop_triggered();
    void on_slTime_sliderMoved(int position);
    void on_lwList_doubleClicked(const QModelIndex &index);
    void on_actionMinus_triggered();
    void on_actionClear_triggered();

    void on_play_released();
    void on_next_released();
    void on_previous_released();

    void on_slVolume_sliderMoved(int position);
    void on_slVolume_valueChanged(int value);

    void on_mode_change_released();

    void on_find_but_clicked();

    void on_menu_but_clicked();

    void on_lwList_f_doubleClicked(const QModelIndex &index);

private:
    QTimer timer;
    QVector<float>wav;

    FMOD::System *system;
    FMOD::Sound *sound;
    FMOD::Channel *channel;
    FMOD_RESULT result;

    FMOD::DSP *mydsp;
    FMOD::ChannelGroup *mastergroup;

public:
    void setCurrentLrc();
    void setCurrentMenu();

private:
    QTextEdit *text;
    int previous_row = 0;
    int previousx_row = 0;

    bool find_menu = 0;
    bool doubleclick = 0;
    int mode_num = 1;

    QStringList menu_list;
};

#endif // MAINWINDOW_H
