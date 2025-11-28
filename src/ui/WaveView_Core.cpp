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
#include <QPainterPath>
#include <cmath>
#include "WaveView.h"
#include <QPainter>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QInputDialog>
#include <QFontMetrics>
#include <QColorDialog>
#include <QLinearGradient>
#include <QMessageBox>
#include <QCursor>
#include <QKeyEvent>



WaveView::WaveView(WaveDocument *doc, QWidget *parent)
    : QWidget(parent),
      m_doc(doc),
      m_rowHeight(40),
      m_cellWidth(20),
      m_leftMargin(100),
      m_topMargin(34),
      m_mode(Mode::None),
      m_selSignal(-1),
      m_selStartSample(-1),
      m_selCurrentSample(-1),
      m_bitPaintSignal(-1),
      m_bitPaintValue(0),
      m_bitLastSample(-1),
      m_cutStartSample(-1),
      m_cutCurrentSample(-1),
      m_isMovingSignal(false),
      m_moveSignalIndex(-1),
      m_markerPreviewSample(-1),
      m_arrowHasStart(false),
      m_arrowStartSignal(-1),
      m_arrowStartSample(-1),
      m_arrowPreviewSignal(-1),
      m_arrowPreviewSample(-1),
      m_exportSize(),
      m_exportBackground()
{
    setMouseTracking(true);
    setAutoFillBackground(true);
    setFocusPolicy(Qt::StrongFocus);

    if (m_doc)
    {
        connect(m_doc, &WaveDocument::dataChanged,
                this, &WaveView::onDocumentChanged);
    }
}

QSize WaveView::minimumSizeHint() const
{
    return QSize(200, 150);
}

QSize WaveView::sizeHint() const
{
    if (!m_doc)
    {
        return QSize(800, 400);
    }

    const auto &sigs = m_doc->signalList();
    int sampleCount = m_doc->sampleCount();
    int signalCount = static_cast<int>(sigs.size());
    if (signalCount <= 0)
        signalCount = 1;

    int rightMargin = 40;
    int bottomMargin = 20;

    int w = m_leftMargin + sampleCount * m_cellWidth + rightMargin;
    int h = m_topMargin + signalCount * m_rowHeight + bottomMargin;

    return QSize(w, h);
}