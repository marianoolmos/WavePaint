
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

void WaveView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);

    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    QColor bg = m_exportBackground.isValid() ? m_exportBackground : palette().base().color();

    int w = m_exportSize.isValid() ? m_exportSize.width() : width();
    int h = m_exportSize.isValid() ? m_exportSize.height() : height();
    QRect drawRect(0, 0, w, h);
    p.fillRect(drawRect, bg);

    if (!m_doc)
        return;

    const auto &sigs = m_doc->signalList();
    int sampleCount = m_doc->sampleCount();

    // Text color for axes depending on background
    int lumBg = qRound(0.299 * bg.red() + 0.587 * bg.green() + 0.114 * bg.blue());
    QColor axisColor = (lumBg < 128) ? Qt::white : Qt::black;

    // Time axis (sample indices)
    p.setPen(axisColor);
    QFontMetrics fm(p.font());
    // Colocamos los números de tiempo bien arriba
    int axisTextY = m_topMargin - fm.height() * 2 - 4;
    if (axisTextY < fm.ascent())
        axisTextY = fm.ascent();

    int labelStep = 1;
    if (sampleCount > 200)
        labelStep = 5;
    if (sampleCount > 1000)
        labelStep = 10;
    if (sampleCount > 5000)
        labelStep = 50;
    if (sampleCount > 20000)
        labelStep = 100;

    for (int t = 0; t < sampleCount; t += labelStep)
    {
        int x = m_leftMargin + t * m_cellWidth;
        QString label = QString::number(t);
        int tw = fm.horizontalAdvance(label);
        p.drawText(x + (m_cellWidth - tw) / 2, axisTextY, label);
    }
    // Vertical grid (dashed lines)
    QPen gridPen(QColor(220, 220, 220));
    gridPen.setStyle(Qt::DashLine);
    p.setPen(gridPen);

    int gridStep = 1;
    if (sampleCount > 2000)
        gridStep = 5;
    if (sampleCount > 5000)
        gridStep = 10;
    if (sampleCount > 20000)
        gridStep = 50;

    for (int t = 0; t <= sampleCount; t += gridStep)
    {
        int x = m_leftMargin + t * m_cellWidth;
        p.drawLine(x, m_topMargin, x, h);
    }

    // Draw signals
    p.setPen(axisColor);
    for (int i = 0; i < static_cast<int>(sigs.size()); ++i)
    {
        drawSignal(p, sigs[i], i);
    }

    // Draw vector selection (if any)
    drawVectorSelection(p);

    // Cut lines (cut mode)
    if (m_mode == Mode::CutSelecting)
    {
        QPen cutPen(Qt::red);
        cutPen.setWidth(2);
        p.setPen(cutPen);

        if (m_cutStartSample >= 0)
        {
            int x0 = m_leftMargin + m_cutStartSample * m_cellWidth;
            p.drawLine(x0, m_topMargin, x0, h);
        }
        if (m_cutCurrentSample >= 0 && m_cutCurrentSample != m_cutStartSample)
        {
            int x1 = m_leftMargin + m_cutCurrentSample * m_cellWidth;
            p.drawLine(x1, m_topMargin, x1, h);
        }
    }
    if (m_mode == Mode::MarkerAdd && m_markerPreviewSample >= 0 &&
        m_markerPreviewSample < sampleCount)
    {

        QPen previewPen(Qt::yellow);
        previewPen.setWidth(1);
        previewPen.setStyle(Qt::DashLine);
        p.setPen(previewPen);

        int x = m_leftMargin + m_markerPreviewSample * m_cellWidth;
        p.drawLine(x, m_topMargin, x, h - 1);
    }
    // --- Marcadores amarillos ---
    const auto &markers = m_doc->markerList();
    if (!markers.empty())
    {
        QPen markerPen(Qt::yellow);
        markerPen.setWidth(2);
        p.setPen(markerPen);

        QFont oldFont = p.font();
        QFont smallFont = oldFont;
        smallFont.setPointSize(std::max(6, oldFont.pointSize() - 1));
        p.setFont(smallFont);
        QFontMetrics fm(smallFont);

        // Y de los números de marcador: entre el eje de tiempo y la primera señal
        int markerLabelY = m_topMargin - fm.height() - 2;

        int index = 0;
        for (const Marker &mk : markers)
        {
            int sample = mk.sample;
            int number = mk.id;
            if (sample < 0 || sample >= sampleCount)
                continue;

            int x = m_leftMargin + sample * m_cellWidth;

            // Línea amarilla
            p.drawLine(x, m_topMargin, x, h - 1);

            QString label = QString::number(number);
            int tw = fm.horizontalAdvance(label);
            int th = fm.height();

            int labelX = x - tw - 6;
            if (labelX < 0)
                labelX = 0;
            QRect r(labelX, markerLabelY, tw + 6, th);

            p.save();
            p.setBrush(QColor(255, 255, 0, 220));
            p.setPen(Qt::black);
            p.drawRect(r);
            p.drawText(r, Qt::AlignCenter, label);
            p.restore();

            ++index;
        }

        p.setFont(oldFont);
    }
    // --- Flechas existentes ---
    const auto &arrows = m_doc->arrowList();
    if (!arrows.empty())
    {
        QPen arrowPen(Qt::red);
        arrowPen.setWidth(2);
        arrowPen.setCapStyle(Qt::RoundCap);
        arrowPen.setJoinStyle(Qt::RoundJoin);
        p.setPen(arrowPen);

        for (const Arrow &a : arrows)
        {
            QPointF p1 = signalSampleToPoint(a.startSignal, a.startSample);
            QPointF p2 = signalSampleToPoint(a.endSignal, a.endSample);

            QPainterPath path;
            path.moveTo(p1);

            // Curva simple: control en el medio, un poco por arriba
            qreal midX = (p1.x() + p2.x()) / 2.0;
            qreal dy = std::abs(p1.y() - p2.y());
            qreal lift = std::max<qreal>(m_rowHeight, dy / 2.0 + m_rowHeight / 2.0);
            qreal ctrlY = std::min(p1.y(), p2.y()) - lift;

            QPointF ctrl(midX, ctrlY);
            path.quadTo(ctrl, p2);

            p.drawPath(path);

            // Cabeza de flecha (triángulo) en el extremo p2
            qreal angle = std::atan2(p2.y() - ctrlY, p2.x() - midX); // aprox dirección
            qreal arrowSize = 10.0;

            QPointF arrowP1 = p2 - QPointF(std::cos(angle - M_PI / 6) * arrowSize,
                                           std::sin(angle - M_PI / 6) * arrowSize);
            QPointF arrowP2 = p2 - QPointF(std::cos(angle + M_PI / 6) * arrowSize,
                                           std::sin(angle + M_PI / 6) * arrowSize);

            p.drawLine(p2, arrowP1);
            p.drawLine(p2, arrowP2);
        }
    }

    // --- Flecha en previsualización (modo ArrowAdd) ---
    if (m_mode == Mode::ArrowAdd && m_arrowHasStart &&
        m_arrowPreviewSignal >= 0 && m_arrowPreviewSample >= 0)
    {

        QPen previewPen(Qt::red);
        previewPen.setWidth(1);
        previewPen.setStyle(Qt::DashLine);
        p.setPen(previewPen);

        QPointF p1 = signalSampleToPoint(m_arrowStartSignal, m_arrowStartSample);
        QPointF p2 = signalSampleToPoint(m_arrowPreviewSignal, m_arrowPreviewSample);

        QPainterPath path;
        path.moveTo(p1);

        qreal midX = (p1.x() + p2.x()) / 2.0;
        qreal dy = std::abs(p1.y() - p2.y());
        qreal lift = std::max<qreal>(m_rowHeight, dy / 2.0 + m_rowHeight / 2.0);
        qreal ctrlY = std::min(p1.y(), p2.y()) - lift;
        QPointF ctrl(midX, ctrlY);

        path.quadTo(ctrl, p2);
        p.drawPath(path);
    }

    if (m_blockSelectionActive && m_doc)
    {
        int topSig, bottomSig, startSample, endSample;
        if (normalizedBlockSelection(topSig, bottomSig, startSample, endSample))
        {
            int x1 = m_leftMargin + startSample * m_cellWidth;
            int x2 = m_leftMargin + (endSample + 1) * m_cellWidth;
            int y1 = m_topMargin + topSig * m_rowHeight;
            int y2 = m_topMargin + (bottomSig + 1) * m_rowHeight;

            QRect selRect(x1, y1, x2 - x1, y2 - y1);

            QColor fill(0, 120, 215, 40);
            QPen pen(QColor(0, 120, 215));
            pen.setStyle(Qt::DashLine);
            pen.setWidth(2);

            p.save();
            p.setPen(pen);
            p.setBrush(fill);
            p.drawRect(selRect);
            p.restore();
        }
    }

    if (m_blockSelectionActive && m_doc)
    {
        int topSig, bottomSig, startSample, endSample;
        if (normalizedBlockSelection(topSig, bottomSig, startSample, endSample))
        {
            int x1 = m_leftMargin + startSample * m_cellWidth;
            int x2 = m_leftMargin + (endSample + 1) * m_cellWidth;
            int y1 = m_topMargin + topSig * m_rowHeight;
            int y2 = m_topMargin + (bottomSig + 1) * m_rowHeight;

            QRect selRect(x1, y1, x2 - x1, y2 - y1);

            QColor fill(0, 120, 215, 40);
            QPen pen(QColor(0, 120, 215));
            pen.setStyle(Qt::DashLine);
            pen.setWidth(2);

            p.save();
            p.setPen(pen);
            p.setBrush(fill);
            p.drawRect(selRect);
            p.restore();
        }
    }

    // --- Preview de pegado (rectángulo que sigue al ratón) ---
    if (m_selectionModeEnabled &&
        m_blockPastePreviewActive &&
        m_doc && m_doc->hasBlockClipboard())
    {

        const auto &sigs = m_doc->signalList();
        int sigCount = static_cast<int>(sigs.size());
        int sampleCount = m_doc->sampleCount();

        int clipRows = m_doc->blockClipboardSignalCount();
        int clipCols = m_doc->blockClipboardSampleCount();

        if (sigCount > 0 && sampleCount > 0 &&
            clipRows > 0 && clipCols > 0)
        {

            int topSig = m_blockPasteSignal;
            int startSample = m_blockPasteSample;

            if (topSig < 0)
                topSig = 0;
            if (topSig > sigCount - clipRows)
                topSig = std::max(0, sigCount - clipRows);
            if (startSample < 0)
                startSample = 0;
            if (startSample > sampleCount - clipCols)
                startSample = std::max(0, sampleCount - clipCols);

            int x1 = m_leftMargin + startSample * m_cellWidth;
            int x2 = x1 + clipCols * m_cellWidth;
            int y1 = m_topMargin + topSig * m_rowHeight;
            int y2 = y1 + clipRows * m_rowHeight;

            // Marco del preview
            QColor fill(0, 100, 255, 30);
            QPen pen(QColor(0, 100, 255));
            pen.setStyle(Qt::DashLine);
            pen.setWidth(2);

            p.save();
            p.setPen(pen);
            p.setBrush(fill);
            p.drawRect(QRect(x1, y1, x2 - x1, y2 - y1));
            p.restore();

            // --- Contenido del clipboard: ondas reales ---
            const auto &vals2D = m_doc->blockClipboardValues();
            const auto &labs2D = m_doc->blockClipboardLabels();
            const auto &types2D = m_doc->blockClipboardTypes();
            const auto &colors2D = m_doc->blockClipboardColors();

            int rows = std::min(clipRows, (int)vals2D.size());

            for (int r = 0; r < rows; ++r)
            {
                int destSignalIndex = topSig + r;
                if (destSignalIndex < 0 || destSignalIndex >= sigCount)
                    continue;

                SignalType stype = (r < (int)types2D.size())
                                       ? types2D[r]
                                       : SignalType::Bit;
                QColor baseColor = (r < (int)colors2D.size())
                                       ? colors2D[r]
                                       : QColor(Qt::blue);

                const auto &rowVals = vals2D[r];
                const auto &rowLabs = (r < (int)labs2D.size()) ? labs2D[r]
                                                               : std::vector<QString>();

                int rowTop = m_topMargin + destSignalIndex * m_rowHeight;
                int rowBottom = rowTop + m_rowHeight - 1;

                if (stype == SignalType::Bit)
                {
                    // --- Preview para señal bit ---
                    int highY = rowTop + m_rowHeight * 0.25;
                    int lowY = rowTop + m_rowHeight * 0.75;

                    p.save();
                    QPen bitPen(baseColor);
                    bitPen.setWidth(2);
                    bitPen.setCapStyle(Qt::RoundCap);
                    p.setPen(bitPen);

                    bool havePrev = false;
                    int prevY = lowY;

                    int cols = std::min(clipCols, (int)rowVals.size());

                    for (int c = 0; c < cols; ++c)
                    {
                        int v = rowVals[c];
                        if (v != 0 && v != 1)
                        {
                            havePrev = false;
                            continue;
                        }

                        int globalSample = startSample + c;
                        int x0 = m_leftMargin + globalSample * m_cellWidth;
                        int x1s = x0 + m_cellWidth;

                        int y = (v == 0) ? lowY : highY;

                        if (!havePrev)
                        {
                            prevY = y;
                            havePrev = true;
                        }
                        else
                        {
                            if (y != prevY)
                            {
                                p.drawLine(x0, prevY, x0, y);
                            }
                            prevY = y;
                        }

                        p.drawLine(x0, prevY, x1s, prevY);
                    }

                    p.restore();
                }
                else
                {
                    // --- Preview para señal vector ---
                    p.save();

                    QColor fillColor = baseColor;
                    fillColor.setAlphaF(0.4);
                    QPen vPen(baseColor);
                    vPen.setWidth(2);
                    p.setPen(vPen);

                    int barTop = rowTop + (int)(m_rowHeight * 0.25);
                    int barHeight = (int)(m_rowHeight * 0.5);

                    int cols = std::min(clipCols, (int)rowVals.size());
                    int c = 0;
                    while (c < cols)
                    {
                        int v = rowVals[c];
                        if (v == UNDEFINED_VALUE)
                        {
                            ++c;
                            continue;
                        }

                        int startC = c;
                        int endC = c;
                        for (int k = c + 1; k < cols; ++k)
                        {
                            if (rowVals[k] != v)
                                break;
                            endC = k;
                        }

                        int leftX = m_leftMargin + (startSample + startC) * m_cellWidth;
                        int rightX = m_leftMargin + (startSample + endC + 1) * m_cellWidth;

                        QRect barRect(leftX, barTop, rightX - leftX, barHeight);
                        p.fillRect(barRect, fillColor);
                        p.drawRect(barRect);

                        QString lab;
                        if (startC < (int)rowLabs.size())
                            lab = rowLabs[startC];

                        QString txt = lab.isEmpty()
                                          ? QString::number(v)
                                          : QString("%1 (%2)").arg(lab).arg(v);

                        QFontMetrics fm(p.font());
                        int tw = fm.horizontalAdvance(txt);
                        if (tw < barRect.width())
                        {
                            int lum = qRound(0.299 * fillColor.red() +
                                             0.587 * fillColor.green() +
                                             0.114 * fillColor.blue());
                            QColor txtColor = (lum < 128) ? Qt::white : Qt::black;
                            p.setPen(txtColor);
                            p.drawText(barRect, Qt::AlignCenter, txt);
                            p.setPen(vPen);
                        }

                        c = endC + 1;
                    }

                    p.restore();
                }

                // Separador inferior de la fila (ligero)
                p.save();
                p.setPen(QColor(200, 200, 200, 120));
                p.drawLine(0, rowBottom, width(), rowBottom);
                p.restore();
            }
        }
    }
}

