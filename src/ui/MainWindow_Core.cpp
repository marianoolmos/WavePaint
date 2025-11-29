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
#include "core/core.h"
#include "MainWindow.h"
#include "WaveView.h"
#include <QToolBar>
#include <QSpinBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QFileDialog>
#include <QInputDialog>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QTreeWidgetItem>
#include <QKeyEvent>
#include <QScrollArea>
#include <QListWidgetItem>
#include <QMap>
#include <QVBoxLayout>
#include <QListWidget>
#include <QTreeWidget>
#include <QSplitter>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_document(this),
      m_waveView(nullptr),
      m_sampleSpin(nullptr),
      m_splitter(nullptr),
      m_hierarchyTree(nullptr),
      m_signalList(nullptr),
      m_waveScroll(nullptr),
      m_cutAction(nullptr),
      m_eraseAction(nullptr),
      m_viewHierarchyAction(nullptr)
{
    createUi();
    createMenus();
    createToolBar();
    m_allActions = {
        m_cutAction,
        m_eraseAction,
        m_addMarkerAction,
        m_subMarkerAction,
        m_arrowAction,
        m_subArrowAction,
        m_selAction
    };
    // Mantener botones de undo/redo actualizados
    statusBar()->showMessage(tr("Ready"));
    QLabel *footerLabel = new QLabel(
        tr("WavePaint by Mariano Olmos (mariano.olmos@outlook.com)"),
        this
    );
    footerLabel->setStyleSheet("color: gray; padding-right: 8px;");
    statusBar()->addPermanentWidget(footerLabel);

    connect(&m_document, &WaveDocument::undoRedoStateChanged,
            this, &MainWindow::updateUndoRedoActions);

    updateUndoRedoActions(); // estado inicial
}

void MainWindow::createUi()
{
    // Main splitter: left hierarchy, right waveform
    m_splitter = new QSplitter(this);
    m_splitter->setOrientation(Qt::Horizontal);

    // Left panel for VCD hierarchy and signal list
    QWidget *leftPanel = new QWidget(m_splitter);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(2, 2, 2, 2);
    leftLayout->setSpacing(2);

    m_hierarchyTree = new QTreeWidget(leftPanel);
    m_hierarchyTree->setHeaderLabel(tr("Hierarchy"));
    leftLayout->addWidget(m_hierarchyTree, 2);

    m_signalList = new QListWidget(leftPanel);
    leftLayout->addWidget(m_signalList, 1);

    leftPanel->setLayout(leftLayout);
    m_splitter->addWidget(leftPanel);

    // Waveform on the right inside a QScrollArea to allow scrolling and zooming
    m_waveScroll = new QScrollArea(m_splitter);
    m_waveScroll->setWidgetResizable(true);
    m_waveScroll->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    m_waveView = new WaveView(&m_document, m_waveScroll);
    m_waveScroll->setWidget(m_waveView);

    m_splitter->addWidget(m_waveScroll);

    setCentralWidget(m_splitter);
    setWindowTitle(tr("WavePaint"));
    resize(1200, 700);

    // By default,dont show hierarchy on the left.
    QList<int> sizes;
    sizes << 0 << width(); // izquierda = 0, derecha = todo
    m_splitter->setSizes(sizes);

    // Connect hierarchy selection to the signal list
    connect(m_hierarchyTree, &QTreeWidget::currentItemChanged,
            this, &MainWindow::onHierarchySelectionChanged);

    // Double-click a signal in the list => add it to the waveform
    connect(m_signalList, &QListWidget::itemDoubleClicked,
            this, &MainWindow::onSignalDoubleClicked);
}

void MainWindow::activateXDesactivateAll(const QList<QAction*> &actions, QAction *except)
{
    for (QAction *act : actions)
    {
        if (!act || act == except)
            continue;

        act->blockSignals(true);
        act->setChecked(false);
        act->blockSignals(false);
    }
}

void MainWindow::onAddMarkerToggled(bool enabled)
{
    if (!m_waveView)
        return;

    if (enabled)
    {
    activateXDesactivateAll(m_allActions, m_addMarkerAction);
    m_waveView->setCutModeEnabled(false);
    m_waveView->setEraseModeEnabled(false);
    m_waveView->setMarkerSubModeEnabled(false);
    m_waveView->setMarkerAddModeEnabled(true);
    m_waveView->setArrowModeEnabled(false);
    m_waveView->setArrowSubModeEnabled(false);
    m_waveView->setSelectionModeEnabled(false);
    }
    else
    {
        m_waveView->setMarkerAddModeEnabled(false);
    }
}

