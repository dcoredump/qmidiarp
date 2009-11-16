/*
 *      lfowidget.h
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

#ifndef LFOWIDGET_H
#define LFOWIDGET_H

#include <QAction>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QString>
#include <QTextStream>
#include <QToolButton>

#include "midilfo.h"
#include "slider.h"
#include "lfoscreen.h"

const int lfoResValues[9] = {1, 2, 4, 8, 16, 32, 64, 96, 192};
const int lfoFreqValues[11] = {1, 2, 3, 4, 8, 12, 16, 20, 24, 28, 32};

class LfoWidget : public QWidget
{
    Q_OBJECT

    // Output channel / port (ALSA Sequencer)
    QSpinBox *channelOut, *portOut, *ccnumberBox;
    QComboBox *waveFormBox, *resBox, *sizeBox, *freqBox;
    QAction *copyToCustomAction;
    QToolButton *copyToCustomButton;
 
    MidiLfo *midiLfo;
    Slider *frequency, *amplitude, *offset;
    QVector<LfoSample> lfoData;
    bool modified;

  public:
    QString lfoName;
    LfoScreen *lfoScreen;
    QStringList waveForms;
    QCheckBox *muteOut;

    LfoWidget(MidiLfo *p_midiLfo, int portCount, QWidget* parent=0);
    ~LfoWidget();
    MidiLfo *getMidiLfo();
    
    void readLfo(QTextStream& arpText);
    void writeLfo(QTextStream& arpText);
    void setChannelOut(int value);
    void setPortOut(int value);
    void loadWaveForms();
    bool isModified();
    void setModified(bool);
  
  signals:
    void patternChanged();
    
  public slots:
    void updateChannelOut(int value);
    void updatePortOut(int value);
    void updateWaveForm(int);
    void updateRes(int);
    void updateSize(int);
    void updateCcnumber(int val);
    void updateFreq(int val);
    void updateAmp(int val);
    void updateOffs(int val);
    void mouseMoved(double, double, int);
    void mousePressed(double, double, int);
    void copyToCustom();
};
  
#endif
