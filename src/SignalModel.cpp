// ========================================================================================
//  /@@      /@@                               /@@@@@@@           /@@             /@@
// | @@  /@ | @@                              | @@__  @@         |__/            | @@
// | @@ /@@@| @@  /@@@@@@  /@@    /@@ /@@@@@@ | @@  \ @@ /@@@@@@  /@@ /@@@@@@@  /@@@@@@
// | @@/@@ @@ @@ |____  @@|  @@  /@@//@@__  @@| @@@@@@@/|____  @@| @@| @@__  @@|_  @@_/
// | @@@@_  @@@@  /@@@@@@@ \  @@/@@/| @@@@@@@@| @@____/  /@@@@@@@| @@| @@  \ @@  | @@
// | @@@/ \  @@@ /@@__  @@  \  @@@/ | @@_____/| @@      /@@__  @@| @@| @@  | @@  | @@ /@@
// | @@/   \  @@|  @@@@@@@   \  @/  |  @@@@@@@| @@     |  @@@@@@@| @@| @@  | @@  |  @@@@/
// |__/     \__/ \_______/    \_/    \_______/|__/      \_______/|__/|__/  |__/   \___/
//
// ___|HHHHHHHHH|______|HHHHHHHHH|___ ___|HHHHHHHHH|___ ___|HHHHHHHHH|___ ___|HHHHHHHHH|___
//
//
// Project:       WavePaint
// Description:
//
// Author:       Mariano Olmos Martin
// Mail  :       mariano.olmos@outlook.com
// Date:         27/11/2025
// Version:      v0.0
// License: MIT License
//
// Copyright (c) 2025 Mariano Olmos
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this VHDL code and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject
// to the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//======================================================================

#include "SignalModel.h"
#include "io/WaveVcdImporter.h"
#include "io/WaveJsonIO.h"

#include <algorithm>

static constexpr int UNDEFINED_VALUE = -1;

WaveDocument::WaveDocument(QObject *parent)
    : QObject(parent),
      m_sampleCount(20),
      m_hasClipboardSignal(false)
{
    // Initial document: no signals, but with a default sample count
}

void WaveDocument::resizeSignals(int newSampleCount)
{
    for (auto &sig : m_signals)
    {
        int oldSize = static_cast<int>(sig.values.size());
        sig.values.resize(newSampleCount, UNDEFINED_VALUE);
        sig.labels.resize(newSampleCount); // QString() by default
        if (newSampleCount > oldSize)
        {
            // The new samples remain at UNDEFINED_VALUE and have empty labels
        }
    }
}

void WaveDocument::setSampleCount(int count)
{
    if (count <= 0)
        return;
    if (count == m_sampleCount)
        return;

    m_sampleCount = count;
    resizeSignals(m_sampleCount);
    emit dataChanged();
}

int WaveDocument::addBitSignal(const QString &name)
{
    Signal s(name, SignalType::Bit, m_sampleCount);
    s.color = QColor(0, 160, 0);
    std::fill(s.values.begin(), s.values.end(), 0); // default 0
    std::fill(s.labels.begin(), s.labels.end(), QString());
    m_signals.push_back(s);
    emit dataChanged();
    return static_cast<int>(m_signals.size()) - 1;
}

int WaveDocument::addVectorSignal(const QString &name)
{
    
    Signal s(name, SignalType::Vector, m_sampleCount);
    s.color = QColor(0, 160, 0);
    std::fill(s.values.begin(), s.values.end(), UNDEFINED_VALUE);
    std::fill(s.labels.begin(), s.labels.end(), QString());
    m_signals.push_back(s);
    emit dataChanged();
    return static_cast<int>(m_signals.size()) - 1;
}

