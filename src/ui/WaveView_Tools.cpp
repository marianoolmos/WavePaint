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

