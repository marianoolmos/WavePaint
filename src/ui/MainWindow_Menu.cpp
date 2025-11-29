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

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QAction *newAct = fileMenu->addAction(tr("&New"), this, &MainWindow::newDocument);
    newAct->setShortcut(QKeySequence::New);

    QAction *openAct = fileMenu->addAction(tr("&Open..."), this, &MainWindow::openFile);
    openAct->setShortcut(QKeySequence::Open);

    QAction *saveAsAct = fileMenu->addAction(tr("Save &As..."), this, &MainWindow::saveFileAs);

    QAction *exportAct = fileMenu->addAction(tr("Export PNG..."), this, &MainWindow::exportPng);

    fileMenu->addSeparator();

    QAction *quitAct = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
    quitAct->setShortcut(QKeySequence::Quit);

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    QAction *clearAct = editMenu->addAction(tr("Clear all signals"), this, &MainWindow::clearAllSignals);
    clearAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));

    Q_UNUSED(clearAct);

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    m_viewHierarchyAction = viewMenu->addAction(tr("Hierarchy && Signals"));
    m_viewHierarchyAction->setCheckable(true);
    m_viewHierarchyAction->setChecked(false); // off by default
    m_viewHierarchyAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_H));
    connect(m_viewHierarchyAction, &QAction::toggled, this, &MainWindow::toggleHierarchyPanel);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction *helpAct = helpMenu->addAction(tr("Documentation"), this, &MainWindow::linkToDoc);
    newAct->setShortcut(QKeySequence::New);
}




void MainWindow::exportPng()
{
    if (!m_waveView)
        return;

    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Export as PNG"),
        QString(),
        tr("PNG Images (*.png)"));

    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".png", Qt::CaseInsensitive))
    {
        fileName += ".png";
    }

    QStringList options;
    options << tr("White background") << tr("Black background");
    bool ok = false;
    QString choice = QInputDialog::getItem(
        this,
        tr("Background color"),
        tr("Choose background for export:"),
        options,
        0,
        false,
        &ok);

    if (!ok)
        return;

    QColor bg = Qt::white;
    if (choice.toLower().contains("black"))
    {
        bg = Qt::black;
    }

    bool result = m_waveView->exportToPng(fileName, bg);
    if (result)
    {
        statusBar()->showMessage(tr("Exported to %1").arg(fileName), 3000);
    }
    else
    {
        statusBar()->showMessage(tr("Export failed"), 3000);
    }
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open waveform"),
        QString(),
        tr("WavePaint files (*.wp *.json *.vcd *.fst *.ghw);;All files (*)"));
    if (fileName.isEmpty())
        return;

    QFileInfo info(fileName);
    const QString ext = info.suffix().toLower();

    bool ok = false;

    if (ext == "vcd")
    {
        ok = m_document.loadFromVcd(fileName);
    }
    else if (ext == "wp" || ext == "json" || ext.isEmpty())
    {
        ok = m_document.loadFromFile(fileName);
    }
    else if (ext == "fst" || ext == "ghw")
    {
        // FST/GHW not implemented yet: could be converted to VCD externally in the future
        ok = false;
    }
    else
    {
        // Try as JSON/own format
        ok = m_document.loadFromFile(fileName);
    }

    if (ok)
    {
        m_currentFile = fileName;
        if (m_sampleSpin)
        {
            m_sampleSpin->setValue(m_document.sampleCount());
        }

        if (ext == "vcd")
        {
            rebuildHierarchy();
        }
        else
        {
            // For other formats, clear the hierarchy but keep the splitter
            if (m_hierarchyTree)
                m_hierarchyTree->clear();
            if (m_signalList)
                m_signalList->clear();
        }

        statusBar()->showMessage(tr("Loaded %1").arg(fileName), 3000);
    }
    else
    {
        if (ext == "fst" || ext == "ghw")
        {
            statusBar()->showMessage(tr("FST/GHW import not implemented yet (VCD supported)."), 5000);
        }
        else
        {
            statusBar()->showMessage(tr("Failed to load %1").arg(fileName), 3000);
        }
    }
}

void MainWindow::saveFileAs()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save waveform"),
        QString(),
        tr("WavePaint files (*.wp *.json);;All files (*)"));
    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".wp", Qt::CaseInsensitive) &&
        !fileName.endsWith(".json", Qt::CaseInsensitive))
    {
        fileName += ".wp";
    }

    if (m_document.saveToFile(fileName))
    {
        m_currentFile = fileName;
        statusBar()->showMessage(tr("Saved to %1").arg(fileName), 3000);
    }
    else
    {
        statusBar()->showMessage(tr("Failed to save %1").arg(fileName), 3000);
    }
}

void MainWindow::newDocument()
{
    m_document.clear();
    if (m_sampleSpin)
    {
        m_sampleSpin->setValue(m_document.sampleCount());
    }
    if (m_hierarchyTree)
        m_hierarchyTree->clear();
    if (m_signalList)
        m_signalList->clear();
    statusBar()->showMessage(tr("New document"), 2000);
}

void MainWindow::clearAllSignals()
{
    m_document.clearSignals();
    if (m_hierarchyTree)
        m_hierarchyTree->clear();
    if (m_signalList)
        m_signalList->clear();
    statusBar()->showMessage(tr("All signals cleared"), 2000);
}

void MainWindow::linkToDoc()
{
    const QUrl url("https://github.com/marianoolmos/WavePaint");
    QDesktopServices::openUrl(url);
}