void WaveView::drawSignal(QPainter &p, const Signal &sig, int index)
{
    int top = m_topMargin + index * m_rowHeight;
    int bottom = top + m_rowHeight - 1;

    // Signal name on the left
    QRect nameRect(0, top, m_leftMargin - 5, m_rowHeight);
    p.save();
    // Text color according to background (for export with black background, etc.)
    QColor bg = m_exportBackground.isValid() ? m_exportBackground : palette().base().color();
    int lumBg = qRound(0.299 * bg.red() + 0.587 * bg.green() + 0.114 * bg.blue());
    QColor nameColor = (lumBg < 128) ? Qt::white : Qt::black;
    p.setPen(nameColor);
    p.drawText(nameRect.adjusted(5, 0, -5, 0),
               Qt::AlignVCenter | Qt::AlignLeft,
               sig.name.isEmpty() ? QString("Signal %1").arg(index) : sig.name);
    p.restore();

    // Horizontal separator
    p.setPen(QColor(200, 200, 200));
    p.drawLine(0, bottom, width(), bottom);

    if (sig.values.empty())
        return;

    if (sig.type == SignalType::Bit)
    {
        drawBitSignal(p, sig, index);
    }
    else
    {
        drawVectorSignal(p, sig, index);
    }
}

void WaveView::drawBitSignal(QPainter &p, const Signal &sig, int index)
{
    int sampleCount = m_doc->sampleCount();
    int top = m_topMargin + index * m_rowHeight;

    // Wave levels
    int highY = top + m_rowHeight * 0.25;
    int lowY = top + m_rowHeight * 0.75;

    p.save();

    // Shading with a gradient under the high level (from 70% opacity to 0)
    QLinearGradient grad(0, highY, 0, lowY);
    QColor cTop = sig.color;
    cTop.setAlphaF(0.7);
    QColor cBottom = sig.color;
    cBottom.setAlphaF(0.0);
    grad.setColorAt(0.0, cTop);
    grad.setColorAt(1.0, cBottom);

    const auto &vals = sig.values;
    int runStart = -1;
    for (int t = 0; t < sampleCount; ++t)
    {
        int v = (t < static_cast<int>(vals.size())) ? vals[t] : UNDEFINED_VALUE;
        if (v == 1)
        {
            if (runStart < 0)
                runStart = t;
        }
        else
        {
            if (runStart >= 0)
            {
                int x1 = m_leftMargin + runStart * m_cellWidth;
                int x2 = m_leftMargin + t * m_cellWidth;
                if (x2 > x1)
                {
                    QRect gradRect(x1, highY, x2 - x1, lowY - highY);
                    p.fillRect(gradRect, grad);
                }
                runStart = -1;
            }
        }
    }
    if (runStart >= 0)
    {
        int x1 = m_leftMargin + runStart * m_cellWidth;
        int x2 = m_leftMargin + sampleCount * m_cellWidth;
        if (x2 > x1)
        {
            QRect gradRect(x1, highY, x2 - x1, lowY - highY);
            p.fillRect(gradRect, grad);
        }
    }

    // Now draw the waveform above the shading
    QPen pen(sig.color);
    pen.setWidth(2);
    p.setPen(pen);

    bool havePrev = false;
    int prevY = lowY;

    for (int t = 0; t < sampleCount; ++t)
    {
        int v = (t < static_cast<int>(vals.size())) ? vals[t] : UNDEFINED_VALUE;

        // If the value is undefined, cut the stroke and draw nothing for this sample.
        if (v != 0 && v != 1)
        {
            havePrev = false;
            continue;
        }

        int x0 = m_leftMargin + t * m_cellWidth;
        int x1 = x0 + m_cellWidth;

        int y = (v == 0) ? lowY : highY;

        if (!havePrev)
        {
            prevY = y;
            havePrev = true;
        }
        else
        {
            // vertical transition if value changes
            if (y != prevY)
            {
                p.drawLine(x0, prevY, x0, y);
            }
            prevY = y;
        }

        // horizontal segment for the sample
        p.drawLine(x0, prevY, x1, prevY);
    }

    p.restore();
}

