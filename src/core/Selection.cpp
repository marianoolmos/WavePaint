
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

#include "core/core.h"


void WaveDocument::copyBlock(int topSignal, int bottomSignal,
                             int startSample, int endSample)
{
  
    int sigCount    = static_cast<int>(m_signals.size());
    int sampleCount = m_sampleCount;
    if (sigCount == 0 || sampleCount == 0)
        return;

    if (topSignal < 0 || bottomSignal < 0 ||
        topSignal >= sigCount || bottomSignal >= sigCount)
        return;

    if (topSignal > bottomSignal)
        std::swap(topSignal, bottomSignal);

    if (startSample < 0 || endSample < 0 ||
        startSample >= sampleCount || endSample >= sampleCount)
        return;

    if (startSample > endSample)
        std::swap(startSample, endSample);

    int rows = bottomSignal - topSignal + 1;
    int cols = endSample    - startSample + 1;

    if (rows <= 0 || cols <= 0)
        return;

    m_blockClipboardSignalCount = rows;
    m_blockClipboardSampleCount = cols;

    m_blockClipboardValues.assign(rows, std::vector<int>(cols, UNDEFINED_VALUE));
    m_blockClipboardLabels.assign(rows, std::vector<QString>(cols));
    m_blockClipboardTypes.resize(rows);
    m_blockClipboardColors.resize(rows);

    for (int r = 0; r < rows; ++r) {
        const Signal &s = m_signals[topSignal + r];

        m_blockClipboardTypes[r]  = s.type;
        m_blockClipboardColors[r] = s.color;

        for (int c = 0; c < cols; ++c) {
            int src = startSample + c;
            if (src >= 0 && src < static_cast<int>(s.values.size())) {
                m_blockClipboardValues[r][c] = s.values[src];
            }
            if (src >= 0 && src < static_cast<int>(s.labels.size())) {
                m_blockClipboardLabels[r][c] = s.labels[src];
            }
        }
    }

    m_hasBlockClipboard = true;
    // No emitimos dataChanged() porque no cambia el documento visible
}


void WaveDocument::pasteBlock(int destTopSignal, int destStartSample)
{
    pushUndoSnapshot();
    if (!m_hasBlockClipboard)
        return;

    int sigCount    = static_cast<int>(m_signals.size());
    int sampleCount = m_sampleCount;
    if (sigCount == 0 || sampleCount == 0)
        return;

    if (destTopSignal < 0 || destTopSignal >= sigCount)
        return;
    if (destStartSample < 0 || destStartSample >= sampleCount)
        return;

    int rows = m_blockClipboardSignalCount;
    int cols = m_blockClipboardSampleCount;

    int maxRows = std::min(rows, sigCount    - destTopSignal);
    int maxCols = std::min(cols, sampleCount - destStartSample);

    for (int r = 0; r < maxRows; ++r) {
        Signal &s = m_signals[destTopSignal + r];

        if (static_cast<int>(s.values.size()) < sampleCount)
            s.values.resize(sampleCount, UNDEFINED_VALUE);
        if (static_cast<int>(s.labels.size()) < sampleCount)
            s.labels.resize(sampleCount);

        for (int c = 0; c < maxCols; ++c) {
            int dst = destStartSample + c;
            if (dst < 0 || dst >= sampleCount)
                continue;

            s.values[dst] = m_blockClipboardValues[r][c];
            s.labels[dst] = m_blockClipboardLabels[r][c];
        }
    }

    emit dataChanged();
}

void WaveDocument::clearBlock(int topSignal, int bottomSignal,
                              int startSample, int endSample)
{
    pushUndoSnapshot();
    int sigCount    = static_cast<int>(m_signals.size());
    int sampleCount = m_sampleCount;
    if (sigCount == 0 || sampleCount == 0)
        return;

    if (topSignal < 0 || bottomSignal < 0 ||
        topSignal >= sigCount || bottomSignal >= sigCount)
        return;
    if (topSignal > bottomSignal)
        std::swap(topSignal, bottomSignal);

    if (startSample < 0 || endSample < 0 ||
        startSample >= sampleCount || endSample >= sampleCount)
        return;
    if (startSample > endSample)
        std::swap(startSample, endSample);

    for (int sIdx = topSignal; sIdx <= bottomSignal; ++sIdx) {
        Signal &s = m_signals[sIdx];

        pushUndoSnapshot();
        if (static_cast<int>(s.values.size()) < sampleCount)
            s.values.resize(sampleCount, UNDEFINED_VALUE);
        if (static_cast<int>(s.labels.size()) < sampleCount)
            s.labels.resize(sampleCount);

        for (int t = startSample; t <= endSample; ++t) {
            s.values[t] = UNDEFINED_VALUE;
            s.labels[t].clear();
        }
    }

    emit dataChanged();
}
