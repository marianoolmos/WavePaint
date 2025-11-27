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

static constexpr int UNDEFINED_VALUE = -1;

WaveView::WaveView(WaveDocument *doc, QWidget *parent)
    : QWidget(parent),
      m_doc(doc),
      m_rowHeight(40),
      m_cellWidth(20),
      m_leftMargin(100),
      m_topMargin(20),
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
      m_exportSize(),
      m_exportBackground()
{
    setMouseTracking(true);
    setAutoFillBackground(true);

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

void WaveView::startCutMode()
{
    // Shortcut to activate cut mode from other parts of the UI
    setCutModeEnabled(true);
}

void WaveView::setCutModeEnabled(bool en)
{
    if (en)
    {
        m_mode = Mode::CutSelecting;
        m_cutStartSample = -1;
        m_cutCurrentSample = -1;
    }
    else
    {
        if (m_mode == Mode::CutSelecting)
        {
            m_mode = Mode::None;
        }
        m_cutStartSample = -1;
        m_cutCurrentSample = -1;
    }
    update();
}

void WaveView::setEraseModeEnabled(bool en)
{
    if (en)
    {
        m_mode = Mode::Erasing;
        m_bitPaintSignal = -1;
        m_bitLastSample = -1;
    }
    else
    {
        if (m_mode == Mode::Erasing)
        {
            m_mode = Mode::None;
        }
        m_bitPaintSignal = -1;
        m_bitLastSample = -1;
    }
    update();
}
void WaveView::zoomIn()
{
    // Zoom in: increase cell width
    m_cellWidth = std::min(m_cellWidth + 4, 200);
    updateGeometry();
    update();
}

void WaveView::zoomOut()
{
    // Zoom out: decrease cell width
    m_cellWidth = std::max(m_cellWidth - 4, 4);
    updateGeometry();
    update();
}

bool WaveView::exportToPng(const QString &fileName, const QColor &background)
{
    if (!m_doc)
        return false;
    if (fileName.isEmpty())
        return false;

    const auto &sigs = m_doc->signalList();
    int sampleCount = m_doc->sampleCount();
    int signalCount = static_cast<int>(sigs.size());

    // Minimum dimensions if there are no signals
    if (signalCount == 0)
    {
        signalCount = 1;
    }

    int rightMargin = 20;
    int bottomMargin = 20;

    int contentWidth = m_leftMargin + sampleCount * m_cellWidth + rightMargin;
    int contentHeight = m_topMargin + signalCount * m_rowHeight + bottomMargin;

    QSize sz(contentWidth, contentHeight);

    QImage image(sz, QImage::Format_ARGB32);
    m_exportSize = sz;
    m_exportBackground = background;

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Let paintEvent use m_exportBackground and m_exportSize
    render(&painter);

    // Restore normal export state
    m_exportBackground = QColor(); // invalida => vuelve a usar palette().base()
    m_exportSize = QSize();

    return image.save(fileName);
}

void WaveView::onDocumentChanged()
{
    updateGeometry();
    update();
}

bool WaveView::mapToSignalSample(const QPoint &pos, int &signalIndex, int &sampleIndex) const
{
    if (!m_doc)
        return false;

    int x = pos.x();
    int y = pos.y();

    if (x < m_leftMargin || y < m_topMargin)
        return false;

    int relY = y - m_topMargin;
    signalIndex = relY / m_rowHeight;

    const auto &sigs = m_doc->signalList();
    if (signalIndex < 0 || signalIndex >= static_cast<int>(sigs.size()))
    {
        return false;
    }

    int relX = x - m_leftMargin;
    if (relX < 0)
        return false;

    int sample = relX / m_cellWidth;
    if (sample < 0 || sample >= m_doc->sampleCount())
        return false;

    sampleIndex = sample;
    return true;
}

int WaveView::mapToSignalIndexFromY(int y) const
{
    if (!m_doc)
        return -1;
    if (y < m_topMargin)
        return -1;

    int relY = y - m_topMargin;
    int signalIndex = relY / m_rowHeight;

    const auto &sigs = m_doc->signalList();
    if (signalIndex < 0 || signalIndex >= static_cast<int>(sigs.size()))
    {
        return -1;
    }
    return signalIndex;
}

void WaveView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
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
        p.drawText(x + (m_cellWidth - tw) / 2, m_topMargin - 5, label);
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
    fillColor.setAlphaF(0.6);
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

void WaveView::mousePressEvent(QMouseEvent *event)
{
    if (!m_doc)
    {
        QWidget::mousePressEvent(event);
        return;
    }

    int sigIdx = -1;
    int sampleIdx = -1;

    if (event->button() == Qt::LeftButton)
    {

        // 1) Click izquierdo en zona de nombres -> empezar a arrastrar la señal
        if (event->pos().x() < m_leftMargin)
        {
            int idx = mapToSignalIndexFromY(event->pos().y());
            if (idx >= 0)
            {
                m_isMovingSignal = true;
                m_moveSignalIndex = idx;
                setCursor(Qt::ClosedHandCursor);
                return; // no seguimos con pintura/borra/etc.
            }
        }

        // 2) Modo goma
        if (m_mode == Mode::Erasing)
        {
            if (mapToSignalSample(event->pos(), sigIdx, sampleIdx))
            {
                m_bitPaintSignal = sigIdx;
                m_bitLastSample = sampleIdx;
                m_doc->clearSample(sigIdx, sampleIdx);
            }
            return;
        }

        // 3) Modo tijeras
        if (m_mode == Mode::CutSelecting)
        {
            if (mapToSignalSample(event->pos(), sigIdx, sampleIdx))
            {
                if (m_cutStartSample < 0)
                {
                    m_cutStartSample = sampleIdx;
                    m_cutCurrentSample = sampleIdx;
                    update();
                }
                else
                {
                    m_cutCurrentSample = sampleIdx;
                    update();

                    int s0 = std::min(m_cutStartSample, sampleIdx);
                    int s1 = std::max(m_cutStartSample, sampleIdx);
                    QString msg = tr("Cut waveform from sample %1 to %2?")
                                      .arg(s0)
                                      .arg(s1);
                    auto reply = QMessageBox::question(
                        this,
                        tr("Cut range"),
                        msg,
                        QMessageBox::Yes | QMessageBox::No,
                        QMessageBox::No);
                    if (reply == QMessageBox::Yes)
                    {
                        m_doc->cutRange(s0, s1);
                    }
                    m_cutStartSample = -1;
                    m_cutCurrentSample = -1;
                    update();
                }
            }
            return;
        }

        // 4) Pintar bit / seleccionar vector
        if (mapToSignalSample(event->pos(), sigIdx, sampleIdx))
        {
            const auto &sigs = m_doc->signalList();
            if (sigIdx >= 0 && sigIdx < static_cast<int>(sigs.size()))
            {
                const Signal &sig = sigs[sigIdx];

                if (sig.type == SignalType::Bit)
                {
                    int top = m_topMargin + sigIdx * m_rowHeight;
                    int midY = top + m_rowHeight / 2;
                    int y = event->pos().y();
                    int v = (y < midY) ? 1 : 0;

                    m_mode = Mode::BitPainting;
                    m_bitPaintSignal = sigIdx;
                    m_bitPaintValue = v;
                    m_bitLastSample = sampleIdx;

                    m_doc->setBitValue(sigIdx, sampleIdx, v);
                }
                else
                {
                    m_mode = Mode::VectorSelecting;
                    m_selSignal = sigIdx;
                    m_selStartSample = sampleIdx;
                    m_selCurrentSample = sampleIdx;
                    update();
                }
            }
            return;
        }
    }

    // Botón derecho -> no hacemos nada aquí, lo maneja contextMenuEvent
    QWidget::mousePressEvent(event);
}

void WaveView::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_doc)
    {
        QWidget::mouseMoveEvent(event);
        return;
    }

    // 1) Arrastre de señal (reordenar verticalmente)
    if (m_isMovingSignal && (event->buttons() & Qt::LeftButton))
    {
        int newIdx = mapToSignalIndexFromY(event->pos().y());
        const auto &sigs = m_doc->signalList();

        if (newIdx >= 0 &&
            newIdx < static_cast<int>(sigs.size()) &&
            newIdx != m_moveSignalIndex)
        {

            m_doc->moveSignal(m_moveSignalIndex, newIdx);
            m_moveSignalIndex = newIdx;
        }
        return;
    }

    // 2) Tijeras: actualizar segunda línea roja
    if (m_mode == Mode::CutSelecting && m_cutStartSample >= 0)
    {
        int sigIdx = -1;
        int sampleIdx = -1;
        if (mapToSignalSample(event->pos(), sigIdx, sampleIdx))
        {
            if (sampleIdx != m_cutCurrentSample)
            {
                m_cutCurrentSample = sampleIdx;
                update();
            }
        }
        return;
    }

    // 3) Goma (borrado continuo)
    if (m_mode == Mode::Erasing && (event->buttons() & Qt::LeftButton))
    {
        int sigIdx = -1;
        int sampleIdx = -1;
        if (mapToSignalSample(event->pos(), sigIdx, sampleIdx))
        {
            if (m_bitPaintSignal < 0)
            {
                m_bitPaintSignal = sigIdx;
                m_bitLastSample = sampleIdx;
                m_doc->clearSample(sigIdx, sampleIdx);
            }
            else if (sigIdx == m_bitPaintSignal && sampleIdx != m_bitLastSample)
            {
                int from = std::min(m_bitLastSample, sampleIdx);
                int to = std::max(m_bitLastSample, sampleIdx);
                for (int s = from; s <= to; ++s)
                {
                    m_doc->clearSample(sigIdx, s);
                }
                m_bitLastSample = sampleIdx;
            }
        }
        return;
    }

    // 4) Pintura de bit continua
    if (m_mode == Mode::BitPainting)
    {
        int sigIdx = -1;
        int sampleIdx = -1;
        if (mapToSignalSample(event->pos(), sigIdx, sampleIdx))
        {
            if (sigIdx == m_bitPaintSignal && sampleIdx != m_bitLastSample)
            {
                int from = std::min(m_bitLastSample, sampleIdx);
                int to = std::max(m_bitLastSample, sampleIdx);
                for (int s = from; s <= to; ++s)
                {
                    m_doc->setBitValue(sigIdx, s, m_bitPaintValue);
                }
                m_bitLastSample = sampleIdx;
            }
        }
        return;
    }

    // 5) Selección de vector (rectángulo azul)
    if (m_mode == Mode::VectorSelecting)
    {
        int sigIdx = -1;
        int sampleIdx = -1;
        if (mapToSignalSample(event->pos(), sigIdx, sampleIdx))
        {
            if (sigIdx == m_selSignal)
            {
                m_selCurrentSample = sampleIdx;
                update();
            }
        }
        return;
    }

    QWidget::mouseMoveEvent(event);
}