void MainWindow::onSubMarkerToggled(bool enabled)
{
    if (!m_waveView)
        return;

    if (enabled)
    {
        activateXDesactivateAll(m_allActions, m_subMarkerAction);
        m_waveView->setCutModeEnabled(false);
        m_waveView->setEraseModeEnabled(false);
        m_waveView->setMarkerSubModeEnabled(true);
        m_waveView->setMarkerAddModeEnabled(false);
        m_waveView->setArrowModeEnabled(false);
        m_waveView->setArrowSubModeEnabled(false);
        m_waveView->setSelectionModeEnabled(false);
    }
    else
    {
        m_waveView->setMarkerSubModeEnabled(false);
    }
}
void MainWindow::onArrowToggled(bool enabled)
{
    if (!m_waveView)
        return;

    if (enabled)
    {
        activateXDesactivateAll(m_allActions, m_arrowAction);
        m_waveView->setCutModeEnabled(false);
        m_waveView->setEraseModeEnabled(false);
        m_waveView->setMarkerSubModeEnabled(false);
        m_waveView->setMarkerAddModeEnabled(false);
        m_waveView->setArrowModeEnabled(true);
        m_waveView->setArrowSubModeEnabled(false);
        m_waveView->setSelectionModeEnabled(false);
    }
    else
    {
        m_waveView->setArrowModeEnabled(false);
    }
}
void MainWindow::onSubArrowToggled(bool enabled)
{
    if (!m_waveView)
        return;

    if (enabled)
    {
        activateXDesactivateAll(m_allActions, m_subArrowAction);
        m_waveView->setCutModeEnabled(false);
        m_waveView->setEraseModeEnabled(false);
        m_waveView->setMarkerSubModeEnabled(true);
        m_waveView->setMarkerAddModeEnabled(false);
        m_waveView->setArrowModeEnabled(false);
        m_waveView->setArrowSubModeEnabled(true);
        m_waveView->setSelectionModeEnabled(false);
    }
    else
    {
        m_waveView->setArrowSubModeEnabled(false);
    }
}
void MainWindow::onSelectBlockToggled(bool enabled)
{
    if (!m_waveView)
        return;

    auto uncheck = [](QAction *act)
    {
        if (!act)
            return;
        act->blockSignals(true);
        act->setChecked(false);
        act->blockSignals(false);
    };

    if (enabled)
    {
        activateXDesactivateAll(m_allActions, m_selAction);
        m_waveView->setCutModeEnabled(false);
        m_waveView->setEraseModeEnabled(false);
        m_waveView->setMarkerSubModeEnabled(true);
        m_waveView->setMarkerAddModeEnabled(false);
        m_waveView->setArrowModeEnabled(false);
        m_waveView->setArrowSubModeEnabled(false);
        m_waveView->setSelectionModeEnabled(true);
    }
    else
    {
        m_waveView->setSelectionModeEnabled(false);
    }
}
void MainWindow::onCutToggled(bool enabled)
{
    if (!m_waveView)
        return;

    if (enabled)
    {
        activateXDesactivateAll(m_allActions, m_cutAction);
        m_waveView->setCutModeEnabled(true);
        m_waveView->setEraseModeEnabled(false);
        m_waveView->setMarkerSubModeEnabled(false);
        m_waveView->setMarkerAddModeEnabled(false);
        m_waveView->setArrowModeEnabled(false);
        m_waveView->setArrowSubModeEnabled(false);
        m_waveView->setSelectionModeEnabled(false);
    }
    else
    {
        m_waveView->setCutModeEnabled(false);
    }
}

void MainWindow::onEraseToggled(bool enabled)
{
    if (!m_waveView)
        return;

    if (enabled)
    {
        activateXDesactivateAll(m_allActions, m_eraseAction);
        m_waveView->setCutModeEnabled(false);
        m_waveView->setEraseModeEnabled(true);
        m_waveView->setMarkerSubModeEnabled(false);
        m_waveView->setMarkerAddModeEnabled(false);
        m_waveView->setArrowModeEnabled(false);
        m_waveView->setArrowSubModeEnabled(false);
        m_waveView->setSelectionModeEnabled(false);
    }
    else
    {
        m_waveView->setEraseModeEnabled(false);
    }
}
void MainWindow::onUndo()
{
    m_document.undo();
}

void MainWindow::onRedo()
{
    m_document.redo();
}

void MainWindow::updateUndoRedoActions()
{
    if (!m_undoAction || !m_redoAction)
        return;

    m_undoAction->setEnabled(m_document.canUndo());
    m_redoAction->setEnabled(m_document.canRedo());
}

