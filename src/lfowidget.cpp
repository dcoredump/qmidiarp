/*!
 * @file lfowidget.cpp
 * @brief Implements the LfoWidget GUI class.
 *
 * @section LICENSE
 *
 *      Copyright 2009 - 2016 <qmidiarp-devel@lists.sourceforge.net>
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
#include <cstdio>
#include <QBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>

#include "lfowidget.h"

#include "pixmaps/lfowsine.xpm"
#include "pixmaps/lfowsawup.xpm"
#include "pixmaps/lfowsawdn.xpm"
#include "pixmaps/lfowtri.xpm"
#include "pixmaps/lfowsquare.xpm"
#include "pixmaps/lfowcustm.xpm"
#include "pixmaps/lfowflip.xpm"
#include "pixmaps/seqrecord.xpm"
#include "config.h"


#ifdef APPBUILD
LfoWidget::LfoWidget(MidiLfo *p_midiWorker, GlobStore *p_globStore,
    int portCount, bool compactStyle,
    bool mutedAdd, bool inOutVisible, const QString& p_name):
    InOutBox(p_globStore, portCount, compactStyle, inOutVisible, p_name),
    midiWorker(p_midiWorker),
    modified(false)
{
#else
LfoWidget::LfoWidget(
    bool compactStyle,
    bool mutedAdd, bool inOutVisible):
    InOutBox(compactStyle, inOutVisible, "LFO:"),
    midiWorker(NULL),
    modified(false)
{
#endif

    connect(enableNoteOff, SIGNAL(toggled(bool)), this, 
            SLOT(updateEnableNoteOff(bool)));
    connect(ccnumberInBox, SIGNAL(valueChanged(int)), this,
            SLOT(updateCcnumberIn(int)));
    connect(ccnumberBox, SIGNAL(valueChanged(int)), this,
            SLOT(updateCcnumber(int)));
    connect(enableRestartByKbd, SIGNAL(toggled(bool)), this, 
            SLOT(updateEnableRestartByKbd(bool)));
    connect(enableTrigByKbd, SIGNAL(toggled(bool)), this, 
            SLOT(updateEnableTrigByKbd(bool)));
    connect(enableTrigLegato, SIGNAL(toggled(bool)), this, 
            SLOT(updateTrigLegato(bool)));
    connect(chIn, SIGNAL(activated(int)), this, 
            SLOT(updateChIn(int)));
    connect(channelOut, SIGNAL(activated(int)), this,
            SLOT(updateChannelOut(int)));
    connect(muteOutAction, SIGNAL(toggled(bool)), this, 
            SLOT(setMuted(bool)));
    connect(deferChangesAction, SIGNAL(toggled(bool)), this, 
            SLOT(updateDeferChanges(bool)));
#ifdef APPBUILD
    connect(portOut, SIGNAL(activated(int)), this, 
            SLOT(updatePortOut(int)));
#endif

    // group box for wave setup
    QGroupBox *waveBox = new QGroupBox(tr("Wave"));

    screen = new LfoScreen(this);
    screen->setToolTip(
        tr("Right button to mute points\nLeft button to draw custom wave\nWheel to change offset"));
    screen->setMinimumHeight(80);

    connect(screen, SIGNAL(mouseEvent(double, double, int, int)), this,
            SLOT(mouseEvent(double, double, int, int)));
    connect(screen, SIGNAL(mouseWheel(int)), this,
            SLOT(mouseWheel(int)));

    cursor = new Cursor('L');

    QLabel *waveFormBoxLabel = new QLabel(tr("&Waveform"), waveBox);
    waveFormBox = new QComboBox(waveBox);
    waveFormBoxLabel->setBuddy(waveFormBox);
    //loadWaveForms();
    waveFormBox->addItem(QPixmap(lfowsine_xpm),"");
    waveFormBox->addItem(QPixmap(lfowsawup_xpm),"");
    waveFormBox->addItem(QPixmap(lfowtri_xpm),"");
    waveFormBox->addItem(QPixmap(lfowsawdn_xpm),"");
    waveFormBox->addItem(QPixmap(lfowsquare_xpm),"");
    waveFormBox->addItem(QPixmap(lfowcustm_xpm),"");
    waveFormBox->setCurrentIndex(0);
    waveFormBoxIndex = 0;
    waveFormBox->setToolTip(tr("Waveform Basis"));

    connect(waveFormBox, SIGNAL(activated(int)), this,
            SLOT(updateWaveForm(int)));
#ifdef APPBUILD
    midiControl->addMidiLearnMenu("WaveForm", waveFormBox, 3);
#endif

    QLabel *freqBoxLabel = new QLabel(tr("&Frequency"),
            waveBox);
    freqBox = new QComboBox;
    freqBoxLabel->setBuddy(freqBox);
    QStringList names;
    names << "1/32" << "1/16" << "1/8" << "1/4"
        << "1/2" << "3/4" << "1" << "2" << "3"
        << "4" << "5" << "6" << "7" << "8";
    freqBox->insertItems(0, names);
    freqBox->setCurrentIndex(4);
    freqBoxIndex = 4;
    freqBox->setToolTip(
            tr("Frequency (cycles/beat): Number of wave cycles produced every beat"));
    freqBox->setMinimumContentsLength(3);
    connect(freqBox, SIGNAL(activated(int)), this,
            SLOT(updateFreq(int)));
#ifdef APPBUILD
    midiControl->addMidiLearnMenu("Frequency", freqBox, 4);
#endif
    QLabel *resBoxLabel = new QLabel(tr("&Resolution"),
            waveBox);
    resBox = new QComboBox;
    resBoxLabel->setBuddy(resBox);
    names.clear();
    names << "1" << "2" << "4" << "8" << "16" << "32" << "64" << "96" << "192";
    resBox->insertItems(0, names);
    resBox->setCurrentIndex(2);
    resBoxIndex = 2;
    resBox->setToolTip(
            tr("Resolution (events/beat): Number of events produced every beat"));
    resBox->setMinimumContentsLength(3);
    connect(resBox, SIGNAL(activated(int)), this,
            SLOT(updateRes(int)));
#ifdef APPBUILD
    midiControl->addMidiLearnMenu("Resolution", resBox, 6);
#endif
    QLabel *sizeBoxLabel = new QLabel(tr("&Length"));
    sizeBox = new QComboBox;
    sizeBoxLabel->setBuddy(sizeBox);
    names.clear();
    names << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8"
            << "12" << "16" << "24" << "32" ;
    sizeBox->insertItems(0, names);
    sizeBox->setCurrentIndex(3);
    sizeBoxIndex = 3;
    sizeBox->setToolTip(tr("Length of LFO wave in beats"));
    sizeBox->setMinimumContentsLength(3);
    connect(sizeBox, SIGNAL(activated(int)), this,
            SLOT(updateSize(int)));
#ifdef APPBUILD
    midiControl->addMidiLearnMenu("Size", sizeBox, 7);
#endif
    loopBox = new QComboBox;
    names.clear();
    names << "->_>" << " <_<-" << "->_<" << " >_<-" << "->_|" << " |_<-" << "RANDM";
    loopBox->insertItems(0, names);
    loopBox->setCurrentIndex(0);
    loopBox->setToolTip(tr("Loop, bounce or play once going forward or backward"));
    loopBox->setMinimumContentsLength(5);
    connect(loopBox, SIGNAL(activated(int)), this,
            SLOT(updateLoop(int)));
#ifdef APPBUILD
    midiControl->addMidiLearnMenu("LoopMode", loopBox, 8);
#endif

    flipWaveVerticalAction = new QAction(QPixmap(lfowflip_xpm),tr("&Flip"), this);
    flipWaveVerticalAction->setToolTip(tr("Do a vertical flip of the wave about its mid value"));
    connect(flipWaveVerticalAction, SIGNAL(triggered(bool)), this, SLOT(updateFlipWaveVertical()));

    QToolButton *flipWaveVerticalButton = new QToolButton;
    flipWaveVerticalButton->setDefaultAction(flipWaveVerticalAction);
    flipWaveVerticalButton->setFixedSize(20, 20);

    QLabel *recordButtonLabel = new QLabel(tr("Re&cord"));
    recordAction = new QAction(QPixmap(seqrecord_xpm), tr("Re&cord"), this);
    recordAction->setToolTip(tr("Record incoming controller"));
    recordAction->setCheckable(true);
    QToolButton *recordButton = new QToolButton;
    recordButton->setDefaultAction(recordAction);
    recordButtonLabel->setBuddy(recordButton);
    connect(recordAction, SIGNAL(toggled(bool)), this, SLOT(setRecord(bool)));
#ifdef APPBUILD
    midiControl->addMidiLearnMenu("RecordToggle", recordButton, 5);
#endif
    amplitude = new Slider(0, 127, 1, 8, 64, Qt::Horizontal,
            tr("&Amplitude"), this);
    connect(amplitude, SIGNAL(valueChanged(int)), this,
            SLOT(updateAmp(int)));
#ifdef APPBUILD
    midiControl->addMidiLearnMenu("Amplitude", amplitude, 1);
#endif

    offset = new Slider(0, 127, 1, 8, 0, Qt::Horizontal,
            tr("&Offset"), this);
    connect(offset, SIGNAL(valueChanged(int)), this,
            SLOT(updateOffs(int)));
#ifdef APPBUILD
    midiControl->addMidiLearnMenu("Offset", offset, 2);
#endif
    QVBoxLayout* sliderLayout = new QVBoxLayout;
    sliderLayout->addWidget(amplitude);
    sliderLayout->addWidget(offset);
    sliderLayout->addStretch();
    if (compactStyle) {
        sliderLayout->setSpacing(1);
        sliderLayout->setMargin(2);
    }

    QGridLayout *paramBoxLayout = new QGridLayout;
    paramBoxLayout->addWidget(loopBox, 0, 0, 1, 2);
    paramBoxLayout->addWidget(muteOut, 1, 0, 1, 1);
    paramBoxLayout->addWidget(deferChangesButton, 1, 1, 1, 2);
    paramBoxLayout->addWidget(recordButtonLabel, 2, 0);
    paramBoxLayout->addWidget(recordButton, 2, 1);
    paramBoxLayout->addWidget(waveFormBoxLabel, 0, 2);
    paramBoxLayout->addWidget(waveFormBox, 0, 3);
    paramBoxLayout->addWidget(freqBoxLabel, 1, 2);
    paramBoxLayout->addWidget(freqBox, 1, 3);
    paramBoxLayout->addWidget(resBoxLabel, 0, 4);
    paramBoxLayout->addWidget(resBox, 0, 5);
    paramBoxLayout->addWidget(sizeBoxLabel, 1, 4);
    paramBoxLayout->addWidget(sizeBox, 1, 5);
    paramBoxLayout->addWidget(flipWaveVerticalButton, 0, 6);
    paramBoxLayout->setColumnStretch(7, 7);

    if (compactStyle) {
        paramBoxLayout->setSpacing(1);
        paramBoxLayout->setMargin(2);
    }

    QGridLayout* waveBoxLayout = new QGridLayout;
    waveBoxLayout->addWidget(screen, 0, 0);
    waveBoxLayout->addWidget(cursor, 1, 0);
    waveBoxLayout->addLayout(paramBoxLayout, 2, 0);
    waveBoxLayout->addLayout(sliderLayout, 3, 0);
    if (compactStyle) {
        waveBoxLayout->setSpacing(0);
        waveBoxLayout->setMargin(2);
    }
    waveBox->setLayout(waveBoxLayout);


    QHBoxLayout *widgetLayout = new QHBoxLayout;
    widgetLayout->addWidget(waveBox, 1);
    widgetLayout->addWidget(hideInOutBoxButton, 0);
    widgetLayout->addWidget(inOutBoxWidget, 0);

    muteOutAction->setChecked(mutedAdd);

    setLayout(widgetLayout);
    updateAmp(64);
    dataChanged = false;
    needsGUIUpdate = false;
}

#ifdef APPBUILD
MidiLfo *LfoWidget::getMidiWorker()
{
    return (midiWorker);
}

void LfoWidget::writeData(QXmlStreamWriter& xml)
{
    QByteArray tempArray;
    int l1;

        writeCommonData(xml);
    
        xml.writeStartElement("waveParams");
            xml.writeTextElement("loopmode", QString::number(
                loopBox->currentIndex()));
            xml.writeTextElement("waveform", QString::number(
                waveFormBox->currentIndex()));
            xml.writeTextElement("frequency", QString::number(
                freqBox->currentIndex()));
            xml.writeTextElement("resolution", QString::number(
                resBox->currentIndex()));
            xml.writeTextElement("size", QString::number(
                sizeBox->currentIndex()));
            xml.writeTextElement("amplitude", QString::number(
                midiWorker->amp));
            xml.writeTextElement("offset", QString::number(
                midiWorker->offs));
        xml.writeEndElement();

        tempArray.clear();
        l1 = 0;
        while (l1 < midiWorker->maxNPoints) {
            tempArray.append(midiWorker->muteMask.at(l1));
            l1++;
        }
        xml.writeStartElement("muteMask");
            xml.writeTextElement("data", tempArray.toHex());
        xml.writeEndElement();

        tempArray.clear();
        l1 = 0;
        while (l1 < midiWorker->maxNPoints) {
            tempArray.append(midiWorker->customWave.at(l1).value);
            l1++;
        }
        xml.writeStartElement("customWave");
            xml.writeTextElement("data", tempArray.toHex());
        xml.writeEndElement();

    xml.writeEndElement();
}

void LfoWidget::readData(QXmlStreamReader& xml)
{
    int tmp;
    int wvtmp = 0;
    Sample sample;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isEndElement())
            break;
        
        readCommonData(xml);

        if (xml.isStartElement() && (xml.name() == "waveParams")) {
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement())
                    break;
                if (xml.name() == "loopmode") {
                    tmp = xml.readElementText().toInt();
                    loopBox->setCurrentIndex(tmp);
                    updateLoop(tmp);
                }
                else if (xml.name() == "waveform")
                    wvtmp = xml.readElementText().toInt();
                else if (xml.name() == "frequency") {
                    tmp = xml.readElementText().toInt();
                    freqBox->setCurrentIndex(tmp);
                    updateFreq(tmp);
                }
                else if (xml.name() == "resolution") {
                    tmp = xml.readElementText().toInt();
                    resBox->setCurrentIndex(tmp);
                    updateRes(tmp);
                }
                else if (xml.name() == "size") {
                    tmp = xml.readElementText().toInt();
                    sizeBox->setCurrentIndex(tmp);
                    updateSize(tmp);
                }
                else if (xml.name() == "amplitude")
                    amplitude->setValue(xml.readElementText().toInt());
                else if (xml.name() == "offset")
                    offset->setValue(xml.readElementText().toInt());
                else skipXmlElement(xml);
            }
        }
        else if (xml.isStartElement() && (xml.name() == "muteMask")) {
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement())
                    break;
                if (xml.isStartElement() && (xml.name() == "data")) {
                    QByteArray tmpArray =
                            QByteArray::fromHex(xml.readElementText().toLatin1());
                    for (int l1 = 0; l1 < tmpArray.count(); l1++) {
                        midiWorker->muteMask.replace(l1, tmpArray.at(l1));
                    }
                    midiWorker->maxNPoints = tmpArray.count();
                }
                else skipXmlElement(xml);
            }
        }
        else if (xml.isStartElement() && (xml.name() == "customWave")) {
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement())
                    break;
                if (xml.isStartElement() && (xml.name() == "data")) {
                    QByteArray tmpArray =
                            QByteArray::fromHex(xml.readElementText().toLatin1());
                    int step = TPQN / midiWorker->res;
                    int lt = 0;
                    for (int l1 = 0; l1 < tmpArray.count(); l1++) {
                        sample.value = tmpArray.at(l1);
                        sample.tick = lt;
                        sample.muted = midiWorker->muteMask.at(l1);
                        midiWorker->customWave.replace(l1, sample);
                        lt+=step;
                    }
                }
                else skipXmlElement(xml);
            }
        }
        else skipXmlElement(xml);
    }
    
    updateChIn(chIn->currentIndex());
    updateChannelOut(channelOut->currentIndex());
    updatePortOut(portOut->currentIndex());
    waveFormBox->setCurrentIndex(wvtmp);
    updateWaveForm(wvtmp);
    modified = false;
}
#endif

void LfoWidget::loadWaveForms()
{
    waveForms << tr("Sine") << tr("Saw up") << tr("Triangle")
        << tr("Saw down") << tr("Square") << tr("Custom");
}

void LfoWidget::updateCcnumber(int val)
{
    if (midiWorker)
        midiWorker->ccnumber = val;
    modified = true;
}

void LfoWidget::updateIndexIn(int value)
{
    if (indexIn[0] == sender()) {
        if (midiWorker) midiWorker->indexIn[0] = value;
    } else {
        if (midiWorker) midiWorker->indexIn[1] = value;
    }
    checkIfInputFilterSet();
    modified = true;
}

void LfoWidget::updateRangeIn(int value)
{
    if (rangeIn[0] == sender()) {
        if (midiWorker) midiWorker->rangeIn[0] = value;
    } else {
        if (midiWorker) midiWorker->rangeIn[1] = value;
    }
    checkIfInputFilterSet();
    modified = true;
}

void LfoWidget::updateChIn(int val)
{
    if (midiWorker) midiWorker->chIn = val;
    modified = true;
}

void LfoWidget::updateCcnumberIn(int val)
{
    if (midiWorker) midiWorker->ccnumberIn = val;
    modified = true;
}
void LfoWidget::updateEnableNoteOff(bool on)
{
    if (midiWorker) midiWorker->enableNoteOff = on;
    modified = true;
}

void LfoWidget::updateEnableRestartByKbd(bool on)
{
    if (midiWorker) midiWorker->restartByKbd = on;
    modified = true;
}

void LfoWidget::updateEnableTrigByKbd(bool on)
{
    if (midiWorker) midiWorker->trigByKbd = on;
    modified = true;
}

void LfoWidget::updateTrigLegato(bool on)
{
    if (midiWorker) midiWorker->trigLegato = on;
    modified = true;
}

void LfoWidget::updateWaveForm(int val)
{
    if (val > 5) return;
    waveFormBoxIndex = val;
    if (midiWorker) midiWorker->updateWaveForm(val);
    if (midiWorker) midiWorker->getData(&data);
    if (midiWorker) screen->updateData(data);
    bool isCustom = (val == 5);
    if (isCustom && midiWorker) midiWorker->newCustomOffset();
    amplitude->setDisabled(isCustom);
    freqBox->setDisabled(isCustom);
    modified = true;

}

void LfoWidget::updateScreen(int val)
{
    if (!midiWorker) return;
    if (!midiWorker->isRecording)
        cursor->updatePosition(val);
}

void LfoWidget::updateFreq(int val)
{
    if (val > 13) return;
    freqBoxIndex = val;
    if (midiWorker) midiWorker->updateFrequency(lfoFreqValues[val]);
    if (midiWorker) midiWorker->getData(&data);
    if (midiWorker) screen->updateData(data);
    modified = true;
}

void LfoWidget::updateRes(int val)
{
    if (val > 8) return;
    resBoxIndex = val;
    if (midiWorker) midiWorker->updateResolution(lfoResValues[val]);
    if (midiWorker) midiWorker->getData(&data);
    if (midiWorker) screen->updateData(data);
    if (midiWorker && (waveFormBoxIndex == 5)) midiWorker->newCustomOffset();
    modified = true;
}

void LfoWidget::updateSize(int val)
{
    if (val > 11) return;
    sizeBoxIndex = val;
    if (!midiWorker) return;
    midiWorker->updateSize(sizeBox->currentText().toInt());
    midiWorker->getData(&data);
    screen->updateData(data);
    if (waveFormBoxIndex == 5) midiWorker->newCustomOffset();
    modified = true;
}

void LfoWidget::updateLoop(int val)
{
    if (val > 6) return;
    if (midiWorker) midiWorker->updateLoop(val);
    modified = true;
}

void LfoWidget::updateAmp(int val)
{
    if (!midiWorker) return;
    midiWorker->updateAmplitude(val);
    midiWorker->getData(&data);
    screen->updateData(data);
    modified = true;
}

void LfoWidget::updateOffs(int val)
{
    if (!midiWorker) return;
    midiWorker->updateOffset(val);
    midiWorker->getData(&data);
    screen->updateData(data);
    modified = true;
}

void LfoWidget::copyToCustom()
{
    if (midiWorker) midiWorker->copyToCustom();
    waveFormBox->setCurrentIndex(5);
    updateWaveForm(5);
    modified = true;
}

void LfoWidget::updateFlipWaveVertical()
{
    if (midiWorker) {
        if (waveFormBox->currentIndex() != 5) copyToCustom();
        midiWorker->flipWaveVertical();
        midiWorker->getData(&data);
        screen->updateData(data);
    }
    modified = true;
}

void LfoWidget::mouseEvent(double mouseX, double mouseY, int buttons, int pressed)
{
    if (!midiWorker) emit mouseSig(mouseX, mouseY, buttons, pressed);
    else midiWorker->mouseEvent(mouseX, mouseY, buttons, pressed);

    if ((buttons == 1) && (waveFormBox->currentIndex() != 5)) {
        waveFormBox->setCurrentIndex(5);
        updateWaveForm(5);
    }
    modified = true;
}

void LfoWidget::mouseWheel(int step)
{
    int cv;
    cv = offset->value() + step;
    if ((cv < 127) && (cv > 0))
    offset->setValue(cv + step);
}

void LfoWidget::setInOutBoxVisible(bool on)
{
    setVisible(on);
    modified=true;
}

void LfoWidget::setMuted(bool on)
{
    if (!midiWorker) return;
    midiWorker->setMuted(on);
    screen->setMuted(midiWorker->isMuted);
#ifdef APPBUILD
    parStore->ndc->setMuted(midiWorker->isMuted);
#endif
    modified = true;
}

void LfoWidget::updateDeferChanges(bool on)
{
    if (midiWorker) midiWorker->updateDeferChanges(on);
    modified = true;
}

void LfoWidget::setRecord(bool on)
{
    if (midiWorker) midiWorker->setRecordMode(on);
    screen->setRecordMode(on);
}

void LfoWidget::updatePortOut(int value)
{
    if (midiWorker) midiWorker->portOut = value;
    modified = true;
}

void LfoWidget::updateChannelOut(int value)
{
    if (midiWorker) midiWorker->channelOut = value;
    modified = true;
}

#ifdef APPBUILD
QVector<Sample> LfoWidget::getCustomWave()
{
    return midiWorker->customWave;
}

bool LfoWidget::isModified()
{
    return (modified || midiControl->isModified());
}

void LfoWidget::setModified(bool m)
{
    modified = m;
    midiControl->setModified(m);
}

void LfoWidget::doStoreParams(int ix, bool empty)
{
    parStore->temp.empty = empty;
    parStore->temp.muteOut = muteOut->isChecked();
    parStore->temp.chIn = chIn->currentIndex();
    parStore->temp.ccnumberIn = ccnumberInBox->value();
    parStore->temp.ccnumber = ccnumberBox->value();
    parStore->temp.channelOut = channelOut->currentIndex();
    parStore->temp.portOut = portOut->currentIndex();
    parStore->temp.indexIn0 = indexIn[0]->value();
    parStore->temp.indexIn1 = indexIn[1]->value();
    parStore->temp.rangeIn0 = rangeIn[0]->value();
    parStore->temp.rangeIn1 = rangeIn[1]->value();
    parStore->temp.res = resBox->currentIndex();
    parStore->temp.size = sizeBox->currentIndex();
    parStore->temp.loopMode = loopBox->currentIndex();
    parStore->temp.freq = freqBox->currentIndex();
    parStore->temp.ampl = amplitude->value();
    parStore->temp.offs = offset->value();
    parStore->temp.waveForm = waveFormBox->currentIndex();

    if (midiWorker) parStore->temp.wave = getCustomWave().mid(0, midiWorker->maxNPoints);
    if (midiWorker) parStore->temp.muteMask = midiWorker->muteMask.mid(0, midiWorker->maxNPoints);

    parStore->tempToList(ix);
}

void LfoWidget::doRestoreParams(int ix)
{
    midiWorker->applyPendingParChanges();
    if (parStore->list.at(ix).empty) return;
    for (int l1 = 0; l1 < parStore->list.at(ix).wave.count(); l1++) {
        midiWorker->customWave.replace(l1, parStore->list.at(ix).wave.at(l1));
        midiWorker->muteMask.replace(l1, parStore->list.at(ix).muteMask.at(l1));
    }
    sizeBox->setCurrentIndex(parStore->list.at(ix).size);
    midiWorker->updateSize(sizeBox->currentText().toInt());

    midiWorker->updateResolution(lfoResValues[parStore->list.at(ix).res]);
    midiWorker->updateWaveForm(parStore->list.at(ix).waveForm);
    midiWorker->updateFrequency(lfoFreqValues[parStore->list.at(ix).freq]);
    freqBox->setCurrentIndex(parStore->list.at(ix).freq);
    resBox->setCurrentIndex(parStore->list.at(ix).res);
    waveFormBox->setCurrentIndex(parStore->list.at(ix).waveForm);
    loopBox->setCurrentIndex(parStore->list.at(ix).loopMode);
    updateLoop(parStore->list.at(ix).loopMode);
    updateWaveForm(parStore->list.at(ix).waveForm);
    if (!parStore->onlyPatternList.at(ix)) {
        //muteOut->setChecked(parStore->list.at(ix).muteOut);
        offset->setValue(parStore->list.at(ix).offs);
        indexIn[0]->setValue(parStore->list.at(ix).indexIn0);
        indexIn[1]->setValue(parStore->list.at(ix).indexIn1);
        rangeIn[0]->setValue(parStore->list.at(ix).rangeIn0);
        rangeIn[1]->setValue(parStore->list.at(ix).rangeIn1);
        chIn->setCurrentIndex(parStore->list.at(ix).chIn);
        updateChIn(parStore->list.at(ix).chIn);
        ccnumberInBox->setValue(parStore->list.at(ix).ccnumberIn);
        ccnumberBox->setValue(parStore->list.at(ix).ccnumber);
        channelOut->setCurrentIndex(parStore->list.at(ix).channelOut);
        updateChannelOut(parStore->list.at(ix).channelOut);
        setPortOut(parStore->list.at(ix).portOut);
        updatePortOut(parStore->list.at(ix).portOut);
        amplitude->setValue(parStore->list.at(ix).ampl);
    }
    midiWorker->setFramePtr(0);
}

void LfoWidget::copyParamsFrom(LfoWidget *fromWidget)
{
    int tmp;

    enableNoteOff->setChecked(fromWidget->enableNoteOff->isChecked());
    enableRestartByKbd->setChecked(fromWidget->enableRestartByKbd->isChecked());
    enableTrigByKbd->setChecked(fromWidget->enableTrigByKbd->isChecked());
    enableTrigLegato->setChecked(fromWidget->enableTrigLegato->isChecked());

    for (int l1 = 0; l1 < 1; l1++) {
        tmp = fromWidget->indexIn[l1]->value();
        indexIn[l1]->setValue(tmp);
    }
    for (int l1 = 0; l1 < 1; l1++) {
        tmp = fromWidget->rangeIn[l1]->value();
        rangeIn[l1]->setValue(tmp);
    }

    tmp = fromWidget->chIn->currentIndex();
    chIn->setCurrentIndex(tmp);
    updateChIn(tmp);
    tmp = fromWidget->channelOut->currentIndex();
    channelOut->setCurrentIndex(tmp);
    updateChannelOut(tmp);
    tmp = fromWidget->portOut->currentIndex();
    portOut->setCurrentIndex(tmp);
    updatePortOut(tmp);

    tmp = fromWidget->ccnumberInBox->value();
    ccnumberInBox->setValue(tmp);
    tmp = fromWidget->ccnumberBox->value();
    ccnumberBox->setValue(tmp);

    tmp = fromWidget->resBox->currentIndex();
    resBox->setCurrentIndex(tmp);
    updateRes(tmp);
    tmp = fromWidget->sizeBox->currentIndex();
    sizeBox->setCurrentIndex(tmp);
    updateSize(tmp);
    tmp = fromWidget->loopBox->currentIndex();
    loopBox->setCurrentIndex(tmp);
    updateLoop(tmp);
    tmp = fromWidget->freqBox->currentIndex();
    freqBox->setCurrentIndex(tmp);
    updateFreq(tmp);

    amplitude->setValue(fromWidget->amplitude->value());
    offset->setValue(fromWidget->offset->value());

    for (int l1 = 0; l1 < fromWidget->getMidiWorker()->maxNPoints; l1++) {
        midiWorker->customWave.replace(l1, fromWidget->getCustomWave().at(l1));
        midiWorker->muteMask.replace(l1, midiWorker->customWave.at(l1).muted);
    }
    midiControl->setCcList(fromWidget->midiControl->ccList);
    muteOutAction->setChecked(true);

    tmp = fromWidget->waveFormBox->currentIndex();
    waveFormBox->setCurrentIndex(tmp);
    updateWaveForm(tmp);
}

void LfoWidget::handleController(int ccnumber, int channel, int value)
{
    bool m;
    int min, max, sval;
    QVector<MidiCC> cclist= midiControl->ccList;

    for (int l2 = 0; l2 < cclist.count(); l2++) {
        min = cclist.at(l2).min;
        max = cclist.at(l2).max;
        if ((ccnumber == cclist.at(l2).ccnumber) &&
            (channel == cclist.at(l2).channel)) {
            switch (cclist.at(l2).ID) {
                case 0: if (min == max) {
                            if (value == max) {
                                m = midiWorker->isMuted;
                                midiWorker->setMuted(!m);
                            }
                        }
                        else {
                            if (value == max) {
                                midiWorker->setMuted(false);
                            }
                            if (value == min) {
                                midiWorker->setMuted(true);
                            }
                        }
                break;

                case 1:
                        sval = min + ((double)value * (max - min) / 127);
                        midiWorker->updateAmplitude(sval);
                break;

                case 2:
                        sval = min + ((double)value * (max - min) / 127);
                        midiWorker->updateOffset(sval);
                break;
                case 3:
                        sval = min + ((double)value * (max - min) / 127);
                        if (sval < 6) waveFormBoxIndex = sval;
                break;
                case 4:
                        sval = min + ((double)value * (max - min) / 127);
                        if (sval < 14) freqBoxIndex = sval;
                break;
                case 5: if (min == max) {
                            if (value == max) {
                                m = midiWorker->recordMode;
                                midiWorker->setRecordMode(!m);
                                return;
                            }
                        }
                        else {
                            if (value == max) {
                                midiWorker->setRecordMode(true);
                            }
                            if (value == min) {
                                midiWorker->setRecordMode(false);
                            }
                        }
                break;
                case 6:
                        sval = min + ((double)value * (max - min) / 127);
                        if (sval < 9) resBoxIndex = sval;
                break;
                case 7:
                        sval = min + ((double)value * (max - min) / 127);
                        if (sval < 12) sizeBoxIndex = sval;
                break;
                case 8:
                        sval = min + ((double)value * (max - min) / 127);
                        if (sval < 6) midiWorker->curLoopMode = sval;
                break;
                case 9:
                        sval = min + ((double)value * (max - min) / 127);
                        if ((sval < parStore->list.count())
                                && (sval != parStore->activeStore)
                                && (sval != parStore->currentRequest)) {
                            parStore->requestDispState(sval, 2);
                            parStore->restoreRequest = sval;
                            parStore->restoreRunOnce = (parStore->jumpToList.at(sval) > -2);
                        }
                        else return;
                break;


                default:
                break;
            }
            needsGUIUpdate = true;
        }
    }
}

void LfoWidget::updateDisplay()
{
    QVector<Sample> data;

    parStore->updateDisplay(getFramePtr()/midiWorker->frameSize, midiWorker->reverse);

    if (midiWorker->dataChanged) {
        midiWorker->getData(&data);
        screen->updateData(data);
        cursor->updateNumbers(midiWorker->res, midiWorker->size);
        offset->setValue(midiWorker->offs);
        midiWorker->dataChanged = false;
    }
    screen->updateDraw();
    cursor->updateDraw();
    midiControl->update();

    if (!(needsGUIUpdate || midiWorker->needsGUIUpdate)) return;

    muteOutAction->setChecked(midiWorker->isMuted);
    screen->setMuted(midiWorker->isMuted);
    parStore->ndc->setMuted(midiWorker->isMuted);
    recordAction->setChecked(midiWorker->recordMode);
    screen->setRecordMode(midiWorker->recordMode);
    resBox->setCurrentIndex(resBoxIndex);
    updateRes(resBoxIndex);
    sizeBox->setCurrentIndex(sizeBoxIndex);
    updateSize(sizeBoxIndex);
    freqBox->setCurrentIndex(freqBoxIndex);
    updateFreq(freqBoxIndex);
    loopBox->setCurrentIndex(midiWorker->curLoopMode);
    amplitude->setValue(midiWorker->amp);
    offset->setValue(midiWorker->offs);
    if (waveFormBoxIndex != waveFormBox->currentIndex()) {
        waveFormBox->setCurrentIndex(waveFormBoxIndex);
        updateWaveForm(waveFormBoxIndex);
    }
    needsGUIUpdate = false;
    midiWorker->needsGUIUpdate = false;
}

#endif
