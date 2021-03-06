/*!
 * @file midilfo.cpp
 * @brief Implements the MidiLfo MIDI worker class for the LFO Module.
 *
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
#include "midilfo.h"


MidiLfo::MidiLfo()
{
    amp = 64;
    offs = 0;
    freq = 8;
    size = 4;
    res = 4;
    maxNPoints = 16;
    old_res = 0;
    waveFormIndex = 0;
    recordMode = false;
    isRecording = false;
    recValue = 0;
    int l1 = 0;
    int lt = 0;
    int step = TPQN / res;
    cwmin = 0;

    customWave.resize(8192);
    Sample sample;
    sample.value = 63;
    sample.tick = 0;
    for (l1 = 0; l1 < size * res; l1++) {
        sample.tick = lt;
        sample.muted = false;
        customWave.replace(l1, sample);
        lt+=step;
    }
    muteMask.fill(false, 8192);
    data.clear();
    frame.resize(32);
    frame.fill(sample);
    updateWaveForm(waveFormIndex);
    getData(&data);
    lastMouseLoc = 0;
    lastMouseY = 0;
    frameSize = 1;

    lastMute = false;
}

void MidiLfo::getNextFrame(int tick)
{
    //this function is called by engine and returns one sample
    //if res <= LFO_FRAMELIMIT. If res > LFO_FRAMELIMIT, a frame is output
    //The FRAMELIMIT avoids excessive cursor updating

    if (framePtr >= data.size()) return;
    
    Sample sample;
    const int step = TPQN / res;
    const int npoints = size * res;
    int lt, l1;
    int framelimit;
    int index;

    gotKbdTrig = false;

    if (isRecording) framelimit = 32; else framelimit = LFO_FRAMELIMIT;
    frameSize = res / framelimit;
    if (!frameSize) frameSize = 1;

    if (restartFlag) setFramePtr(0);
    if (!framePtr) grooveTick = newGrooveTick;

    l1 = 0;
    lt = nextTick;
    do {
        if (reverse) {
            index = (frameSize - 1 - l1 + framePtr) % npoints;
        }
        else {
            index = (l1 + framePtr) % npoints;
        }
        sample = data.at(index);

        if (isRecording) {
            if (frameSize < 2) {
                sample.value = recValue;
            }
            else {
            /* We do linear interpolation of points within frames if
             * frameSize is > 0 to get a smooth recording at high resolutions
             * interpolation is linear between lastSampleValue and current recValue
             */
                sample.value = lastSampleValue
                            + (double)(recValue - lastSampleValue) / res * framelimit
                            * ((double)l1 + .5);
            }
            customWave.replace(index, sample);
            dataChanged = true;
        }
        sample.tick = lt;
        if (seqFinished) sample.muted = true;
        frame.replace(l1, sample);
        lt+=step;
        l1++;
    } while ((l1 < frameSize) && (l1 < npoints));


    reflect = pingpong;

    if ((!framePtr && !reverse)
        || (framePtr == npoints - l1 && reverse)) applyPendingParChanges();

    if (curLoopMode == 6) {
        framePtr = (rand() % npoints) / l1;
        framePtr *= l1;
    }
    else {
        if (reverse) {
            framePtr-=l1;
            if (framePtr < 0) {
                if (!enableLoop) seqFinished = true;
                framePtr = npoints - l1;
                if (reflect  || !backward) {
                    reverse = false;
                    framePtr = 0;
                }
            }
        }
        else {
            framePtr+=l1;
            if (framePtr >= npoints) {
                if (!enableLoop) seqFinished = true;
                framePtr = 0;
                if (reflect || backward) {
                    reverse = true;
                    framePtr = npoints - l1;
                }
            }
        }
    }
    int cur_grv_sft = 0.01 * (grooveTick * (step - 1));
    /* pairwise application of new groove shift */
    if (!(framePtr % 2)) {
        cur_grv_sft = -cur_grv_sft;
        grooveTick = newGrooveTick;
    }
    if (res > 16) cur_grv_sft = 0;

    lastSampleValue = recValue;

    nextTick = lt + cur_grv_sft;
    if (nextTick < (tick - lt)) nextTick = tick;
    sample.value = -1;
    sample.tick = nextTick;
    frame.replace(l1, sample);

    if (!trigByKbd && !(framePtr % 2) && !grooveTick) {
        /* round-up to current resolution (quantize) */
        nextTick/= (step * frameSize);
        nextTick*= (step * frameSize);
    }

    if (seqFinished) framePtr = 0;

}

