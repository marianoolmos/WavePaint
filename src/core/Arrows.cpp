
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

int WaveDocument::addArrow(int startSignal, int startSample,
                           int endSignal,   int endSample)
{
    pushUndoSnapshot();
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
    pushUndoSnapshot();
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
    pushUndoSnapshot();
    m_arrows.clear();
    m_nextArrowId = 1;
    emit dataChanged();
}
