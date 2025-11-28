
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



int WaveDocument::addMarker(int sampleIndex)
{
    pushUndoSnapshot();
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
    pushUndoSnapshot();
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
    pushUndoSnapshot();
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