void MidiLfo::getData(QVector<Sample> *p_data)
{
    //this function returns the full LFO wave

    Sample sample;
    const int step = TPQN / res;
    const int npoints = size * res;
    int val = 0;
    int lt = 0;
    bool cl = false;
    QVector<Sample> tmpdata;

    tmpdata.clear();

    switch(waveFormIndex) {
        case 0: //sine
            for (int l1 = 0; l1 < npoints; l1++) {
                sample.value = clip((-cos((double)(l1 * 6.28 /
                res * freq / 32)) + 1) * amp / 2 + offs, 0, 127, &cl);
                sample.tick = lt;
                sample.muted = muteMask.at(l1);
                tmpdata.append(sample);
                lt += step;
            }
        break;
        case 1: //sawtooth up
            val = 0;
            for (int l1 = 0; l1 < npoints; l1++) {
                sample.value = clip(val * amp / res / 32
                + offs, 0, 127, &cl);
                sample.tick = lt;
                sample.muted = muteMask.at(l1);
                tmpdata.append(sample);
                lt += step;
                val += freq;
                val %= res * 32;
            }
        break;
        case 2: //triangle
            val = 0;
            for (int l1 = 0; l1 < npoints; l1++) {
                int tempval = val - res * 16;
                if (tempval < 0 ) tempval = -tempval;
                sample.value = clip((res * 16 - tempval) * amp
                        / res / 16 + offs, 0, 127, &cl);
                sample.tick = lt;
                sample.muted = muteMask.at(l1);
                tmpdata.append(sample);
                lt += step;
                val += freq;
                val %= res * 32;
            }
        break;
        case 3: //sawtooth down
            val = 0;
            for (int l1 = 0; l1 < npoints; l1++) {
                sample.value = clip((res * 32 - val)
                        * amp / res / 32 + offs, 0, 127, &cl);
                sample.tick = lt;
                sample.muted = muteMask.at(l1);
                tmpdata.append(sample);
                lt+=step;
                val += freq;
                val %= res * 32;
            }
        break;
        case 4: //square
            for (int l1 = 0; l1 < npoints; l1++) {
                sample.value = clip(amp * ((l1 * freq / 16
                        / res) % 2 == 0) + offs, 0, 127, &cl);
                sample.tick = lt;
                sample.muted = muteMask.at(l1);
                tmpdata.append(sample);
                lt+=step;
            }
        break;
        case 5: //custom
            lt = step * npoints;
            tmpdata = customWave.mid(0, npoints);
        break;
        default:
        break;
    }
    sample.value = -1;
    sample.tick = lt;
    tmpdata.append(sample);
    data = tmpdata;
    *p_data = data;
}

void MidiLfo::updateWaveForm(int val)
{
    waveFormIndex = val;
}

void MidiLfo::updateFrequency(int val)
{
    freq = val;
}

void MidiLfo::updateAmplitude(int val)
{
    amp = val;
}

void MidiLfo::updateOffset(int val)
{
    if (isRecording) return;
    if (waveFormIndex == 5) updateCustomWaveOffset(val);
    offs = val;
}

void MidiLfo::updateResolution(int val)
{
    res = val;
    resizeAll();
}

void MidiLfo::updateSize(int val)
{
    size = val;
    resizeAll();
}

void MidiLfo::updateLoop(int val)
{
    backward = val&1;
    pingpong = val&2;
    enableLoop = !(val&4);
    curLoopMode = val;
    if (seqFinished) {
        seqFinished = false;
        setFramePtr(0);
    }
}

