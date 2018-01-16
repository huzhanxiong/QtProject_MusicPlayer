//////////////////////////////////////////////////////////////////////////
// Version:		1.0
// Date:	    2018-1-15
// Author:	    huzhanxiong
// Copyright:   huzhanxiong
// Desciption:
// A Local MusicPlayer.
//////////////////////////////////////////////////////////////////////////
#include "mainwindow.h"
#include <QTextStream>
#include <QFileDialog>
#include <QFile>
#include <QSettings>
#include <QDateTime>
#include <QMessageBox>
#include "cstdlib"
#include "ctime"
#include <QtWidgets>

#include <QDebug>
#include <QDir>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setupUi(this);
    connect(&timer, SIGNAL(timeout()), SLOT(onTimer()));
QDir dir;
qDebug() << dir.currentPath();
    result = FMOD::System_Create(&system);
    result = system->init(32, FMOD_INIT_NORMAL, 0);
    sound = 0;
    channel = 0;
    wav.resize(2048);

    ploter->setAdjustVal(0.5);
    ploter->outPut();

    slVolume->setValue(100);

    textEdit->setReadOnly(1);
    textEdit->setAlignment(Qt::AlignCenter);

    text = new QTextEdit(this);
    text->hide();//用于查找下句歌词
    lwList_f->hide();  //用于歌曲搜索

    loadIni(true);
}

MainWindow::~MainWindow()
{
    on_actionStop_triggered();
    loadIni(false);

    delete text;
}


void MainWindow::loadIni(bool isLoad)
{
    QSettings setter("./config.ini", QSettings::IniFormat);
    QStringList list;
    if (isLoad)
    {
        list = setter.value("sound/files", list).toStringList();
        for (int i = 0 ; i < list.size(); i++)
        {
            QStringList tmp = list[i].split(";");
            QListWidgetItem *item = new QListWidgetItem(tmp[0]);
            item->setData(Qt::UserRole, tmp[1]);

            lwList->addItem(item);

            menu_list << lwList->item(i)->text();
        }
    }
    else
    {
        for (int i = 0; i < lwList->count(); i++)
        {
            QListWidgetItem *item = lwList->item(i);
            list << item->text() + ";" + item->data(Qt::UserRole).toString();
            setter.setValue("sound/files", list);
        }
    }
}


void MainWindow::setCurrentLrc()//设置当前歌词显示
{
    //QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QString strFile;
    QString lrcName;

    if(!find_menu)
    {
        strFile = lwList->currentItem()->data(Qt::UserRole).toString();
        lrcName = strFile.remove(strFile.right(3)) + "lrc";

        text->clear();
        textEdit->clear();

        this->setWindowTitle(lwList->currentItem()->text());
    }
    else
    {
        strFile = lwList_f->currentItem()->data(Qt::UserRole).toString();
        lrcName = strFile.remove(strFile.right(3)) + "lrc";

        text->clear();
        textEdit->clear();

        this->setWindowTitle(lwList_f->currentItem()->text());
    }

    QFile file(lrcName);

    label->setText("music");

    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        textEdit->setText(QString::fromLocal8Bit("当前目录下未找到歌词文件"));
    }
    else
    {
        /*while(!file.atEnd())
        {
            QByteArray line = file.readLine();
            QString str = codec->toUnicode(line);
            textEdit->append(str);
        }*/
        QTextStream in(&file);
        text->setText(in.readAll());
        textEdit->setText(text->toPlainText());
    }
}