void WaveView::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_doc)
    {
        QWidget::mouseReleaseEvent(event);
        return;
    }

    // Fin de arrastre de señal
    if (m_isMovingSignal && event->button() == Qt::LeftButton)
    {
        m_isMovingSignal = false;
        m_moveSignalIndex = -1;
        unsetCursor();
        event->accept();
        return;
    }

    if (m_mode == Mode::Erasing && event->button() == Qt::LeftButton)
    {
        m_bitPaintSignal = -1;
        m_bitLastSample = -1;
        return;
    }

    if (m_mode == Mode::BitPainting && event->button() == Qt::LeftButton)
    {
        m_mode = Mode::None;
        m_bitPaintSignal = -1;
        m_bitLastSample = -1;
        return;
    }

    if (m_mode == Mode::VectorSelecting && event->button() == Qt::LeftButton)
    {
        int sigIdx = -1;
        int sampleIdx = -1;
        if (mapToSignalSample(event->pos(), sigIdx, sampleIdx) &&
            sigIdx == m_selSignal)
        {



         

        
    
        int value;
        QString label = QInputDialog::getText(
            this, tr("Vector label"),
            tr("Name or Value:"),
            QLineEdit::Normal,
            QString());
        if (label.isEmpty())
        {
             value = 0;
        }else
        {
             value = label.toInt(0)+1;
        }
 
        m_doc->setVectorRange(m_selSignal, m_selStartSample, sampleIdx, value, label);
                
            
        }

        m_mode = Mode::None;
        m_selSignal = -1;
        m_selStartSample = -1;
        m_selCurrentSample = -1;
        update();
        return;
    }

    QWidget::mouseReleaseEvent(event);
}