int WaveDocument::addClockSignal(const QString &name, int pulses, int highSamples, int lowSamples)
{
    if (pulses <= 0 || highSamples < 0 || lowSamples < 0)
    {
        return -1;
    }

    int period = highSamples + lowSamples;
    if (period <= 0)
    {
        return -1;
    }

    int neededSamples = pulses * period;
    if (neededSamples > m_sampleCount)
    {
        setSampleCount(neededSamples);
    }

    Signal s(name, SignalType::Bit, m_sampleCount);
    std::fill(s.values.begin(), s.values.end(), 0);
    std::fill(s.labels.begin(), s.labels.end(), QString());

    for (int p = 0; p < pulses; ++p)
    {
        int base = p * period;
        int highStart = base + lowSamples;
        int highEnd = std::min(base + period, m_sampleCount);
        for (int i = highStart; i < highEnd; ++i)
        {
            s.values[i] = 1;
        }
    }

    m_signals.push_back(s);
    emit dataChanged();
    return static_cast<int>(m_signals.size()) - 1;
}

void WaveDocument::toggleBitValue(int signalIndex, int sampleIndex)
{
    if (signalIndex < 0 || signalIndex >= static_cast<int>(m_signals.size()))
        return;
    if (sampleIndex < 0 || sampleIndex >= m_sampleCount)
        return;

    Signal &s = m_signals[signalIndex];
    if (s.type != SignalType::Bit)
        return;

    int &v = s.values[sampleIndex];
    if (v == 1)
        v = 0;
    else
        v = 1;

    emit dataChanged();
}

void WaveDocument::setBitValue(int signalIndex, int sampleIndex, int value)
{
    if (signalIndex < 0 || signalIndex >= static_cast<int>(m_signals.size()))
        return;
    if (sampleIndex < 0 || sampleIndex >= m_sampleCount)
        return;

    Signal &s = m_signals[signalIndex];
    if (s.type != SignalType::Bit)
        return;

    int v = (value != 0) ? 1 : 0;
    if (s.values[sampleIndex] == v)
        return;

    s.values[sampleIndex] = v;
    s.labels[sampleIndex].clear(); // no labels for bits
    emit dataChanged();
}

void WaveDocument::setVectorRange(int signalIndex, int startSample, int endSample, int value, const QString &label)
{
    if (signalIndex < 0 || signalIndex >= static_cast<int>(m_signals.size()))
        return;
    if (startSample < 0 && endSample < 0)
        return;

    Signal &s = m_signals[signalIndex];
    if (s.type != SignalType::Vector)
        return;

    int s0 = std::max(0, std::min(startSample, endSample));
    int s1 = std::min(m_sampleCount - 1, std::max(startSample, endSample));

    for (int i = s0; i <= s1; ++i)
    {
            s.values[i] = value;
            s.labels[i] = label;
      
    }

    emit dataChanged();
}

void WaveDocument::clearSample(int signalIndex, int sampleIndex)
{
    if (signalIndex < 0 || signalIndex >= static_cast<int>(m_signals.size()))
        return;
    if (sampleIndex < 0 || sampleIndex >= m_sampleCount)
        return;

    Signal &s = m_signals[signalIndex];
    s.values[sampleIndex] = UNDEFINED_VALUE;
    if (sampleIndex < static_cast<int>(s.labels.size()))
        s.labels[sampleIndex].clear();
    emit dataChanged();
}

void WaveDocument::setSignalColor(int signalIndex, const QColor &c)
{
    if (signalIndex < 0 || signalIndex >= static_cast<int>(m_signals.size()))
        return;
    m_signals[signalIndex].color = c;
    emit dataChanged();
}

void WaveDocument::renameSignal(int signalIndex, const QString &name)
{
    if (signalIndex < 0 || signalIndex >= static_cast<int>(m_signals.size()))
        return;
    m_signals[signalIndex].name = name;
    emit dataChanged();
}

void WaveDocument::clear()
{
    m_signals.clear();
    m_vcdSignals.clear();
    m_sampleCount = 20;
    emit dataChanged();
}

void WaveDocument::clearSignals()
{
    m_signals.clear();
    emit dataChanged();
}

