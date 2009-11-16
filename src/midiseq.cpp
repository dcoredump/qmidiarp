/*
 *      midiseq.cpp
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

#include <cmath>
#include <alsa/asoundlib.h>
#include "midiseq.h"


MidiSeq::MidiSeq()
{
    enableNoteIn = true;
    enableVelIn = true;
    chIn = 0;
    queueTempo = 100.0;
    vel = 0;
    transp = 0;
    size = 4;
    res = 4;
    notelength = 74;
    portOut = 0;
    channelOut = 0;
    waveFormIndex = 0;
    isMuted = false;
    int lt = 0;
    int l1 = 0;
    int step = TICKS_PER_QUARTER / res;
    SeqSample seqSample;
    seqSample.value = 60;
    customWave.clear();
    for (l1 = 0; l1 < size * res; l1++) {
            seqSample.tick = lt;
            seqSample.muted = false;
            customWave.append(seqSample);
            lt+=step;
    }
    muteMask.fill(false, size * res);
}

MidiSeq::~MidiSeq(){
}

void MidiSeq::muteSeq(bool on)
{
    isMuted = on;
}

void MidiSeq::muteSeq()
{
    isMuted = not(isMuted);
    emit(toggleMute());
}

bool MidiSeq::isSeq(snd_seq_event_t *evIn) {

    if ((evIn->type != SND_SEQ_EVENT_NOTEON)
            && (evIn->type != SND_SEQ_EVENT_NOTEOFF)
            && (evIn->type != SND_SEQ_EVENT_CONTROLLER)) 
    {
        return(false);
    }
    if ((evIn->data.control.channel < chIn)
            || (evIn->data.control.channel > chIn)) 
    {
        return(false);
    }
    if ((evIn->type == SND_SEQ_EVENT_NOTEON) 
            || (evIn->type == SND_SEQ_EVENT_NOTEOFF)) {
        if (!(enableNoteIn)) {
            return(false);
        }
    }
    return(true);
}

void MidiSeq::getData(QVector<SeqSample> *p_seqData)
{ 
    SeqSample seqSample;
    int lt = 0;
    int step = TICKS_PER_QUARTER / res;

    //res: number of events per beat
    //size: size of waveform in beats
    QVector<SeqSample> seqData;
    seqData.clear();
    
    switch(waveFormIndex) {
        case 0: //custom
            lt = step * customWave.count();
            seqData = customWave;
        break;
        default:
        break;
    }
    seqSample.value = -1;
    seqSample.tick = lt;
    seqData.append(seqSample);    
    *p_seqData = seqData;
}

int MidiSeq::clip(int value, int min, int max, bool *outOfRange)
{
    int tmp = value;

    *outOfRange = false;
    if (tmp > max) {
        tmp = max;
        *outOfRange = true;
    } else if (tmp < min) {
        tmp = min;
        *outOfRange = true;
    }  
    return(tmp);
}

void MidiSeq::updateWaveForm(int val)
{
    waveFormIndex = val;
}

void MidiSeq::updateVelocity(int val)
{
    vel = val;
}

void MidiSeq::updateTranspose(int val)
{
    transp = val;
}

void MidiSeq::updateQueueTempo(int val)
{
    queueTempo = (double)val;
}

void MidiSeq::setCustomWavePoint(double mouseX, double mouseY)
{
    SeqSample seqSample;
    seqSample = customWave.at(mouseX * res * size);
    seqSample.value = mouseY * 48 + 36;
    seqSample.tick = (int)(mouseX * res * size) * TICKS_PER_QUARTER / res;
    customWave.replace(mouseX * res * size, seqSample);
    
}

void MidiSeq::resizeAll()
{
    int lt = 0;
    int l1 = 0;
    int os;
    int step = TICKS_PER_QUARTER / res;
    SeqSample seqSample;
    
    os = customWave.count();
    customWave.resize(size * res);
    muteMask.resize(size * res);
    for (l1 = 0; l1 < customWave.count(); l1++) {
        seqSample = customWave.at(l1 % os);
        seqSample.tick = lt;
        seqSample.muted = muteMask.at(l1);
        customWave.replace(l1, seqSample);
        lt+=step;
    }
}

void MidiSeq::copyToCustom()
{
    QVector<SeqSample> seqData;
    int m;
    seqData.clear();
    getData(&seqData);
    m = seqData.count();
    seqData.remove(m - 1);
    customWave = seqData;
}

void MidiSeq::toggleMutePoint(double mouseX)
{
    SeqSample seqSample;
    bool m;
    m = muteMask.at(mouseX * res * size);
    muteMask.replace(mouseX * res * size, !m);
    seqSample = customWave.at(mouseX * res * size);
    seqSample.muted = !m;
    customWave.replace(mouseX * res * size, seqSample);
}