int MidiLfo::setCustomWavePoint(double mouseX, double mouseY, bool newpt)
{
    Sample sample;
    int loc = mouseX * (res * size);
    int Y = mouseY * 128;

    if (newpt || (lastMouseLoc >= (res * size))) {
    // the mouse was just clicked so we can directly set the point
        lastMouseLoc = loc;
        lastMouseY = Y;
    }

    if (loc == lastMouseLoc) lastMouseY = Y;

    do {
    //if the mouse was moved, we interpolate potentially missing points after
    //the last mouse position
        if (loc > lastMouseLoc) {
            lastMouseY += (double)(lastMouseY - Y) / (lastMouseLoc - loc) + .5;
            lastMouseLoc++;
        }
        if (loc < lastMouseLoc) {
            lastMouseY -= (double)(lastMouseY - Y) / (lastMouseLoc - loc) - .5;
            lastMouseLoc--;
        }
        sample = customWave.at(lastMouseLoc);
        sample.value = lastMouseY;
        customWave.replace(lastMouseLoc, sample);
    } while (lastMouseLoc != loc);

    newCustomOffset();
    return (loc);
}

int MidiLfo::mouseEvent(double mouseX, double mouseY, int buttons, int pressed)
{
    int ix = 0;
    if (buttons == 2) {
        if (pressed == 1) {
            lastMute = toggleMutePoint(mouseX);
            ix = lastMute;
        }
        else if (pressed == 0)
            ix = setMutePoint(mouseX, lastMute);
    }
    else if ((pressed != 2) && (buttons == 1)) {
        if (waveFormIndex < 5) copyToCustom();
        ix = setCustomWavePoint(mouseX, mouseY, pressed);
    }
    dataChanged = true;
    return (ix);
}

void MidiLfo::resizeAll()
{
    const int step = TPQN / res;
    const int npoints = res * size;
    Sample sample;

    framePtr%=npoints;

    if (maxNPoints < npoints) {
        int lt = 0;
        for (int l1 = 0; l1 < npoints; l1++) {
            if (l1 >= maxNPoints)
                muteMask.replace(l1, muteMask.at(l1 % maxNPoints));
            sample = customWave.at(l1 % maxNPoints);
            sample.tick = lt;
            sample.muted = muteMask.at(l1);
            customWave.replace(l1, sample);
            lt+=step;
        }
        maxNPoints = npoints;
    }
    nPoints = npoints;
    dataChanged = true;
}

void MidiLfo::copyToCustom()
{
    updateWaveForm(5);
    for (int l1 = 0; l1 < nPoints; l1++)
        customWave.replace(l1, data.at(l1));

}

void MidiLfo::newCustomOffset()
{
    int min = 127;
    const int npoints = res * size;
    for (int l1 = 0; l1 < npoints; l1++) {
        int value = customWave.at(l1).value;
        if (value < min) min = value;
    }
    cwmin = min;
#ifdef APPBUILD
    offs = min;
#endif
}

void MidiLfo::flipWaveVertical()
{
    Sample sample;
    int min = 127;
    int max = 0;
    const int npoints = res * size;
    
    if (waveFormIndex < 5) {
        copyToCustom();
    }
    
    for (int l1 = 0; l1 < npoints; l1++) {
        int value = customWave.at(l1).value;
        if (value < min) min = value;
        if (value > max) max = value;
    }

    for (int l1 = 0; l1 < npoints; l1++) {
        sample = customWave.at(l1);
        sample.value = min + max - sample.value;
        customWave.replace(l1, sample);
    }
    cwmin = min;
#ifdef APPBUILD
    offs = min;
#endif
}

