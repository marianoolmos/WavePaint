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

static constexpr int UNDEFINED_VALUE = -1;

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
void WaveView::mousePressEvent(QMouseEvent *event)
{
    if (!m_doc)
    {
        QWidget::mousePressEvent(event);
        return;
    }

    int sigIdx = -1;
    int sampleIdx = -1;

    if (event->button() == Qt::LeftButton && m_selectionModeEnabled)
    {
        if (mapToSignalSample(event->pos(), sigIdx, sampleIdx))
        {
            // Empezamos SIEMPRE una nueva selección
            m_blockSelecting = true;
            m_blockSelectionActive = true;
            m_blockSelStartSignal = sigIdx;
            m_blockSelEndSignal = sigIdx;
            m_blockSelStartSample = sampleIdx;
            m_blockSelEndSample = sampleIdx;

            // Mientras estás definiendo selección, NO hay preview de pegado
            m_blockPastePreviewActive = false;
            update();
        }
        else
        {
            // Click fuera de la zona de waveform -> cancelamos selección y preview
            m_blockSelecting = false;
            m_blockSelectionActive = false;
            m_blockPastePreviewActive = false;
            update();
        }
        return;
    }

    // SOLO gestionamos aquí el botón izquierdo
    if (event->button() == Qt::LeftButton)
    {

        // 1) Click en la zona de nombres -> empezar a mover señal
        if (event->pos().x() < m_leftMargin)
        {
            int idx = mapToSignalIndexFromY(event->pos().y());
            if (idx >= 0)
            {
                m_isMovingSignal = true;
                m_moveSignalIndex = idx;
                setCursor(Qt::ClosedHandCursor);
                return; // no seguimos con pintura ni nada más
            }
        }
        // 2) Modo borrar flecha (ArrowDelete):
        //    un clic cerca de una flecha borra SOLO esa flecha
        if (m_mode == Mode::ArrowSub)
        {
            const auto &arrows = m_doc->arrowList();
            if (!arrows.empty())
            {
                QPointF click = event->pos();
                int bestIndex = -1;
                qreal bestDist2 = (qreal)(m_cellWidth * m_cellWidth); // umbral máximo

                for (int i = 0; i < static_cast<int>(arrows.size()); ++i)
                {
                    const Arrow &a = arrows[i];
                    QPointF p1 = signalSampleToPoint(a.startSignal, a.startSample);
                    QPointF p2 = signalSampleToPoint(a.endSignal, a.endSample);

                    // Aproximamos la curva por el segmento recto p1-p2
                    QLineF seg(p1, p2);
                    if (seg.length() <= 0.1)
                        continue;

                    // Proyección del click en el segmento
                    QPointF v = p2 - p1;
                    QPointF w = click - p1;
                    qreal c1 = QPointF::dotProduct(w, v);
                    qreal c2 = QPointF::dotProduct(v, v);
                    qreal t = (c2 > 0.0) ? (c1 / c2) : 0.0;
                    t = std::max<qreal>(0.0, std::min<qreal>(1.0, t));

                    QPointF proj = p1 + v * t;
                    qreal dx = click.x() - proj.x();
                    qreal dy = click.y() - proj.y();
                    qreal dist2 = dx * dx + dy * dy;

                    if (dist2 < bestDist2)
                    {
                        bestDist2 = dist2;
                        bestIndex = i;
                    }
                }

                if (bestIndex >= 0)
                {
                    m_doc->subArrowById(arrows[bestIndex].id);
                    return; // ya hemos borrado una flecha
                }
            }

            // Si no había flecha cerca, no hacemos nada más en este modo
            return;
        }

        // 2) Modo flecha: dos clics definen origen y destino
        if (m_mode == Mode::ArrowAdd)
        {
            if (mapToSignalSample(event->pos(), sigIdx, sampleIdx))
            {

                if (!m_arrowHasStart)
                {
                    // Primer punto: origen
                    m_arrowHasStart = true;
                    m_arrowStartSignal = sigIdx;
                    m_arrowStartSample = sampleIdx;
                    m_arrowPreviewSignal = sigIdx;
                    m_arrowPreviewSample = sampleIdx;
                    update();
                }
                else
                {
                    // Segundo punto: destino -> crear flecha
                    m_doc->addArrow(m_arrowStartSignal, m_arrowStartSample,
                                    sigIdx, sampleIdx);

                    m_arrowHasStart = false;
                    m_arrowStartSignal = -1;
                    m_arrowStartSample = -1;
                    m_arrowPreviewSignal = -1;
                    m_arrowPreviewSample = -1;
                    update();
                }
            }
            return;
        }

        // 2) Modos de MARCADORES (añadir / borrar)
        if (m_mode == Mode::MarkerAdd || m_mode == Mode::MarkerSub)
        {
            // Nos basta con conocer el sample, la señal da igual
            if (mapToSignalSample(event->pos(), sigIdx, sampleIdx))
            {

                if (m_mode == Mode::MarkerAdd)
                {
                    // Añadir marcador en este sample
                    m_doc->addMarker(sampleIdx);
                }
                else
                { // Mode::MarkerDelete
                    const auto &markers = m_doc->markerList();
                    if (!markers.empty())
                    {
                        int bestIndex = -1;
                        int bestDist = INT_MAX;

                        int clickX = event->pos().x();

                        for (int i = 0; i < static_cast<int>(markers.size()); ++i)
                        {
                            int mx = m_leftMargin + markers[i].sample * m_cellWidth;

                            int dist = mx - clickX;
                            if (dist < 0)
                                dist = -dist;

                            if (dist < bestDist)
                            {
                                bestDist = dist;
                                bestIndex = i;
                            }
                        }

                        // Si clicas razonablemente cerca (hasta una celda de ancho)
                        if (bestIndex >= 0 && bestDist <= m_cellWidth)
                        {
                            int id = markers[bestIndex].id;
                            m_doc->subMarkerById(id);
                        }
                    }
                }
            }
            return;
        }

        // 3) Modo goma (eraser)
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

        // 4) Modo tijeras (cut)
        if (m_mode == Mode::CutSelecting)
        {
            if (mapToSignalSample(event->pos(), sigIdx, sampleIdx))
            {
                if (m_cutStartSample < 0)
                {
                    // Primer punto
                    m_cutStartSample = sampleIdx;
                    m_cutCurrentSample = sampleIdx;
                    update();
                }
                else
                {
                    // Segundo punto
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

                    // Reseteamos pero mantenemos el modo tijeras
                    m_cutStartSample = -1;
                    m_cutCurrentSample = -1;
                    update();
                }
            }
            return;
        }

        // 5) Pintar bit o seleccionar vector
        if (mapToSignalSample(event->pos(), sigIdx, sampleIdx))
        {
            const auto &sigs = m_doc->signalList();
            if (sigIdx >= 0 && sigIdx < static_cast<int>(sigs.size()))
            {
                const Signal &sig = sigs[sigIdx];

                if (sig.type == SignalType::Bit)
                {
                    // Pintura continua: parte superior = 1, inferior = 0
                    int top = m_topMargin + sigIdx * m_rowHeight;
                    int midY = top + m_rowHeight / 2;
                    int y = event->pos().y();
                    int v = (y < midY) ? 1 : 0;

                    m_mode = Mode::BitPainting;
                    m_bitPaintSignal = sigIdx;
                    m_bitPaintValue = v;
                    m_bitLastSample = sampleIdx;

                    m_doc->setBitValue(sigIdx, sampleIdx, v);
                    return;
                }
                else
                {
                    // Vector: empezamos selección de rango
                    m_mode = Mode::VectorSelecting;
                    m_selSignal = sigIdx;
                    m_selStartSample = sampleIdx;
                    m_selCurrentSample = sampleIdx;
                    update();
                    return;
                }
            }
        }
    }

    // El botón derecho no se gestiona aquí, lo maneja contextMenuEvent
    QWidget::mousePressEvent(event);
}

