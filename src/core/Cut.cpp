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
// Date:         2025
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

void WaveDocument::cutRange(int startSample, int endSample)
{
    pushUndoSnapshot();
    if (m_sampleCount <= 0)
        return;

    if (startSample < 0 || endSample < 0)
        return;
    if (startSample >= m_sampleCount || endSample >= m_sampleCount)
        return;

    // Asegurar que start <= end
    if (startSample > endSample)
        std::swap(startSample, endSample);

    const int first    = startSample;
    const int last     = endSample;
    const int newCount = last - first + 1;
    if (newCount <= 0 || newCount == m_sampleCount) {
        // Nada que recortar
        return;
    }

    // --- 1) Recortar TODAS las señales al rango [first, last] y renumerar ---
    for (Signal &s : m_signals) {
        // Valores
        if (static_cast<int>(s.values.size()) < m_sampleCount) {
            s.values.resize(m_sampleCount, -1);   // -1 = indefinido
        }

        std::vector<int> newVals(newCount, -1);
        for (int i = 0; i < newCount; ++i) {
            int src = first + i;
            if (src >= 0 && src < static_cast<int>(s.values.size()))
                newVals[i] = s.values[src];
        }
        s.values.swap(newVals);

        // Labels
        if (static_cast<int>(s.labels.size()) < m_sampleCount) {
            s.labels.resize(m_sampleCount);
        }

        std::vector<QString> newLabs(newCount);
        for (int i = 0; i < newCount; ++i) {
            int src = first + i;
            if (src >= 0 && src < static_cast<int>(s.labels.size()))
                newLabs[i] = s.labels[src];
        }
        s.labels.swap(newLabs);
    }

    // --- 2) Ajustar MARCADORES ---
    if (!m_markers.empty()) {
        std::vector<Marker> newMarkers;
        newMarkers.reserve(m_markers.size());

        for (Marker m : m_markers) {
            // Si el marker está fuera del rango que conservamos, lo eliminamos
            if (m.sample < first || m.sample > last)
                continue;

            // Nuevo índice = viejo - first
            m.sample -= first;
            newMarkers.push_back(m);
        }

        m_markers.swap(newMarkers);

        // Recalcular siguiente ID de marker
        int maxId = 0;
        for (const Marker &m : m_markers) {
            if (m.id > maxId)
                maxId = m.id;
        }
        m_nextMarkerId = (maxId > 0) ? maxId + 1 : 1;
    }

    // --- 3) Ajustar FLECHAS ---
    if (!m_arrows.empty()) {
        std::vector<Arrow> newArrows;
        newArrows.reserve(m_arrows.size());

        for (Arrow a : m_arrows) {
            // Si cualquiera de los extremos queda fuera del nuevo rango, descartamos la flecha
            if (a.startSample < first || a.startSample > last ||
                a.endSample   < first || a.endSample   > last) {
                continue;
            }

            // Renumerar samples
            a.startSample -= first;
            a.endSample   -= first;

            newArrows.push_back(a);
        }

        m_arrows.swap(newArrows);

        // Recalcular siguiente ID de flecha
        int maxId = 0;
        for (const Arrow &a : m_arrows) {
            if (a.id > maxId)
                maxId = a.id;
        }
        m_nextArrowId = (maxId > 0) ? maxId + 1 : 1;
    }

    // --- 4) Actualizar sampleCount y avisar a la vista ---
    m_sampleCount = newCount;
    emit dataChanged();
}