void WaveView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!m_doc)
    {
        QWidget::mouseDoubleClickEvent(event);
        return;
    }

    // Doble clic en el nombre de la señal para cambiar el color
    if (event->button() == Qt::LeftButton && event->pos().x() < m_leftMargin)
    {
        int sigIdx = mapToSignalIndexFromY(event->pos().y());
        if (sigIdx >= 0)
        {
            const auto &sigs = m_doc->signalList();
            QColor initial = sigs[sigIdx].color;
            QColor c = QColorDialog::getColor(initial, this, tr("Choose signal color"));
            if (c.isValid())
            {
                m_doc->setSignalColor(sigIdx, c);
            }
            return;
        }
    }

    QWidget::mouseDoubleClickEvent(event);
}

void WaveView::contextMenuEvent(QContextMenuEvent *event)
{
    if (!m_doc)
    {
        QWidget::contextMenuEvent(event);
        return;
    }

    // Right click
    if (event->pos().x() < m_leftMargin)
    {
        int sigIdx = mapToSignalIndexFromY(event->pos().y());
        if (sigIdx >= 0)
        {
            QMenu menu(this);
            QAction *renameAct = menu.addAction(tr("Rename signal..."));
            QAction *colorAct = menu.addAction(tr("Change color..."));
            menu.addSeparator();

            // Copy /Paste
            QAction *copyAct = menu.addAction(tr("Copy signal"));
            QAction *pasteAct = nullptr;
            if (m_doc && m_doc->hasClipboardSignal())
            {
                pasteAct = menu.addAction(tr("Paste signal (duplicate)"));
            }


            menu.addSeparator();
            QAction *deleteAct = menu.addAction(tr("Delete signal"));

            QAction *chosen = menu.exec(event->globalPos());
            if (!chosen)
                return;

            // Rename signal
            if (chosen == renameAct)
            {
                const auto &sigs = m_doc->signalList();
                QString currentName = sigs[sigIdx].name;
                bool ok = false;
                QString newName = QInputDialog::getText(
                    this,
                    tr("Rename signal"),
                    tr("New signal name:"),
                    QLineEdit::Normal,
                    currentName,
                    &ok);
                if (ok && !newName.isEmpty())
                    m_doc->renameSignal(sigIdx, newName);
            }

            // Change color
            else if (chosen == colorAct)
            {
                const auto &sigs = m_doc->signalList();
                QColor initial = sigs[sigIdx].color;
                QColor c = QColorDialog::getColor(initial, this, tr("Choose signal color"));
                if (c.isValid())
                    m_doc->setSignalColor(sigIdx, c);
            }

            // Copy signal
            else if (chosen == copyAct)
            {
                m_doc->copySignal(sigIdx);
            }

            // Paste Signal 
            else if (pasteAct && chosen == pasteAct)
            {
                m_doc->pasteSignal(sigIdx + 1);
            }
            // Remove Signal
            else if (chosen == deleteAct) {

                m_doc->removeSignal(sigIdx);
            }
        }
        return;
    }

    // Waveform zone
    QMenu menu(this);
    QAction *addBitAct = menu.addAction(tr("Add bit signal"));
    QAction *addVectorAct = menu.addAction(tr("Add vector signal"));
    QAction *addClockAct = menu.addAction(tr("Add clock signal"));

    menu.addSeparator();
    QAction *cancelAct = menu.addAction(tr("Cancel"));
    Q_UNUSED(cancelAct);

    QAction *chosen = menu.exec(event->globalPos());
    if (!chosen)
        return;

    if (chosen == addBitAct)
    {
        addBitSignal();
    }
    else if (chosen == addVectorAct)
    {
        addVectorSignal();
    }
    else if (chosen == addClockAct)
    {
        addClockSignal();
    }
}