void MidiLfo::updateCustomWaveOffset(int o)
{
    Sample sample;
    const int count = res * size;
    int l1 = 0;
    bool cl = false;

    while ((!cl) && (l1 < count)) {
        sample.value = clip(customWave.at(l1).value + o - cwmin,
                            0, 127, &cl);
        l1++;
        }

    if (cl) return;

    for (l1 = 0; l1 < count; l1++) {
        sample = customWave.at(l1);
        sample.value += o - cwmin;
        customWave.replace(l1, sample);
    }
    cwmin = o;
}

bool MidiLfo::toggleMutePoint(double mouseX)
{
    Sample sample;
    bool m;
    int loc = mouseX * (res * size);

    m = muteMask.at(loc);
    muteMask.replace(loc, !m);
    if (waveFormIndex == 5) {
        sample = customWave.at(loc);
        sample.muted = !m;
        customWave.replace(loc, sample);
    }
    lastMouseLoc = loc;
    return(!m);
}

int MidiLfo::setMutePoint(double mouseX, bool on)
{
    Sample sample;
    int loc = mouseX * (res * size);

    if (lastMouseLoc >= (res * size)) lastMouseLoc = loc;

    do {
        if (waveFormIndex == 5) {
            sample = customWave.at(lastMouseLoc);
            sample.muted = on;
            customWave.replace(lastMouseLoc, sample);
        }
        muteMask.replace(lastMouseLoc, on);
        if (loc > lastMouseLoc) lastMouseLoc++;
        if (loc < lastMouseLoc) lastMouseLoc--;
    } while (lastMouseLoc != loc);

    return (loc);
}

void MidiLfo::setFramePtr(int idx)
{
    framePtr = idx;
    if (!idx) {
        reverse = curLoopMode&1;
        seqFinished = (enableNoteOff && !noteCount);
        restartFlag = false;
        if (reverse) framePtr = res * size - 1;
    }
}

void MidiLfo::setRecordMode(bool on)
{
    if (!on) {
        isRecording = false;
        newCustomOffset();
        dataChanged = true;
    }
    recordMode = on;
}

void MidiLfo::record(int value)
{
    recValue = value;
    isRecording = true;
}

bool MidiLfo::handleEvent(MidiEvent inEv, int tick)
{

    if (!recordMode && (inEv.type == EV_CONTROLLER)) return(true);
    if (inEv.channel != chIn) return(true);
    if ((inEv.type == EV_CONTROLLER) && (inEv.data != ccnumberIn)) return(true);

    if ((inEv.type == EV_CONTROLLER) && recordMode) {
        record(inEv.value);
        return (false);
    }
    if (inEv.type != EV_NOTEON) return (true);
    
    if (((inEv.data < indexIn[0]) || (inEv.data > indexIn[1]))
        || ((inEv.value < rangeIn[0]) || (inEv.value > rangeIn[1]))) {
        return(true);
    }

    if (inEv.value) {
        /*This is a NOTE ON event*/
        if (restartByKbd && (!noteCount || trigLegato)) restartFlag = true;
        seqFinished = false;
        noteCount++;
        if (trigByKbd && ((noteCount == 1) || trigLegato)) {
            nextTick = tick + 2; //schedDelayTicks;
            gotKbdTrig = true;
        }
     }
    else {
        /*This is a NOTE OFF event*/
        if (enableNoteOff && (noteCount == 1)) seqFinished = true;
        if (noteCount) noteCount--;
    }
    return(false);
}

void MidiLfo::applyPendingParChanges()
{
    if (!parChangesPending) return;

    int olddefer = deferChanges;
    deferChanges = false;
    setMuted(isMutedDefer);
    deferChanges = olddefer;
    parChangesPending = false;
    needsGUIUpdate = true;
}

void MidiLfo::setNextTick(int tick)
{
    int tickres = TPQN/res;
    int pos = (tick/tickres) % nPoints;

    reverse = false;
    if (pingpong) reverse = (((tick/tickres) / nPoints) % 2);

    if (backward) reverse = !reverse;
    if (reverse) pos = nPoints - pos;

    setFramePtr(pos);
    nextTick = (tick/tickres) * tickres;
}