void WaveDocument::cutRange(int startSample, int endSample)
{
    if (m_sampleCount <= 0)
        return;
    if (startSample < 0 && endSample < 0)
        return;

    if (startSample < 0)
        startSample = 0;
    if (endSample < 0)
        endSample = 0;
    if (startSample > endSample)
        std::swap(startSample, endSample);

    if (startSample >= m_sampleCount)
        return;
    if (endSample >= m_sampleCount)
        endSample = m_sampleCount - 1;

    int newCount = endSample - startSample + 1;
    if (newCount <= 0)
        return;

    for (auto &sig : m_signals)
    {
        std::vector<int> newValues(newCount, UNDEFINED_VALUE);
        std::vector<QString> newLabels(newCount);

        for (int i = 0; i < newCount; ++i)
        {
            int src = startSample + i;
            if (src >= 0 && src < static_cast<int>(sig.values.size()))
            {
                newValues[i] = sig.values[src];
            }
            else
            {
                newValues[i] = UNDEFINED_VALUE;
            }
            if (src >= 0 && src < static_cast<int>(sig.labels.size()))
            {
                newLabels[i] = sig.labels[src];
            }
        }

        sig.values.swap(newValues);
        sig.labels.swap(newLabels);
    }

    m_sampleCount = newCount;
    emit dataChanged();
}

bool WaveDocument::loadFromVcd(const QString &fileName)
{
    return WaveVcdImporter::loadFromVcd(*this, fileName);
}

int WaveDocument::addSignalFromVcd(const QString &fullName)
{
    // Search for the signal in the VCD library and copy it to the visible list
    int idx = -1;
    for (int i = 0; i < static_cast<int>(m_vcdSignals.size()); ++i)
    {
        if (m_vcdSignals[i].name == fullName)
        {
            idx = i;
            break;
        }
    }
    if (idx < 0)
        return -1;

    const Signal &src = m_vcdSignals[idx];

    // Ensure sampleCount is consistent
    if (static_cast<int>(src.values.size()) != m_sampleCount)
    {
        m_sampleCount = static_cast<int>(src.values.size());
        resizeSignals(m_sampleCount);
    }

    m_signals.push_back(src);
    emit dataChanged();
    return static_cast<int>(m_signals.size()) - 1;
}

bool WaveDocument::saveToFile(const QString &fileName) const
{
    return WaveJsonIO::saveToFile(*this, fileName);
}

bool WaveDocument::loadFromFile(const QString &fileName)
{
    return WaveJsonIO::loadFromFile(*this, fileName);
}

void WaveDocument::copySignal(int signalIndex)
{
    if (signalIndex < 0 || signalIndex >= static_cast<int>(m_signals.size()))
        return;

    m_clipboardSignal = m_signals[signalIndex];
    m_hasClipboardSignal = true;
}

int WaveDocument::pasteSignal(int destIndex)
{
    if (!m_hasClipboardSignal)
        return -1;

    Signal s = m_clipboardSignal;

    // 1 Name only
    QString baseName = s.name;
    QString newName = baseName;
    int count = static_cast<int>(m_signals.size());
    int copyIndex = 1;
    while (std::any_of(m_signals.begin(), m_signals.end(),
                       [&](const Signal &sig)
                       { return sig.name == newName; }))
    {
        newName = baseName + QStringLiteral("_copy%1").arg(copyIndex++);
    }
    s.name = newName;

    if (destIndex < 0 || destIndex > count)
        destIndex = count;

    m_signals.insert(m_signals.begin() + destIndex, s);
    emit dataChanged();
    return destIndex;
}
void WaveDocument::removeSignal(int signalIndex)
{
    int n = static_cast<int>(m_signals.size());
    if (signalIndex < 0 || signalIndex >= n)
        return;

    m_signals.erase(m_signals.begin() + signalIndex);

    // Eliminar flechas que usan esa señal
    m_arrows.erase(
        std::remove_if(m_arrows.begin(), m_arrows.end(),
                       [signalIndex](const Arrow &a) {
                           return a.startSignal == signalIndex ||
                                  a.endSignal   == signalIndex;
                       }),
        m_arrows.end()
    );

    // Ajustar índices de flechas que están por debajo
    for (Arrow &a : m_arrows) {
        if (a.startSignal > signalIndex) a.startSignal--;
        if (a.endSignal   > signalIndex) a.endSignal--;
    }

    emit dataChanged();
}