void MainWindow::onTimer()
{
    float *dat = new float[16384];
    system->getWaveData(dat, 16384, 0);
    //FMOD_System_GetSpectrum(system, dat, 8192, 1, FMOD_DSP_FFT_WINDOW_RECT); 频谱
    for (int i = 0, n = 0; i < 16384; i+= 8, n++)
        wav[n] = dat[i];
    ploter->outPut(wav);
    delete [] dat;

    if(channel)
    {
        uint pos;
        channel->getPosition(&pos, FMOD_TIMEUNIT_MS);
        slTime->setValue(pos);

        lbCurTime->setText(QString().sprintf(" %02d:%02d", (pos/60000)%60, (pos/1000)%60));

        //QString lyrics_time = QString().sprintf("%02d:%02d.%d", (pos/60000)%60, (pos/1000)%60, pos%1000);
        QString lyrics_time = QString().sprintf("%02d:%02d", (pos/60000)%60, (pos/1000)%60);
        if(text->find(lyrics_time.left(5)))
        {
            QString str = text->textCursor().block().text().replace(QRegExp("\\[\\d{2}:\\d{2}\\.\\d{2}\\]"),"");
            if(!str.isEmpty())
            label->setText(str);

            /*
            QTextCursor tc = textEdit->textCursor();
            //int position = 25;
            int position = textEdit->document()->findBlockByLineNumber(1);
            tc.setPosition(position,QTextCursor::MoveAnchor);
            textEdit->setTextCursor( tc );
            */
        }
        else if(text->find(lyrics_time.left(5), QTextDocument::FindBackward))
        {
            QString str = text->textCursor().block().text().replace(QRegExp("\\[\\d{2}:\\d{2}\\.\\d{2}\\]"),"");
            if(!str.isEmpty())
            label->setText(str);
        }

        bool isPlay;
        channel->isPlaying(&isPlay);
        if (!isPlay)
        {
            timer.stop();
            this->setWindowTitle(QString::fromLocal8Bit("瓦力音频播放器"));
            switch (mode_num)
            {
                case 0:
                    on_actionStop_triggered();
                    if(!find_menu)
                    {
                        QListWidgetItem *item = lwList->item(previous_row);
                        item->setBackgroundColor(Qt::white);
                    }
                    else
                    {
                        QListWidgetItem *item = lwList_f->item(previousx_row);
                        item->setBackgroundColor(Qt::white);
                    }
                    break;
                case 1:
                if(!find_menu)
                {
                    if (lwList->currentRow() < lwList->count() - 1)
                    {
                        lwList->setCurrentRow(lwList->currentRow() + 1);
                        doubleclick = 1;
                        on_play_released();
                    }
                    else
                    //on_actionStop_triggered();
                    {
                         lwList->setCurrentRow(0);
                         doubleclick = 1;
                         on_play_released();
                    }
                }
                else
                {
                    if (lwList_f->currentRow() < lwList_f->count() - 1)
                    {
                        lwList_f->setCurrentRow(lwList_f->currentRow() + 1);
                        doubleclick = 1;
                        on_play_released();
                    }
                    else
                    //on_actionStop_triggered();
                    {
                        lwList_f->setCurrentRow(0);
                        doubleclick = 1;
                        on_play_released();
                    }
                }
                    break;
                case 2:
                    doubleclick = 1;
                    on_play_released();
                    break;
                case 3:
                    srand((unsigned)time( NULL ));
                    if(!find_menu)
                        lwList->setCurrentRow(rand()%(lwList->count()));
                    else
                        lwList_f->setCurrentRow(rand()%(lwList_f->count()));
                    doubleclick = 1;
                    on_play_released();
                    break;
                default:
                    break;
            }
        }
    }
}

/*
void MainWindow::plotAllWav(char *fileName)
{
    FMOD_System_CreateSound(system, fileName, FMOD_2D | FMOD_SOFTWARE | FMOD_CREATESAMPLE, 0, &sound);
    uint bytes, len1, len2;
    void *ptr1, *ptr2;
    FMOD_Sound_GetLength(sound, &bytes, FMOD_TIMEUNIT_PCMBYTES);
    FMOD_Sound_Lock(sound, 0, bytes, &ptr1, &ptr2, &len1, &len2);

    bytes /= 2;
    int step= 1, len= bytes;
    if (bytes > 10000)
    {
        len = 10000;
        step= (int)(bytes / len);
    }
    QVector<float>wavAll(10000);
    short* ps = (short*)ptr1;
    for (int i= 0, n = 0; n< len; i+= step, n++)
        wavAll[n] = ps[i];
    FMOD_Sound_Unlock(sound, ptr1, ptr2, len1, len2);
    FMOD_Sound_Release(sound);
    ploterAll->outPut(wavAll);
}
*/

