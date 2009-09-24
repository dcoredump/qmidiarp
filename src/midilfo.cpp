/*
 *      midilfo.cpp
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

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <QString>
#include <alsa/asoundlib.h>
#include "midilfo.h"


MidiLfo::MidiLfo()
{
	queueTempo = 100.0;
	lfoAmp = 0;
	lfoOffs = 0;
	lfoFreq = 4;
	lfoSize = 1;
	lfoRes = 16;
	lfoCCnumber = 74;
    portOut = 0;
    channelOut = 0;
	waveFormIndex = 0;
}

MidiLfo::~MidiLfo(){
}


void MidiLfo::muteLfo(bool on)
{
    isMuted = on;
}

void MidiLfo::muteLfo()
{
    isMuted = not(isMuted);
	emit(toggleMute());
}


void MidiLfo::getData(QList<LfoSample> *p_lfoData)
{ 
	LfoSample lfoSample;
	int l1 = 0;
	int lt = 0;
	int step = TICKS_PER_QUARTER / lfoRes;
	int lfoval = 0;
	//I think it is better to do the array filling on request here instead
	//of updating on every parameter change. First draft, needs optimization
	//lfoRes: number of events per beat
	//lfoSize: size of waveform in beats
	QList<LfoSample> lfoData;
	lfoData.clear();
	
	switch(waveFormIndex) {
		case 0: //sine
			for (l1 = 0; l1 < lfoSize * lfoRes; l1++) {
				lfoSample.lfoValue = (sin((double)(l1 * 6.28 / lfoRes * lfoFreq / 4) 
				) + 1) * lfoAmp / 2;
				lfoSample.lfoTick = lt;
				lt += step;
				lfoData.append(lfoSample);
			}
			lfoSample.lfoValue = -1;
			lfoSample.lfoTick = lt;
			lfoData.append(lfoSample);
		break;
		case 1: //sawtooth up
			lfoval = 0;
			for (l1 = 0; l1 < lfoSize * lfoRes; l1++) {
				lfoSample.lfoValue = lfoval * lfoAmp / lfoRes / 4;
				lfoSample.lfoTick = lt;
				lt += step;
				lfoval += lfoFreq;
				lfoval %= lfoRes * 4;
				lfoData.append(lfoSample);
			}
			lfoSample.lfoValue = -1;
			lfoSample.lfoTick = lt;
			lfoData.append(lfoSample);
		break;
		case 2: //triangle
			do {
				lfoval = 0;
				do {
					lfoSample.lfoValue = lfoval * lfoAmp / lfoRes / 4;
					lfoSample.lfoTick = lt;
					lfoData.append(lfoSample);
					lt += step;
					lfoval += lfoFreq * 2;
				} while ((lfoval < lfoRes * 4) && (lt < TICKS_PER_QUARTER * lfoSize));
				do {
					lfoSample.lfoValue = lfoval * lfoAmp / lfoRes / 4;
					lfoSample.lfoTick = lt;
					lfoData.append(lfoSample);
					lt += step;
					lfoval -= lfoFreq * 2;
				} while ((lfoval > 0) && (lt < TICKS_PER_QUARTER * lfoSize));
			} while (lt < TICKS_PER_QUARTER * lfoSize);
			
			lfoSample.lfoValue = -1;
			lfoSample.lfoTick = lt;
			lfoData.append(lfoSample);
		break;
		case 3: //sawtooth down
			lfoval = 0;
			for (l1 = 0; l1 < lfoSize * lfoRes; l1++) {
				lfoSample.lfoValue = (lfoRes * 4 - lfoval) * lfoAmp / lfoRes / 4;
				lfoSample.lfoTick = lt;
				lfoData.append(lfoSample);
				lt+=step;
				lfoval += lfoFreq;
				lfoval %= lfoRes * 4;
			}
			lfoSample.lfoValue = -1;
			lfoSample.lfoTick = lt;
			lfoData.append(lfoSample);
		break;
		case 4: //square
			for (l1 = 0; l1 < lfoSize * lfoRes; l1++) {

				lfoSample.lfoValue = lfoAmp * ((l1 * lfoFreq / 2 / lfoRes) % 2 == 0);
				lfoSample.lfoTick = lt;
				lfoData.append(lfoSample);
				lt+=step;
			}
			lfoSample.lfoValue = -1;
			lfoSample.lfoTick = lt;
			lfoData.append(lfoSample);
		break;
		default:
		break;
	}
	
	*p_lfoData = lfoData;
}

void MidiLfo::updateWaveForm(int val)
{
	waveFormIndex = val;
}

int MidiLfo::clip(int value, int min, int max, bool *outOfRange)
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

void MidiLfo::updateFrequency(int val)
{
    lfoFreq = val;
}

void MidiLfo::updateAmplitude(int val)
{
    lfoAmp = val;
}

void MidiLfo::updateOffset(int val)
{
    lfoOffs = val;
}

void MidiLfo::updateQueueTempo(int val)
{
    queueTempo = (double)val;
}