void WaveView::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_doc)
    {
        QWidget::mouseMoveEvent(event);
        return;
    }

    if (m_selectionModeEnabled)
    {

        // 1) Si estás arrastrando con el botón izquierdo, actualizamos la selección
        if (m_blockSelecting && (event->buttons() & Qt::LeftButton))
        {
            int sig, samp;
            if (mapToSignalSample(event->pos(), sig, samp))
            {
                m_blockSelEndSignal = sig;
                m_blockSelEndSample = samp;
                update();
            }
            return;
        }

        // 2) Solo movemos el PREVIEW si YA está activo
        //    (es decir, después de Ctrl+C o Ctrl+X)
        if (m_blockPastePreviewActive && m_doc->hasBlockClipboard())
        {
            int sig, samp;
            if (mapToSignalSample(event->pos(), sig, samp))
            {
                const auto &sigs = m_doc->signalList();
                int sigCount = static_cast<int>(sigs.size());
                int sampleCount = m_doc->sampleCount();
                int clipRows = m_doc->blockClipboardSignalCount();
                int clipCols = m_doc->blockClipboardSampleCount();

                int destSignal = sig;
                int destSample = samp;

                // Que quepa el bloque dentro del documento
                if (destSignal > sigCount - clipRows)
                    destSignal = std::max(0, sigCount - clipRows);
                if (destSample > sampleCount - clipCols)
                    destSample = std::max(0, sampleCount - clipCols);
                if (destSignal < 0)
                    destSignal = 0;
                if (destSample < 0)
                    destSample = 0;

                if (destSignal != m_blockPasteSignal ||
                    destSample != m_blockPasteSample)
                {

                    m_blockPasteSignal = destSignal;
                    m_blockPasteSample = destSample;
                    update();
                }
            }
            else
            {
                // Ratón fuera de la zona de señales -> ocultamos el preview
                m_blockPastePreviewActive = false;
                update();
            }
            return;
        }

        // Si estás en modo selección pero sin arrastrar ni preview, no tocamos nada más
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
    // 2) Modo ArrowAdd: mover segundo punto de la flecha (previsualización)
    if (m_mode == Mode::ArrowAdd && m_arrowHasStart)
    {
        int sigIdx = -1;
        int sampleIdx = -1;
        if (mapToSignalSample(event->pos(), sigIdx, sampleIdx))
        {
            if (sigIdx != m_arrowPreviewSignal || sampleIdx != m_arrowPreviewSample)
            {
                m_arrowPreviewSignal = sigIdx;
                m_arrowPreviewSample = sampleIdx;
                update();
            }
        }
        return;
    }

    // 2) Modo MarkerAdd: solo previsualizamos,
    //    no añadimos hasta el click (en mousePressEvent)
    if (m_mode == Mode::MarkerAdd)
    {
        int sigIdx = -1;
        int sampleIdx = -1;

        if (mapToSignalSample(event->pos(), sigIdx, sampleIdx))
        {
            if (sampleIdx != m_markerPreviewSample)
            {
                m_markerPreviewSample = sampleIdx;
                update();
            }
        }
        else
        {
            if (m_markerPreviewSample != -1)
            {
                m_markerPreviewSample = -1;
                update();
            }
        }
        // No return: si quieres que en MarkerAdd solo se vea la línea,
        // puedes hacer 'return;' aquí. Si no, deja seguir.
        return;
    }

    // 3) Tijeras: actualizar segunda línea roja
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

    // 4) Goma (borrado continuo)
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

    // 5) Pintura de bit continua
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

    // 6) Selección de vector (rectángulo azul)
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

    if (m_selectionModeEnabled && event->button() == Qt::LeftButton)
    {
        if (m_blockSelecting)
        {
            m_blockSelecting = false;
            update();
        }
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
            }
            else
            {
                value = label.toInt(0) + 1;
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

    if (m_selectionModeEnabled)
    {
        QMenu menu(this);

        QAction *cancelAct = nullptr;
        QAction *pasteAct = nullptr;

        // Opción para cancelar selección / preview
        if (m_blockSelectionActive || m_blockPastePreviewActive)
        {
            cancelAct = menu.addAction(tr("Cancel current selection"));
            menu.addSeparator();
        }

        // Opción para pegar bloque con botón derecho
        int sigIdx = -1, sampleIdx = -1;
        if (m_doc->hasBlockClipboard() &&
            mapToSignalSample(event->pos(), sigIdx, sampleIdx))
        {

            pasteAct = menu.addAction(tr("Paste block here"));
        }

        QAction *chosen = menu.exec(event->globalPos());
        if (!chosen)
            return;

        // --- CANCELAR SELECCIÓN ---
        if (chosen == cancelAct)
        {
            m_blockSelectionActive = false;
            m_blockSelecting = false;
            m_blockPastePreviewActive = false;

            m_blockSelStartSignal = -1;
            m_blockSelEndSignal = -1;
            m_blockSelStartSample = -1;
            m_blockSelEndSample = -1;

            update();
            return;
        }

        // --- PEGAR BLOQUE CON CLICK DERECHO ---
        if (pasteAct && chosen == pasteAct)
        {
            int clipRows = m_doc->blockClipboardSignalCount();
            int clipCols = m_doc->blockClipboardSampleCount();

            const auto &sigs = m_doc->signalList();
            int sigCount = static_cast<int>(sigs.size());
            int sampleCount = m_doc->sampleCount();

            if (sigCount > 0 && sampleCount > 0 &&
                clipRows > 0 && clipCols > 0)
            {

                int destSignal = sigIdx;
                int destSample = sampleIdx;

                if (destSignal > sigCount - clipRows)
                    destSignal = std::max(0, sigCount - clipRows);
                if (destSample > sampleCount - clipCols)
                    destSample = std::max(0, sampleCount - clipCols);
                if (destSignal < 0)
                    destSignal = 0;
                if (destSample < 0)
                    destSample = 0;

                m_doc->pasteBlock(destSignal, destSample);

                // Actualizamos selección al bloque pegado
                m_blockSelectionActive = true;
                m_blockSelecting = false;
                m_blockSelStartSignal = destSignal;
                m_blockSelEndSignal = destSignal + clipRows - 1;
                m_blockSelStartSample = destSample;
                m_blockSelEndSample = destSample + clipCols - 1;

                // El preview ya no es necesario justo después de pegar con botón derecho
                m_blockPastePreviewActive = false;

                update();
            }
            return;
        }

        // Si se eligió algo más en el futuro...
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
            else if (chosen == deleteAct)
            {

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
void WaveView::setMarkerAddModeEnabled(bool en)
{
    if (en)
    {
        m_mode = Mode::MarkerAdd;
    }
    else if (m_mode == Mode::MarkerAdd)
    {
        m_mode = Mode::None;
    }
    update();
}

void WaveView::setMarkerSubModeEnabled(bool en)
{
    if (en)
    {
        m_mode = Mode::MarkerSub;
    }
    else if (m_mode == Mode::MarkerSub)
    {
        m_mode = Mode::None;
    }
    m_markerPreviewSample = -1;
    update();
}

void WaveView::setArrowModeEnabled(bool en)
{
    if (en)
    {
        m_mode = Mode::ArrowAdd;
    }
    else if (m_mode == Mode::ArrowAdd)
    {
        m_mode = Mode::None;
    }

    m_arrowHasStart = false;
    m_arrowStartSignal = -1;
    m_arrowStartSample = -1;
    m_arrowPreviewSignal = -1;
    m_arrowPreviewSample = -1;

    update();
}
QPointF WaveView::signalSampleToPoint(int signalIndex, int sampleIndex) const
{
    // X en el INICIO del timestep (coincide con la línea vertical de la grid)
    qreal x = m_leftMargin + sampleIndex * m_cellWidth;

    // Y en el centro de la fila de la señal
    qreal y = m_topMargin + (signalIndex + 0.5) * m_rowHeight;

    return QPointF(x, y);
}
void WaveView::setArrowSubModeEnabled(bool en)
{
    if (en)
    {
        m_mode = Mode::ArrowSub;
    }
    else if (m_mode == Mode::ArrowSub)
    {
        m_mode = Mode::None;
    }
    // No necesitamos estado especial como en ArrowAdd
    update();
}
void WaveView::setSelectionModeEnabled(bool en)
{
    m_selectionModeEnabled = en;
    m_blockSelecting = false;
    m_blockPastePreviewActive = false;
    if (!en)
    {
        m_blockSelectionActive = false;
        m_blockSelStartSignal = -1;
        m_blockSelEndSignal = -1;
        m_blockSelStartSample = -1;
        m_blockSelEndSample = -1;
    }
    update();
}

bool WaveView::normalizedBlockSelection(int &topSignal, int &bottomSignal,
                                        int &startSample, int &endSample) const
{
    if (!m_blockSelectionActive || !m_doc)
        return false;

    const auto &sigs = m_doc->signalList();
    if (sigs.empty())
        return false;

    int sigCount = static_cast<int>(sigs.size());
    int sampleCount = m_doc->sampleCount();

    int tSig = std::min(m_blockSelStartSignal, m_blockSelEndSignal);
    int bSig = std::max(m_blockSelStartSignal, m_blockSelEndSignal);
    int sS = std::min(m_blockSelStartSample, m_blockSelEndSample);
    int eS = std::max(m_blockSelStartSample, m_blockSelEndSample);

    if (tSig < 0 || tSig >= sigCount ||
        bSig < 0 || bSig >= sigCount)
        return false;
    if (sS < 0 || sS >= sampleCount ||
        eS < 0 || eS >= sampleCount)
        return false;

    topSignal = tSig;
    bottomSignal = bSig;
    startSample = sS;
    endSample = eS;
    return true;
}

bool WaveView::pointInBlockSelection(int signalIndex, int sampleIndex) const
{
    int top, bottom, start, end;
    if (!normalizedBlockSelection(top, bottom, start, end))
        return false;

    if (signalIndex < top || signalIndex > bottom)
        return false;
    if (sampleIndex < start || sampleIndex > end)
        return false;

    return true;
}
void WaveView::keyPressEvent(QKeyEvent *event)
{
    if (!m_doc)
    {
        QWidget::keyPressEvent(event);
        return;
    }

    if (m_selectionModeEnabled)
    {
        int top, bottom, start, end;

        // --- Ctrl + C: COPIAR bloque y activar preview ---
        if (event->matches(QKeySequence::Copy))
        {
            if (normalizedBlockSelection(top, bottom, start, end))
            {
                m_doc->copyBlock(top, bottom, start, end);

                // Ahora empezamos la previsualización desde la esquina de la selección
                m_blockPasteSignal = top;
                m_blockPasteSample = start;
                m_blockPastePreviewActive = true;
                update();
            }
            event->accept();
            return;
        }

        // --- Ctrl + X: CORTAR bloque (copiar + borrar) y activar preview ---
        if (event->matches(QKeySequence::Cut))
        {
            if (normalizedBlockSelection(top, bottom, start, end))
            {
                m_doc->copyBlock(top, bottom, start, end);
                m_doc->clearBlock(top, bottom, start, end);

                m_blockPasteSignal = top;
                m_blockPasteSample = start;
                m_blockPastePreviewActive = true;
                update();
            }
            event->accept();
            return;
        }

        // --- Ctrl + V: PEGAR en la posición del preview ---
        if (event->matches(QKeySequence::Paste))
        {
            if (m_doc->hasBlockClipboard())
            {
                const auto &sigs = m_doc->signalList();
                int sigCount = static_cast<int>(sigs.size());
                int sampleCount = m_doc->sampleCount();
                int clipRows = m_doc->blockClipboardSignalCount();
                int clipCols = m_doc->blockClipboardSampleCount();

                if (sigCount > 0 && sampleCount > 0 &&
                    clipRows > 0 && clipCols > 0)
                {

                    int destSignal = m_blockPastePreviewActive ? m_blockPasteSignal : 0;
                    int destSample = m_blockPastePreviewActive ? m_blockPasteSample : 0;

                    if (destSignal > sigCount - clipRows)
                        destSignal = std::max(0, sigCount - clipRows);
                    if (destSample > sampleCount - clipCols)
                        destSample = std::max(0, sampleCount - clipCols);
                    if (destSignal < 0)
                        destSignal = 0;
                    if (destSample < 0)
                        destSample = 0;

                    m_doc->pasteBlock(destSignal, destSample);

                    // La selección pasa a ser el bloque pegado
                    m_blockSelectionActive = true;
                    m_blockSelecting = false;
                    m_blockSelStartSignal = destSignal;
                    m_blockSelEndSignal = destSignal + clipRows - 1;
                    m_blockSelStartSample = destSample;
                    m_blockSelEndSample = destSample + clipCols - 1;

                    update();
                }
            }
            event->accept();
            return;
        }
    }

    QWidget::keyPressEvent(event);
}
