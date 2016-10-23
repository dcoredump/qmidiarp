/*!
 * @file midiworker.cpp
 * @brief Implements the MIDI worker module base class.
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
 *
 */
#include <cmath>
#include "midiworker.h"


MidiWorker::MidiWorker()
{
    enableNoteIn = true;
    enableNoteOff = false;
    enableVelIn = true;
    trigByKbd = false;
    gotKbdTrig = false;
    restartByKbd = false;
    trigLegato = false;
    for (int l1 = 0; l1 < 2; l1++) {
        rangeIn[l1] = (l1) ? 127 : 0;
        indexIn[l1] = (l1) ? 127 : 0;
    }
    
    queueTempo = 100.0;
    ccnumber = 74;
    portOut = 0;
    channelOut = 0;
    chIn = 0;
    ccnumberIn = 74;
    isMuted = false;
    isMutedDefer = false;
    reverse = false;
    pingpong = false;
    backward = false;
    reflect = false;
    seqFinished = false;
    restartFlag = false;
    triggerMode = 0;
    enableLoop = true;
    curLoopMode = 0;
    noteCount = 0;
    nextTick = 0;
    grooveTick = 0;
    newGrooveTick = 0;
    grooveVelocity = 0;
    grooveLength = 0;
    dataChanged = false;
    needsGUIUpdate = false;

}

void MidiWorker::setMuted(bool on)
{
    isMuted = on;
}

int MidiWorker::clip(int value, int min, int max, bool *outOfRange)
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

void MidiWorker::updateQueueTempo(int val)
{
    queueTempo = (double)val;
}

void MidiWorker::newGrooveValues(int p_grooveTick, int p_grooveVelocity,
        int p_grooveLength)
{
    // grooveTick is only updated on pair steps to keep quantization
    // newGrooveTick stores the GUI value temporarily
    newGrooveTick = p_grooveTick;
    grooveVelocity = p_grooveVelocity;
    grooveLength = p_grooveLength;
}

void MidiWorker::updateTriggerMode(int val)
{
    triggerMode = val;
    trigByKbd = ((val == 2) || (val == 4));
    restartByKbd = (val > 0);
    trigLegato = (val > 2);
}