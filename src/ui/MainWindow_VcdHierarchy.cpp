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
    QMap<QString, QTreeWidgetItem *> pathToItem;

    for (const Signal &s : sigs)
    {
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
        for (const QString &part : moduleParts)
        {
            pathSoFar = pathSoFar.isEmpty() ? part : pathSoFar + "." + part;
            if (!pathToItem.contains(pathSoFar))
            {
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
    if (m_splitter)
    {
        QList<int> sizes = m_splitter->sizes();
        if (sizes.size() >= 2)
        {
            if (sizes[0] == 0)
            {
                sizes[0] = width() / 3;
                sizes[1] = width() - sizes[0];
                m_splitter->setSizes(sizes);
            }
        }
    }
}
void MainWindow::toggleHierarchyPanel(bool visible)
{
    if (!m_splitter)
        return;

    QList<int> sizes = m_splitter->sizes();
    if (sizes.size() < 2)
        return;

    if (visible)
    {
        // Darle un ancho razonable al panel izquierdo
        if (sizes[0] == 0)
        {
            sizes[0] = width() / 4;
            sizes[1] = width() - sizes[0];
        }
    }
    else
    {
        // Colapsar el panel izquierdo
        sizes[0] = 0;
        sizes[1] = width();
    }

    m_splitter->setSizes(sizes);
}