void MainWindow::on_actionAdd_triggered()
{
    /*
    QString strPath = QFileDialog::getExistingDirectory(0, QString::fromLocal8Bit("请选择歌曲路径"));
    if (strPath.isNull() || strPath.isEmpty())
        return;
    QDir dir(strPath);
    QStringList list;
    list << "*.mp3" << "*.wma" << "*.ape" << "*.flac" << "*aac" << "*.amr" << "*.ogg" << "*.wav";

    QFileInfoList listFiles = dir.entryInfoList(list, QDir::Files, QDir::Name);
    int len = listFiles.size();
    if (len <= 0)
        return;
    for (int i = 0; i < len; i++)
    {
        QListWidgetItem *item = new QListWidgetItem(listFiles[i].fileName().split(".")[0]);
        item->setData(Qt::UserRole, listFiles[i].absoluteFilePath());
        lwList->addItem(item);
    }
    */
    QStringList list = QFileDialog::getOpenFileNames(this, QString::fromLocal8Bit("选取加入菜单歌曲"), "../",
                        QString(" mp3(*.mp3) ;; wav(*.wav) ;; ape(*.ape) ;;\
                        flac(*.flac) ;; acc(*.acc) ;; amr(*.amr) ;;\
                        ogg(*.ogg) ;; wma(*.wma) "));
                        // type(*.mp3 *.wav *.ape *.flac *.acc *.amr *.ogg *.wma)
    int len = list.size();
    if (len <= 0)
        return;
    for (int i=0; i < len; i++)
    {
        QFileInfo info(list[i]);

        QString file_type = "." + info.suffix();
        QListWidgetItem *item = new QListWidgetItem(info.fileName().split(file_type)[0]);
        item->setData(Qt::UserRole, info.absoluteFilePath());

        bool repeat = 0;
        if(lwList->count()>0)
        {
            for(int i=0; i<lwList->count(); i++)
                if(item->text() == lwList->item(i)->text())
                {
                    repeat = 1;
                    break;
                }
        }
        if(!repeat)
        {
            lwList->addItem(item);
            menu_list << item->text();
        }
    }
}


void MainWindow::on_actionMinus_triggered()
{
    int row = lwList->currentRow();
    if (row == -1)
        return;
    QListWidgetItem *item = lwList->item(row);
    lwList->removeItemWidget(item);
    menu_list.removeAt(row);
    delete item;
}


void MainWindow::on_actionClear_triggered()
{
    lwList->clear();
}


void MainWindow::on_actionList_triggered()
{
    //lwList->setVisible(!lwList->isVisible());
}


void MainWindow::on_actionStop_triggered()
{
    if (channel)
    {
        play->setIcon(QIcon(":/images/uplay.png"));
        play->setToolTip(QString::fromLocal8Bit("播放"));
        channel->stop();
        channel = NULL;
        timer.stop();
        lbCurTime->setText(" 00:00");
        lbTotalTime->setText("00:00 ");
        slTime->setValue(0);
        ploter->outPut();
    }
}


void MainWindow::on_slTime_sliderMoved(int position)
{
    bool play_sta;
    channel->isPlaying(&play_sta);

    if(play_sta)
        channel->setPosition(position, FMOD_TIMEUNIT_MS);

    if(position>1000)
    {
        uint posx = position;
        QString lyrics_timex;
        QString str;

        while(posx>1000)
        {
            lyrics_timex = QString().sprintf("%02d:%02d", (posx/60000)%60, (posx/1000)%60);
            if(text->find(lyrics_timex.left(5)))
            {
                str = text->textCursor().block().text().replace(QRegExp("\\[\\d{2}:\\d{2}\\.\\d{2}\\]"),"");
                if(!str.isEmpty())
                    break;
            }

            else if(text->find(lyrics_timex.left(5), QTextDocument::FindBackward))
            {
                str = text->textCursor().block().text().replace(QRegExp("\\[\\d{2}:\\d{2}\\.\\d{2}\\]"),"");
                if(!str.isEmpty())
                    break;
            }
            else
                posx -= 1000;
        }
        if(!str.isEmpty())
        label->setText(str);
    }
}


void MainWindow::on_lwList_doubleClicked(const QModelIndex &index)
{
    doubleclick = 1;
    on_play_released();
}


void MainWindow::on_lwList_f_doubleClicked(const QModelIndex &index)
{
    doubleclick = 1;
    on_play_released();
}


