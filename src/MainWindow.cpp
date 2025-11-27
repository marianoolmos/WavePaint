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

    statusBar()->showMessage(tr("Ready"));
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
    sizes << 0 << width();   // izquierda = 0, derecha = todo
    m_splitter->setSizes(sizes);

    // Connect hierarchy selection to the signal list
    connect(m_hierarchyTree, &QTreeWidget::currentItemChanged,
            this, &MainWindow::onHierarchySelectionChanged);

    // Double-click a signal in the list => add it to the waveform
    connect(m_signalList, &QListWidget::itemDoubleClicked,
            this, &MainWindow::onSignalDoubleClicked);
}




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
    connect(m_viewHierarchyAction, &QAction::toggled,this, &MainWindow::toggleHierarchyPanel);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction *helpAct = helpMenu->addAction(tr("Documentation"), this, &MainWindow::linkToDoc);
    newAct->setShortcut(QKeySequence::New);
}




void MainWindow::createToolBar()
{
    QToolBar *tb = addToolBar(tr("Timeline"));
    tb->setMovable(false);

    // Zoom out / zoom in buttons
    QAction *zoomOutAct = tb->addAction(QStringLiteral("-"));
    zoomOutAct->setToolTip(tr("Zoom out (less detail)"));
    QAction *zoomInAct = tb->addAction(QStringLiteral("+"));
    zoomInAct->setToolTip(tr("Zoom in (more detail)"));

    // Scissors (cut) button with emoji
    m_cutAction = tb->addAction(QString::fromUtf8("✂"));
    m_cutAction->setToolTip(tr("Cut range: select two points to keep only that interval"));
    m_cutAction->setCheckable(true);
    m_cutAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));

    // Eraser button
    m_eraseAction = tb->addAction(QString::fromUtf8("🧽"));
    m_eraseAction->setToolTip(tr("Eraser: click/drag to clear samples"));
    m_eraseAction->setCheckable(true);
    m_eraseAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));

    m_sampleSpin = new QSpinBox(tb);
    m_sampleSpin->setRange(4, 1000000);
    m_sampleSpin->setValue(m_document.sampleCount());
    m_sampleSpin->setToolTip(tr("Number of time steps"));

    tb->addWidget(m_sampleSpin);

    // Connections
    connect(zoomOutAct, &QAction::triggered, m_waveView, &WaveView::zoomOut);
    connect(zoomInAct, &QAction::triggered, m_waveView, &WaveView::zoomIn);

    connect(m_cutAction, &QAction::toggled, this, &MainWindow::onCutToggled);
    connect(m_eraseAction, &QAction::toggled, this, &MainWindow::onEraseToggled);

    connect(m_sampleSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            &m_document, &WaveDocument::setSampleCount);
}





void MainWindow::exportPng()
{
    if (!m_waveView)
        return;

    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Export as PNG"),
        QString(),
        tr("PNG Images (*.png)")
    );

    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".png", Qt::CaseInsensitive)) {
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
        &ok
    );

    if (!ok)
        return;

    QColor bg = Qt::white;
    if (choice.toLower().contains("black")) {
        bg = Qt::black;
    }

    bool result = m_waveView->exportToPng(fileName, bg);
    if (result) {
        statusBar()->showMessage(tr("Exported to %1").arg(fileName), 3000);
    } else {
        statusBar()->showMessage(tr("Export failed"), 3000);
    }
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open waveform"),
        QString(),
        tr("WavePaint files (*.wp *.json *.vcd *.fst *.ghw);;All files (*)")
    );
    if (fileName.isEmpty())
        return;

    QFileInfo info(fileName);
    const QString ext = info.suffix().toLower();

    bool ok = false;

    if (ext == "vcd") {
        ok = m_document.loadFromVcd(fileName);
    } else if (ext == "wp" || ext == "json" || ext.isEmpty()) {
        ok = m_document.loadFromFile(fileName);
    } else if (ext == "fst" || ext == "ghw") {
        // FST/GHW not implemented yet: could be converted to VCD externally in the future
        ok = false;
    } else {
        // Try as JSON/own format
        ok = m_document.loadFromFile(fileName);
    }


    if (ok) {
        m_currentFile = fileName;
        if (m_sampleSpin) {
            m_sampleSpin->setValue(m_document.sampleCount());
        }

        if (ext == "vcd") {
            rebuildHierarchy();
        } else {
            // For other formats, clear the hierarchy but keep the splitter
            if (m_hierarchyTree) m_hierarchyTree->clear();
            if (m_signalList) m_signalList->clear();
        }

        statusBar()->showMessage(tr("Loaded %1").arg(fileName), 3000);
    } else {
        if (ext == "fst" || ext == "ghw") {
            statusBar()->showMessage(tr("FST/GHW import not implemented yet (VCD supported)."), 5000);
        } else {
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
        tr("WavePaint files (*.wp *.json);;All files (*)")
    );
    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".wp", Qt::CaseInsensitive) &&
        !fileName.endsWith(".json", Qt::CaseInsensitive)) {
        fileName += ".wp";
    }

    if (m_document.saveToFile(fileName)) {
        m_currentFile = fileName;
        statusBar()->showMessage(tr("Saved to %1").arg(fileName), 3000);
    } else {
        statusBar()->showMessage(tr("Failed to save %1").arg(fileName), 3000);
    }
}



void MainWindow::newDocument()
{
    m_document.clear();
    if (m_sampleSpin) {
        m_sampleSpin->setValue(m_document.sampleCount());
    }
    if (m_hierarchyTree) m_hierarchyTree->clear();
    if (m_signalList) m_signalList->clear();
    statusBar()->showMessage(tr("New document"), 2000);
}