void WaveView::drawVectorSignal(QPainter &p, const Signal &sig, int index)
{
    if (!m_doc)
        return;

    int sampleCount = m_doc->sampleCount();
    int top = m_topMargin + index * m_rowHeight;

    int barTop = top + static_cast<int>(m_rowHeight * 0.25);
    int barHeight = static_cast<int>(m_rowHeight * 0.5);

    p.save();

    QColor baseColor = sig.color;
    QColor fillColor = baseColor;
    fillColor.setAlphaF(0.8);
    QPen pen(baseColor);
    pen.setWidth(2);
    p.setPen(pen);

    const auto &vals = sig.values;
    const auto &labs = sig.labels;

    int triW = std::min(8, std::max(4, m_cellWidth / 3));

    int t = 0;
    while (t < sampleCount)
    {
        int v = (t < static_cast<int>(vals.size())) ? vals[t] : UNDEFINED_VALUE;
        if (v == UNDEFINED_VALUE)
        {
            ++t;
            continue;
        }

        // Group segment of consecutive samples with the same value
        int start = t;
        int end = t;
        for (int k = t + 1; k < sampleCount; ++k)
        {
            int vk = (k < static_cast<int>(vals.size())) ? vals[k] : UNDEFINED_VALUE;
            if (vk != v)
                break;
            end = k;
        }

        // Check if there will be a peak to the left/right
        int prevIndex = start - 1;
        bool hasLeftPeak = false;
        if (prevIndex >= 0)
        {
            int prevV = (prevIndex < static_cast<int>(vals.size())) ? vals[prevIndex] : UNDEFINED_VALUE;
            hasLeftPeak = (prevV != UNDEFINED_VALUE && prevV != v);
        }

        int nextIndex = end + 1;
        bool hasRightPeak = false;
        if (nextIndex < sampleCount)
        {
            int nextV = (nextIndex < static_cast<int>(vals.size())) ? vals[nextIndex] : UNDEFINED_VALUE;
            hasRightPeak = (nextV != UNDEFINED_VALUE && nextV != v);
        }

        // Total edges of the segment
        int leftEdge = m_leftMargin + start * m_cellWidth + 1;
        int rightEdge = m_leftMargin + (end + 1) * m_cellWidth - 1;

        // Effective bar edges, trimming space for peaks
        int barLeft = leftEdge + (hasLeftPeak ? triW : 0);
        int barRight = rightEdge - (hasRightPeak ? triW : 0);
        if (barRight < barLeft)
            barRight = barLeft;

        QRect barRect(barLeft, barTop, barRight - barLeft, barHeight);

        // Fill and border of the bar
        p.fillRect(barRect, fillColor);
        p.drawRect(barRect);

        // Text: label + value (if label exists)
        QString label;
        if (start < static_cast<int>(labs.size()))
            label = labs[start];

        QString text;
        if (label.isEmpty())
            text = QString("%1").arg(label);
        else
            text = QString("%1").arg(label);

        // Choose readable text color over the fill
        int lum = qRound(0.299 * fillColor.red() + 0.587 * fillColor.green() + 0.114 * fillColor.blue());
        QColor textColor = (lum < 128) ? Qt::white : Qt::black;
        p.setPen(textColor);
        p.drawText(barRect, Qt::AlignCenter, text);
        p.setPen(pen);

        int cy = barTop + barHeight / 2;

        // Left side: peak with base on the bar and tip outward
        if (hasLeftPeak)
        {
            int baseX = barLeft;
            int tipX = leftEdge;
            QPolygon triL;
            triL << QPoint(baseX, barTop)
                 << QPoint(tipX, cy)
                 << QPoint(baseX, barTop + barHeight);
            p.setBrush(fillColor);
            p.drawPolygon(triL);
            p.setBrush(Qt::NoBrush);
        }

        // Right side: peak with base on the bar and tip outward
        if (hasRightPeak)
        {
            int baseX = barRight;
            int tipX = rightEdge;
            QPolygon triR;
            triR << QPoint(baseX, barTop)
                 << QPoint(tipX, cy)
                 << QPoint(baseX, barTop + barHeight);
            p.setBrush(fillColor);
            p.drawPolygon(triR);
            p.setBrush(Qt::NoBrush);
        }

        t = end + 1;
    }

    p.restore();
}

void WaveView::drawVectorSelection(QPainter &p)
{
    if (m_mode != Mode::VectorSelecting)
        return;
    if (!m_doc)
        return;

    const auto &sigs = m_doc->signalList();
    if (m_selSignal < 0 || m_selSignal >= static_cast<int>(sigs.size()))
        return;

    int start = std::min(m_selStartSample, m_selCurrentSample);
    int end = std::max(m_selStartSample, m_selCurrentSample);
    if (start < 0 || end < 0 || start >= m_doc->sampleCount())
        return;

    int top = m_topMargin + m_selSignal * m_rowHeight + 4;
    int heightRect = m_rowHeight - 8;
    int x1 = m_leftMargin + start * m_cellWidth + 1;
    int x2 = m_leftMargin + (end + 1) * m_cellWidth - 1;

    QRect selRect(x1, top, x2 - x1, heightRect);
    QColor c(0, 120, 215, 60);
    QPen pen(QColor(0, 120, 215));
    pen.setStyle(Qt::DashLine);

    p.save();
    p.setBrush(c);
    p.setPen(pen);
    p.drawRect(selRect);
    p.restore();
}