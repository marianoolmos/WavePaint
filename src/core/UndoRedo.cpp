
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

void WaveDocument::pushUndoSnapshot()
{
    Snapshot snap;
    snap.sampleCount   = m_sampleCount;
    snap.m_signals     = m_signals;
    snap.m_vcdSignals  = m_vcdSignals;
    snap.m_markers     = m_markers;
    snap.m_nextMarkerId= m_nextMarkerId;
    snap.m_arrows      = m_arrows;
    snap.m_nextArrowId = m_nextArrowId;

    m_undoStack.push_back(std::move(snap));
    if ((int)m_undoStack.size() > m_maxUndoSteps) {
        m_undoStack.erase(m_undoStack.begin());
    }

    // Cada vez que hay un punto de undo nuevo, el redo se limpia
    m_redoStack.clear();

    emit undoRedoStateChanged();
}

void WaveDocument::clearHistory()
{
    m_undoStack.clear();
    m_redoStack.clear();
    emit undoRedoStateChanged();
}

bool WaveDocument::canUndo() const
{
    return !m_undoStack.empty();
}

bool WaveDocument::canRedo() const
{
    return !m_redoStack.empty();
}

void WaveDocument::undo()
{
    if (m_undoStack.empty())
        return;

    // Guardar estado actual en REDO
    Snapshot cur;
    cur.sampleCount   = m_sampleCount;
    cur.m_signals     = m_signals;
    cur.m_vcdSignals  = m_vcdSignals;
    cur.m_markers     = m_markers;
    cur.m_nextMarkerId= m_nextMarkerId;
    cur.m_arrows      = m_arrows;
    cur.m_nextArrowId = m_nextArrowId;
    m_redoStack.push_back(std::move(cur));

    // Recuperar último snapshot en UNDO
    Snapshot snap = std::move(m_undoStack.back());
    m_undoStack.pop_back();

    m_sampleCount   = snap.sampleCount;
    m_signals       = std::move(snap.m_signals);
    m_vcdSignals    = std::move(snap.m_vcdSignals);
    m_markers       = std::move(snap.m_markers);
    m_nextMarkerId  = snap.m_nextMarkerId;
    m_arrows        = std::move(snap.m_arrows);
    m_nextArrowId   = snap.m_nextArrowId;

    emit dataChanged();
    emit undoRedoStateChanged();
}

void WaveDocument::redo()
{
    if (m_redoStack.empty())
        return;

    // Guardar estado actual en UNDO
    Snapshot cur;
    cur.sampleCount   = m_sampleCount;
    cur.m_signals     = m_signals;
    cur.m_vcdSignals  = m_vcdSignals;
    cur.m_markers     = m_markers;
    cur.m_nextMarkerId= m_nextMarkerId;
    cur.m_arrows      = m_arrows;
    cur.m_nextArrowId = m_nextArrowId;
    m_undoStack.push_back(std::move(cur));

    // Recuperar snapshot de REDO
    Snapshot snap = std::move(m_redoStack.back());
    m_redoStack.pop_back();

    m_sampleCount   = snap.sampleCount;
    m_signals       = std::move(snap.m_signals);
    m_vcdSignals    = std::move(snap.m_vcdSignals);
    m_markers       = std::move(snap.m_markers);
    m_nextMarkerId  = snap.m_nextMarkerId;
    m_arrows        = std::move(snap.m_arrows);
    m_nextArrowId   = snap.m_nextArrowId;

    emit dataChanged();
    emit undoRedoStateChanged();
}