void MainWindow::clearAllSignals()
{
    m_document.clearSignals();
    if (m_hierarchyTree) m_hierarchyTree->clear();
    if (m_signalList) m_signalList->clear();
    statusBar()->showMessage(tr("All signals cleared"), 2000);
}
void MainWindow::toggleHierarchyPanel(bool visible)
{
    if (!m_splitter)
        return;

    QList<int> sizes = m_splitter->sizes();
    if (sizes.size() < 2)
        return;

    if (visible) {
        // Darle un ancho razonable al panel izquierdo
        if (sizes[0] == 0) {
            sizes[0] = width() / 4;
            sizes[1] = width() - sizes[0];
        }
    } else {
        // Colapsar el panel izquierdo
        sizes[0] = 0;
        sizes[1] = width();
    }

    m_splitter->setSizes(sizes);
}

void MainWindow::linkToDoc()
{
    const QUrl url("https://github.com/marianoolmos/WavePaint"); 
    QDesktopServices::openUrl(url);
}

void MainWindow::rebuildHierarchy()
{
    if (!m_hierarchyTree || !m_signalList)
        return;

    m_hierarchyTree->clear();
    m_signalList->clear();

    const auto &sigs = m_document.vcdSignalList();
    if (sigs.empty())
        return;

    // Build hierarchy tree from names separated by '.'
    QMap<QString, QTreeWidgetItem*> pathToItem;

    for (const Signal &s : sigs) {
        QString fullName = s.name;
        if (fullName.isEmpty())
            continue;

        QStringList parts = fullName.split('.');
        if (parts.size() <= 1)
            continue;

        QStringList moduleParts = parts;
        moduleParts.removeLast(); // last element = signal name

        QString pathSoFar;
        QTreeWidgetItem *parentItem = nullptr;
        for (const QString &part : moduleParts) {
            pathSoFar = pathSoFar.isEmpty() ? part : pathSoFar + "." + part;
            if (!pathToItem.contains(pathSoFar)) {
                QTreeWidgetItem *item = new QTreeWidgetItem();
                item->setText(0, part);
                if (parentItem)
                    parentItem->addChild(item);
                else
                    m_hierarchyTree->addTopLevelItem(item);
                pathToItem.insert(pathSoFar, item);
            }
            parentItem = pathToItem.value(pathSoFar);
        }
    }

    m_hierarchyTree->expandAll();

    // Ensure left panel remains visible
    if (m_splitter) {
        QList<int> sizes = m_splitter->sizes();
        if (sizes.size() >= 2) {
            if (sizes[0] == 0) {
                sizes[0] = width() / 3;
                sizes[1] = width() - sizes[0];
                m_splitter->setSizes(sizes);
            }
        }
    }
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
    while (it) {
        chain.prepend(it->text(0));
        it = it->parent();
    }
    QString modulePath = chain.join('.');

    const auto &sigs = m_document.vcdSignalList();
    for (const Signal &s : sigs) {
        QString fullName = s.name;
        if (fullName.isEmpty())
            continue;

        QStringList parts = fullName.split('.');
        if (parts.size() <= 1)
            continue;

        QString leaf = parts.last();
        parts.removeLast();
        QString sigModulePath = parts.join('.');

        if (sigModulePath == modulePath) {
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
    while (it) {
        chain.prepend(it->text(0));
        it = it->parent();
    }
    QString modulePath = chain.join('.');

    QString leafName = item->text();
    QString fullName = modulePath.isEmpty() ? leafName : (modulePath + "." + leafName);

    // Add signal visible from the VCD library
    int idx = m_document.addSignalFromVcd(fullName);
    if (idx < 0) {
        statusBar()->showMessage(tr("Signal %1 not found in VCD library").arg(fullName), 3000);
    } else {
        statusBar()->showMessage(tr("Added signal %1").arg(fullName), 2000);
    }
}


void MainWindow::onCutToggled(bool enabled)
{
    if (!m_waveView)
        return;

    if (enabled) {
        if (m_eraseAction) {
            m_eraseAction->blockSignals(true);
            m_eraseAction->setChecked(false);
            m_eraseAction->blockSignals(false);
        }
        m_waveView->setEraseModeEnabled(false);
        m_waveView->setCutModeEnabled(true);
    } else {
        m_waveView->setCutModeEnabled(false);
    }
}

void MainWindow::onEraseToggled(bool enabled)
{
    if (!m_waveView)
        return;

    if (enabled) {
        if (m_cutAction) {
            m_cutAction->blockSignals(true);
            m_cutAction->setChecked(false);
            m_cutAction->blockSignals(false);
        }
        m_waveView->setCutModeEnabled(false);
        m_waveView->setEraseModeEnabled(true);
    } else {
        m_waveView->setEraseModeEnabled(false);
    }
}

void MainWindow::cancelModes()
{
    if (!m_waveView)
        return;

    if (m_cutAction) {
        m_cutAction->blockSignals(true);
        m_cutAction->setChecked(false);
        m_cutAction->blockSignals(false);
    }
    if (m_eraseAction) {
        m_eraseAction->blockSignals(true);
        m_eraseAction->setChecked(false);
        m_eraseAction->blockSignals(false);
    }

    m_waveView->setCutModeEnabled(false);
    m_waveView->setEraseModeEnabled(false);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        cancelModes();
        event->accept();
        return;
    }

    QMainWindow::keyPressEvent(event);
}
