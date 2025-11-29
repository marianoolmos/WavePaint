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

void MainWindow::createToolBar()
{


    QToolBar *tb = addToolBar(tr("Timeline"));
    tb->setMovable(false);

    // Undo / redo (Actions)
    m_undoAction = tb->addAction(QString::fromUtf8("↶"));
    m_undoAction->setToolTip(tr("Undo last action"));
    m_undoAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z));


    m_redoAction = tb->addAction(QString::fromUtf8("↷"));
    m_redoAction->setToolTip(tr("Redo last undone action"));
    m_redoAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));

    tb->addSeparator();

    // Zoom out / zoom in buttons
    QAction *zoomOutAct = tb->addAction(QStringLiteral("-"));
    zoomOutAct->setToolTip(tr("Zoom out (less detail)"));
    QAction *zoomInAct = tb->addAction(QStringLiteral("+"));
    zoomInAct->setToolTip(tr("Zoom in (more detail)"));

    tb->addSeparator();

    // Scissors (cut) button with emoji
    m_cutAction = tb->addAction(QString::fromUtf8("✂"));
    m_cutAction->setToolTip(tr("Cut range: select two points to keep only that interval"));
    m_cutAction->setCheckable(true);
    m_cutAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));

    // Eraser button
    m_eraseAction = tb->addAction(QString::fromUtf8("x"));
    m_eraseAction->setToolTip(tr("Eraser: click/drag to clear samples"));
    m_eraseAction->setCheckable(true);
    m_eraseAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));

    //* Selector
    m_selAction = tb->addAction(QString::fromUtf8("⬚"));
    m_selAction->setToolTip(tr("Selection: select to delete/cut/copy"));
    m_selAction->setCheckable(true);
    m_selAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_W));

    tb->addSeparator();

    // Arrows
    m_arrowAction = tb->addAction(QString::fromUtf8("↝"));
    m_arrowAction->setToolTip(tr("Arrows: Select two points to add an arrow"));
    m_arrowAction->setCheckable(true);
    m_arrowAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_A));

    m_subArrowAction = tb->addAction(QString::fromUtf8("-↝"));
    m_subArrowAction->setToolTip(tr("Delete Arrow"));
    m_subArrowAction->setCheckable(true);
    m_subArrowAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_A));
    tb->addSeparator();

    // Add Marker
    m_addMarkerAction = tb->addAction(QString::fromUtf8("+|"));
    m_addMarkerAction->setToolTip(tr("Add a marker"));
    m_addMarkerAction->setCheckable(true);
    m_addMarkerAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_A));

    // Sub Marker
    m_subMarkerAction = tb->addAction(QString::fromUtf8("-|"));
    m_subMarkerAction->setToolTip(tr("Add a marker"));
    m_subMarkerAction->setCheckable(true);
    m_subMarkerAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_A));

    tb->addSeparator();

    // Number of time steps
    m_sampleSpin = new QSpinBox(tb);
    m_sampleSpin->setRange(4, 1000000);
    m_sampleSpin->setValue(m_document.sampleCount());
    m_sampleSpin->setToolTip(tr("Number of time steps"));

    tb->addWidget(m_sampleSpin);

    // Connections
    connect(m_undoAction, &QAction::triggered, this, &MainWindow::onUndo);
    connect(m_redoAction, &QAction::triggered, this, &MainWindow::onRedo);

    connect(zoomOutAct, &QAction::triggered, m_waveView, &WaveView::zoomOut);
    connect(zoomInAct, &QAction::triggered, m_waveView, &WaveView::zoomIn);

    connect(m_cutAction, &QAction::toggled, this, &MainWindow::onCutToggled);
    connect(m_eraseAction, &QAction::toggled, this, &MainWindow::onEraseToggled);

    connect(m_sampleSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            &m_document, &WaveDocument::setSampleCount);

    connect(m_addMarkerAction,    &QAction::toggled,
            this,                 &MainWindow::onAddMarkerToggled);
    connect(m_subMarkerAction, &QAction::toggled,
            this,                 &MainWindow::onSubMarkerToggled);

    connect(m_arrowAction,        &QAction::toggled,
            this,                 &MainWindow::onArrowToggled);
    connect(m_subArrowAction,  &QAction::toggled,
            this,                 &MainWindow::onSubArrowToggled);
    connect(m_selAction, &QAction::toggled,
            this,                &MainWindow::onSelectBlockToggled);
}