void MainWindow::onHierarchySelectionChanged(QTreeWidgetItem *current, QTreeWidgetItem * /*previous*/)
{
    if (!m_hierarchyTree || !m_signalList)
        return;

    m_signalList->clear();
    if (!current)
        return;

    // Rebuild the selected module path (concatenate texts up to the root)
    QStringList chain;
    QTreeWidgetItem *it = current;
    while (it)
    {
        chain.prepend(it->text(0));
        it = it->parent();
    }
    QString modulePath = chain.join('.');

    const auto &sigs = m_document.vcdSignalList();
    for (const Signal &s : sigs)
    {
        QString fullName = s.name;
        if (fullName.isEmpty())
            continue;

        QStringList parts = fullName.split('.');
        if (parts.size() <= 1)
            continue;

        QString leaf = parts.last();
        parts.removeLast();
        QString sigModulePath = parts.join('.');

        if (sigModulePath == modulePath)
        {
            QListWidgetItem *item = new QListWidgetItem(leaf, m_signalList);
            item->setToolTip(fullName);
        }
    }
}

void MainWindow::onSignalDoubleClicked(QListWidgetItem *item)
{
    if (!item)
        return;
    if (!m_hierarchyTree)
        return;

    // Get the currently selected module in the hierarchy
    QTreeWidgetItem *current = m_hierarchyTree->currentItem();
    if (!current)
        return;

    QStringList chain;
    QTreeWidgetItem *it = current;
    while (it)
    {
        chain.prepend(it->text(0));
        it = it->parent();
    }
    QString modulePath = chain.join('.');

    QString leafName = item->text();
    QString fullName = modulePath.isEmpty() ? leafName : (modulePath + "." + leafName);

    // Add signal visible from the VCD library
    int idx = m_document.addSignalFromVcd(fullName);
    if (idx < 0)
    {
        statusBar()->showMessage(tr("Signal %1 not found in VCD library").arg(fullName), 3000);
    }
    else
    {
        statusBar()->showMessage(tr("Added signal %1").arg(fullName), 2000);
    }
}


void MainWindow::cancelModes()
{
    if (!m_waveView)
        return;

    if (m_cutAction)
    {
        m_cutAction->blockSignals(true);
        m_cutAction->setChecked(false);
        m_cutAction->blockSignals(false);
    }
    if (m_eraseAction)
    {
        m_eraseAction->blockSignals(true);
        m_eraseAction->setChecked(false);
        m_eraseAction->blockSignals(false);
    }
        if (m_arrowAction)
    {
        m_arrowAction->blockSignals(true);
        m_arrowAction->setChecked(false);
        m_arrowAction->blockSignals(false);
    }
        if (m_subArrowAction)
    {
        m_subArrowAction->blockSignals(true);
        m_subArrowAction->setChecked(false);
        m_subArrowAction->blockSignals(false);
    }
        if (m_subMarkerAction)
    {
        m_subMarkerAction->blockSignals(true);
        m_subMarkerAction->setChecked(false);
        m_subMarkerAction->blockSignals(false);
    }
        if (m_addMarkerAction)
    {
        m_addMarkerAction->blockSignals(true);
        m_addMarkerAction->setChecked(false);
        m_addMarkerAction->blockSignals(false);
    }

     if (m_selAction)
    {
        m_selAction->blockSignals(true);
        m_selAction->setChecked(false);
        m_selAction->blockSignals(false);
    }
    m_waveView->setCutModeEnabled(false);
    m_waveView->setEraseModeEnabled(false);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        cancelModes();
        event->accept();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

void WaveDocument::moveSignal(int fromIndex, int toIndex)
{
    int n = static_cast<int>(m_signals.size());
    if (fromIndex < 0 || fromIndex >= n ||
        toIndex < 0 || toIndex >= n ||
        fromIndex == toIndex)
    {
        return;
    }

    // 1) Mover la señal en el vector m_signals
    Signal sig = m_signals[fromIndex];
    m_signals.erase(m_signals.begin() + fromIndex);
    m_signals.insert(m_signals.begin() + toIndex, sig);

    // 2) Actualizar los índices de las flechas
    if (!m_arrows.empty())
    {
        for (Arrow &a : m_arrows)
        {

            auto updateIndex = [&](int idx) -> int
            {
                if (fromIndex < toIndex)
                {
                    // Ej: [0 1 2 3 4], move 1 -> 3
                    // index 1 pasa a 3
                    // 2 y 3 bajan a 1 y 2
                    if (idx == fromIndex)
                        return toIndex;
                    if (idx > fromIndex && idx <= toIndex)
                        return idx - 1;
                    return idx;
                }
                else
                {
                    // fromIndex > toIndex
                    // Ej: [0 1 2 3 4], move 3 -> 1
                    // index 3 pasa a 1
                    // 1 y 2 suben a 2 y 3
                    if (idx == fromIndex)
                        return toIndex;
                    if (idx >= toIndex && idx < fromIndex)
                        return idx + 1;
                    return idx;
                }
            };

            a.startSignal = updateIndex(a.startSignal);
            a.endSignal = updateIndex(a.endSignal);
        }
    }

    emit dataChanged();
}
