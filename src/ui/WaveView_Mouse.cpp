
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
