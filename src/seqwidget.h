/*
 *      seqwidget.h
 *      
 *      Copyright 2009 <alsamodular-devel@lists.sourceforge.net>
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#ifndef SEQWIDGET_H
#define SEQWIDGET_H

#include <QString>
#include <QComboBox>
#include <QSpinBox>
#include <QTextStream>
#include <QCheckBox>
#include <QAction>
#include <QToolButton>

#include "midiseq.h"
#include "slider.h"
#include "seqscreen.h"

const int seqResValues[5] = {1, 2, 4, 8, 16};

class SeqWidget : public QWidget
{
    Q_OBJECT

    QSpinBox *chIn;
    QSpinBox *channelOut, *portOut;
    QComboBox *waveFormBox, *resBox, *sizeBox, *freqBox;
    QAction *copyToCustomAction;
    QAction *deleteAction, *renameAction;
    QToolButton *copyToCustomButton;
 
    MidiSeq *midiSeq;
    Slider *velocity, *transpose, *notelength;
    QVector<SeqSample> seqData;
    bool modified;

  public:
    QString name;
    SeqScreen *seqScreen;
    QStringList waveForms;
    QCheckBox *muteOut;
    QCheckBox *enableNoteIn;               
    QCheckBox *enableVelIn; 
    int ID, parentDockID;
    
    SeqWidget(MidiSeq *p_midiSeq, int portCount, QWidget* parent=0);
    ~SeqWidget();
    MidiSeq *getMidiSeq();
    
    void readSeq(QTextStream& arpText);
    void writeSeq(QTextStream& arpText);
    void setChIn(int value);
    void setEnableNoteIn(bool on);
    void setEnableVelIn(bool on);
    void setChannelOut(int value);
    void setPortOut(int value);
    void loadWaveForms();
    bool isModified();
    void setModified(bool);
  
  signals:
    void patternChanged();
    void seqRemove(int ID);
    void dockRename(QString name, int parentDockID);  
    
  public slots:
    void updateChIn(int value);
    void updateEnableNoteIn(bool on);
    void updateEnableVelIn(bool on);
    void updateChannelOut(int value);
    void updatePortOut(int value);
    void updateWaveForm(int);
    void updateRes(int);
    void updateSize(int);
    void updateNoteLength(int val);
    void updateVelocity(int val);
    void updateTranspose(int val);
    void mouseMoved(double, double, int);
    void mousePressed(double, double, int);
    void copyToCustom();
    void moduleDelete();
    void moduleRename();
};
  
#endif