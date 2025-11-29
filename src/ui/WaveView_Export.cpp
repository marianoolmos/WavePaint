
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
// a copy of this CPP code and associated documentation files (the
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

bool WaveView::exportToPng(const QString &fileName, const QColor &background)
{
    if (!m_doc)
        return false;
    if (fileName.isEmpty())
        return false;

    const auto &sigs = m_doc->signalList();
    int sampleCount = m_doc->sampleCount();
    int signalCount = static_cast<int>(sigs.size());

    if (signalCount == 0)
    {
        signalCount = 1;
    }

    int rightMargin = 20;
    int bottomMargin = 20;

    int contentWidth = m_leftMargin + sampleCount * m_cellWidth + rightMargin;
    int contentHeight = m_topMargin + signalCount * m_rowHeight + bottomMargin;

    // Tamaño lógico (el tamaño "normal" del widget)
    QSize logicalSize(contentWidth, contentHeight);

    // Factor de escala para exportar en más resolución
    const int scale = 3; // prueba 2 o 3

    // Tamaño real del PNG = lógico * scale
    QSize imageSize(logicalSize.width() * scale,
                    logicalSize.height() * scale);

    QImage image(imageSize, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    m_exportSize = logicalSize; // usamos el tamaño lógico en paintEvent
    m_exportBackground = background;

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    // dibujamos todo escalado
    painter.scale(scale, scale);

    // Usa la propia lógica de pintado de la vista
    render(&painter);

    // Restaurar estado de export
    m_exportBackground = QColor();
    m_exportSize = QSize();

    return image.save(fileName);
}

void WaveView::onDocumentChanged()
{
    updateGeometry();
    update();
}