int WaveDocument::addMarker(int sampleIndex)
{
    if (sampleIndex < 0 || sampleIndex >= m_sampleCount)
        return -1;

    // Si ya hay un marcador en ese sample, devolvemos su id
    for (const Marker &m : m_markers) {
        if (m.sample == sampleIndex)
            return m.id;
    }

    Marker m;
    m.id     = m_nextMarkerId++;
    m.sample = sampleIndex;

    m_markers.push_back(m);

    // Opcional: ordenarlos por sample
    std::sort(m_markers.begin(), m_markers.end(),
              [](const Marker &a, const Marker &b) {
                  return a.sample < b.sample;
              });

    emit dataChanged();
    return m.id;
}

void WaveDocument::subMarkerById(int markerId)
{
    auto it = std::find_if(m_markers.begin(), m_markers.end(),
                           [markerId](const Marker &m) { return m.id == markerId; });
    if (it == m_markers.end())
        return;

    m_markers.erase(it);

    // 👇 Recalcular el siguiente ID
    if (m_markers.empty()) {
        // Si ya no hay marcadores, empezamos de nuevo en 1
        m_nextMarkerId = 1;
    } else {
        // Si aún quedan, ponemos el siguiente al máximo + 1
        int maxId = 0;
        for (const Marker &m : m_markers) {
            if (m.id > maxId)
                maxId = m.id;
        }
        m_nextMarkerId = maxId + 1;
    }

    emit dataChanged();
}


void WaveDocument::clearMarkers()
{
    m_markers.clear();
    m_nextMarkerId = 1;
    emit dataChanged();
}

void WaveDocument::addMarkerFromLoad(int id, int sampleIndex)
{
    if (sampleIndex < 0 || sampleIndex >= m_sampleCount)
        return;

    Marker m;
    m.id     = id;
    m.sample = sampleIndex;
    m_markers.push_back(m);

    if (id >= m_nextMarkerId)
        m_nextMarkerId = id + 1;
}
int WaveDocument::addArrow(int startSignal, int startSample,
                           int endSignal,   int endSample)
{
    int sigCount = static_cast<int>(m_signals.size());
    if (startSignal < 0 || startSignal >= sigCount) return -1;
    if (endSignal   < 0 || endSignal   >= sigCount) return -1;
    if (startSample < 0 || startSample >= m_sampleCount) return -1;
    if (endSample   < 0 || endSample   >= m_sampleCount) return -1;

    Arrow a;
    a.id          = m_nextArrowId++;
    a.startSignal = startSignal;
    a.startSample = startSample;
    a.endSignal   = endSignal;
    a.endSample   = endSample;

    m_arrows.push_back(a);
    emit dataChanged();
    return a.id;
}

void WaveDocument::subArrowById(int arrowId)
{
    auto it = std::find_if(m_arrows.begin(), m_arrows.end(),
                           [arrowId](const Arrow &a){ return a.id == arrowId; });
    if (it == m_arrows.end())
        return;

    m_arrows.erase(it);

    // Recalcular siguiente ID si quieres "compactar"
    if (m_arrows.empty()) {
        m_nextArrowId = 1;
    } else {
        int maxId = 0;
        for (const Arrow &a : m_arrows) {
            if (a.id > maxId) maxId = a.id;
        }
        m_nextArrowId = maxId + 1;
    }

    emit dataChanged();
}

void WaveDocument::clearArrows()
{
    m_arrows.clear();
    m_nextArrowId = 1;
    emit dataChanged();
}