void MainWindow::on_play_released()
{
    bool play_state;
    channel->isPlaying(&play_state);

    QString strFile;

    if(lwList->count()!=0 || play_state || lwList_f->count()!=0)
    {
        if (channel && !doubleclick)
        {
            bool pause;
            channel->getPaused(&pause);
            channel->setPaused(!pause);
            //actionPause->setChecked(!pause);
            (!pause) ? timer.stop() : timer.start(300);
            if(!pause)
            {
                play->setIcon(QIcon(":/images/uplay.png"));
                play->setToolTip(QString::fromLocal8Bit("播放"));
            }
            else
            {
                play->setIcon(QIcon(":/images/upluse.png"));
                play->setToolTip(QString::fromLocal8Bit("暂停"));
            }
        }

        else
        {
            if(!find_menu)
            {
                int row = lwList->currentRow();
                if (row == -1)
                    row = 0;
                lwList->setCurrentRow(row);

                QListWidgetItem *itemx = lwList->item(previous_row);
                itemx->setBackgroundColor(Qt::white);

                QListWidgetItem *item = lwList->item(row);
                item->setBackgroundColor(Qt::cyan);
                previous_row = row;

                on_actionStop_triggered();
                strFile = lwList->currentItem()->data(Qt::UserRole).toString();

                if (!QFile::exists(strFile))
                {
                    if (row >= lwList->count() - 1)
                        return;
                    lwList->setCurrentRow(row + 1);
                    on_play_released();
                }
            }
            else
            {
                int row = lwList_f->currentRow();
                if (row == -1)
                    row = 0;
                lwList_f->setCurrentRow(row);

                QListWidgetItem *itemx = lwList_f->item(previousx_row);
                itemx->setBackgroundColor(Qt::white);

                QListWidgetItem *item = lwList_f->item(row);
                item->setBackgroundColor(Qt::cyan);
                previousx_row = row;

                on_actionStop_triggered();
                strFile = lwList_f->currentItem()->data(Qt::UserRole).toString();

                if (!QFile::exists(strFile))
                {
                    if (row >= lwList_f->count() - 1)
                        return;
                    lwList_f->setCurrentRow(row + 1);
                    on_play_released();
                }

                setCurrentMenu();
            }
            //plotAllWav(strFile.toLocal8Bit().data());

            sound->release();
            system->createSound(strFile.toLocal8Bit().data(), FMOD_SOFTWARE | FMOD_CREATESAMPLE, 0, &sound);
            sound->setMode(FMOD_LOOP_OFF);
            system->playSound(FMOD_CHANNEL_FREE, sound, false, &channel);
            uint length;
            sound->getLength(&length, FMOD_TIMEUNIT_MS);
            slTime->setMaximum(length);
            length /= 1000;
            lbTotalTime->setText(QString().sprintf("%02d:%02d ", length/60, length%60));
            system->update();

            int position = slVolume->value();
            channel->setVolume((float)position/100);
            timer.start(300);

            play->setIcon(QIcon(":/images/upluse.png"));
            play->setToolTip(QString::fromLocal8Bit("暂停"));
            doubleclick = 0;

            setCurrentLrc();
        }
    }
}