void WaveView::addBitSignal()
{
    bool ok = false;
    QString name = QInputDialog::getText(this, tr("Add bit signal"),
                                         tr("Signal name:"), QLineEdit::Normal,
                                         tr("bit_signal"), &ok);
    if (!ok || name.isEmpty())
        return;

    m_doc->addBitSignal(name);
}

void WaveView::addVectorSignal()
{
    bool ok = false;
    QString name = QInputDialog::getText(this, tr("Add vector signal"),
                                         tr("Signal name:"), QLineEdit::Normal,
                                         tr("vec_signal"), &ok);
    if (!ok || name.isEmpty())
        return;

    m_doc->addVectorSignal(name);
}

void WaveView::addClockSignal()
{
    bool ok = false;
    QString name = QInputDialog::getText(this, tr("Add clock signal"),
                                         tr("Clock name:"), QLineEdit::Normal,
                                         tr("clk"), &ok);
    if (!ok || name.isEmpty())
        return;

    bool okPulses = false;
    int pulses = QInputDialog::getInt(this, tr("Clock pulses"),
                                      tr("Number of pulses:"), 8, 1, 100000, 1, &okPulses);
    if (!okPulses)
        return;

    bool okHigh = false;
    int highSamples = QInputDialog::getInt(this, tr("High time"),
                                           tr("High time (samples):"), 1, 0, 100000, 1, &okHigh);
    if (!okHigh)
        return;

    bool okLow = false;
    int lowSamples = QInputDialog::getInt(this, tr("Low time"),
                                          tr("Low time (samples):"), 1, 0, 100000, 1, &okLow);
    if (!okLow)
        return;

    m_doc->addClockSignal(name, pulses, highSamples, lowSamples);
}