void MainWindow::on_next_released()
{
    QString strFile;

    if(lwList->count()!=0 || lwList_f->count()!=0)
    {
        if(mode_num == 3)
        {
            srand((unsigned)time( NULL ));

            if(!find_menu)
            {
                int row = rand()%(lwList->count());
                lwList->setCurrentRow(row);

                QListWidgetItem *itemx = lwList->item(previous_row);
                itemx->setBackgroundColor(Qt::white);
                QListWidgetItem *item = lwList->item(row);
                item->setBackgroundColor(Qt::cyan);
                previous_row = row;

                on_actionStop_triggered();
                strFile = lwList->currentItem()->data(Qt::UserRole).toString();
                if(!QFile::exists(strFile))
                    return;
            }
            else
            {
                int row = rand()%(lwList_f->count());
                lwList_f->setCurrentRow(row);

                QListWidgetItem *itemx = lwList_f->item(previousx_row);
                itemx->setBackgroundColor(Qt::white);
                QListWidgetItem *item = lwList_f->item(row);
                item->setBackgroundColor(Qt::cyan);
                previousx_row = row;

                on_actionStop_triggered();
                strFile = lwList_f->currentItem()->data(Qt::UserRole).toString();
                if(!QFile::exists(strFile))
                    return;

                setCurrentMenu();
            }
        }

        else
        {
            if(!find_menu)
            {
                int row = previous_row;
                row += 1;
                if(row >= lwList->count())
                    row = 0;
                    //return;
                lwList->setCurrentRow(row);
    
                QListWidgetItem *itemx = lwList->item(previous_row);
                itemx->setBackgroundColor(Qt::white);
                QListWidgetItem *item = lwList->item(row);
                item->setBackgroundColor(Qt::cyan);
                previous_row = row;

                on_actionStop_triggered();
                strFile = lwList->currentItem()->data(Qt::UserRole).toString();
                if(!QFile::exists(strFile))
                    return;
            }
            else
            {
                int row = previousx_row;
                row += 1;
                if(row >= lwList_f->count())
                    row = 0;
                    //return;
                lwList_f->setCurrentRow(row);

                QListWidgetItem *itemx = lwList_f->item(previousx_row);
                itemx->setBackgroundColor(Qt::white);
                QListWidgetItem *item = lwList_f->item(row);
                item->setBackgroundColor(Qt::cyan);
                previousx_row = row;

                on_actionStop_triggered();
                strFile = lwList_f->currentItem()->data(Qt::UserRole).toString();
                if(!QFile::exists(strFile))
                    return;

                setCurrentMenu();
            }
        }

        sound->release();
        system->createSound(strFile.toLocal8Bit().data(), FMOD_SOFTWARE | FMOD_CREATESAMPLE, 0, &sound);
        sound->setMode(FMOD_LOOP_OFF);
        system->playSound(FMOD_CHANNEL_FREE, sound, false, &channel);
        uint length;
        sound->getLength(&length, FMOD_TIMEUNIT_MS);
        slTime->setMaximum(length);
        length /= 1000;
        lbTotalTime->setText(QString().sprintf("%02d:%02d", length/60, length%60));
        system->update();

        int position = slVolume->value();
        channel->setVolume((float)position/100);
        timer.start(300);

        play->setIcon(QIcon(":/images/upluse.png"));
        play->setToolTip(QString::fromLocal8Bit("暂停"));
        doubleclick = 0;

        setCurrentLrc();//打开歌词文件
    }
}


void MainWindow::on_previous_released()
{
    QString strFile;

    if(lwList->count()!=0 || lwList_f->count()!=0)
    {
        if(mode_num == 3)
        {
            srand((unsigned)time( NULL ));

            if(!find_menu)
            {
                int row = rand()%(lwList->count());
                lwList->setCurrentRow(row);

                QListWidgetItem *itemx = lwList->item(previous_row);
                itemx->setBackgroundColor(Qt::white);
                QListWidgetItem *item = lwList->item(row);
                item->setBackgroundColor(Qt::cyan);
                previous_row = row;

                on_actionStop_triggered();
                strFile = lwList->currentItem()->data(Qt::UserRole).toString();
                if(!QFile::exists(strFile))
                    return;
            }
            else
            {
                int row = rand()%(lwList_f->count());
                lwList_f->setCurrentRow(row);

                QListWidgetItem *itemx = lwList_f->item(previousx_row);
                itemx->setBackgroundColor(Qt::white);
                QListWidgetItem *item = lwList_f->item(row);
                item->setBackgroundColor(Qt::cyan);
                previousx_row = row;

                on_actionStop_triggered();
                strFile = lwList_f->currentItem()->data(Qt::UserRole).toString();
                if(!QFile::exists(strFile))
                    return;

                setCurrentMenu();
            }
        }

        else
        {
            if(!find_menu)
            {
                int row = previous_row;
                row -= 1;
                if(row < 0)
                    row = lwList->count()-1;
                    //return;
                lwList->setCurrentRow(row);

                QListWidgetItem *itemx = lwList->item(previous_row);
                itemx->setBackgroundColor(Qt::white);
                QListWidgetItem *item = lwList->item(row);
                item->setBackgroundColor(Qt::cyan);
                previous_row = row;

                on_actionStop_triggered();
                strFile = lwList->currentItem()->data(Qt::UserRole).toString();
                if(!QFile::exists(strFile))
                    return;
            }
            else
            {
                int row = previousx_row;
                row -= 1;
                if(row < 0)
                    row = lwList_f->count()-1;
                    //return;
                lwList_f->setCurrentRow(row);

                QListWidgetItem *itemx = lwList_f->item(previousx_row);
                itemx->setBackgroundColor(Qt::white);
                QListWidgetItem *item = lwList_f->item(row);
                item->setBackgroundColor(Qt::cyan);
                previousx_row = row;

                on_actionStop_triggered();
                strFile = lwList_f->currentItem()->data(Qt::UserRole).toString();
                if(!QFile::exists(strFile))
                    return;

                setCurrentMenu();
            }
        }

        sound->release();
        system->createSound(strFile.toLocal8Bit().data(), FMOD_SOFTWARE | FMOD_CREATESAMPLE, 0, &sound);
        sound->setMode(FMOD_LOOP_OFF);
        system->playSound(FMOD_CHANNEL_FREE, sound, false, &channel);
        uint length;
        sound->getLength(&length, FMOD_TIMEUNIT_MS);
        slTime->setMaximum(length);
        length /= 1000;
        lbTotalTime->setText(QString().sprintf("%02d:%02d", length/60, length%60));
        system->update();
        timer.start(300);

        play->setIcon(QIcon(":/images/upluse.png"));
        play->setToolTip(QString::fromLocal8Bit("暂停"));
        doubleclick = 0;

        setCurrentLrc();//打开歌词文件
    }
}


void MainWindow::on_slVolume_sliderMoved(int position)
{
    if(position == 0)
        icon_Volume->setIcon(QIcon(":/images/volume0.png"));
    else if(position>0 && position<70)
        icon_Volume->setIcon(QIcon(":/images/volume1.png"));
    else
        icon_Volume->setIcon(QIcon(":/images/volume2.png"));
    channel->setVolume((float)position/100);
}


void MainWindow::on_slVolume_valueChanged(int value)
{
    float position = value;
    if(position == 0)
        icon_Volume->setIcon(QIcon(":/images/volume0.png"));
    else if(position>0 && position<70)
        icon_Volume->setIcon(QIcon(":/images/volume1.png"));
    else
        icon_Volume->setIcon(QIcon(":/images/volume2.png"));
    channel->setVolume((float)position/100);
}


void MainWindow::on_mode_change_released()
{
    mode_num++;
    if(mode_num > 3)
        mode_num = 0;

    switch (mode_num) {
    case 0:
        mode_change->setIcon(QIcon(":/images/uloop0.png"));
        mode_change->setToolTip(QString::fromLocal8Bit("单曲播放"));
        break;
    case 1:
        mode_change->setIcon(QIcon(":/images/uloop1.png"));
        mode_change->setToolTip(QString::fromLocal8Bit("列表循环"));
        break;
    case 2:
        mode_change->setIcon(QIcon(":/images/uloop2.png"));
        mode_change->setToolTip(QString::fromLocal8Bit("单曲循环"));
        break;
    case 3:
        mode_change->setIcon(QIcon(":/images/uloop3.png"));
        mode_change->setToolTip(QString::fromLocal8Bit("随机播放"));
        break;
    default:
        break;
    }
}


void MainWindow::on_find_but_clicked()
{
    QList<QListWidgetItem*> find_list;

    QString find_string = find_Edit->text();

    if(!find_string.isEmpty())
    {
        find_list = lwList->findItems(find_string, Qt::MatchContains);

        if(!find_list.isEmpty())
        {
            lwList->hide();
            lwList_f->show();
            find_but->setEnabled(false);
            menu_but->setEnabled(true);
            previousx_row = 0;

            for(int i=0; i<find_list.length(); i++)
            {
                QListWidgetItem *listItem = new QListWidgetItem(find_list.at(i)->text());
                listItem->setData(Qt::UserRole, find_list.at(i)->data(Qt::UserRole));
                lwList_f->addItem(listItem);
            }

            find_menu = 1;
        }
    }
}


void MainWindow::on_menu_but_clicked()
{   
    lwList_f->hide();
    lwList->show();
    lwList_f->clear();
    find_but->setEnabled(true);
    menu_but->setEnabled(false);

    find_menu = 0;
}


void MainWindow::setCurrentMenu()
{
    for(int i=0; i<menu_list.length(); i++)
    {
        if(menu_list.at(i) == lwList_f->currentItem()->text())
        {
            lwList->setCurrentRow(i);

            QListWidgetItem *itemx = lwList->item(previous_row);
            itemx->setBackgroundColor(Qt::white);

            QListWidgetItem *item = lwList->item(i);
            item->setBackgroundColor(Qt::cyan);
            previous_row = i;

            break;
        }
    }
}
