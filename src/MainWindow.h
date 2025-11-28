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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include "SignalModel.h"

class WaveView;
class QTreeWidget;
class QListWidget;
class QSplitter;
class QSpinBox;
class QTreeWidgetItem;
class QScrollArea;
class QListWidgetItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    WaveDocument m_document;
    WaveView *m_waveView;
    QSpinBox *m_sampleSpin;
    QString m_currentFile;

    // Main layout: splitter with hierarchy panel on the left and waveform on the right
    QSplitter *m_splitter;
    QTreeWidget *m_hierarchyTree;
    QAction *m_viewHierarchyAction;
    QListWidget *m_signalList;
    QScrollArea *m_waveScroll;
    QAction *m_cutAction;
    QAction *m_arrowAction;
    QAction *m_subArrowAction;
    QAction *m_selAction;
    QAction *m_addMarkerAction;
    QAction *m_subMarkerAction;
    QAction *m_eraseAction;

    void createUi();
    void createMenus();
    void createToolBar();
    void rebuildHierarchy();
    int signalCount() const;
    void moveSignal(int from, int to);

private slots:
    void newDocument();
    void clearAllSignals();
    void toggleHierarchyPanel(bool visible);
    void linkToDoc();
    void exportPng();
    void openFile();
    void saveFileAs();
    void onHierarchySelectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void onSignalDoubleClicked(QListWidgetItem *item);
    void onCutToggled(bool enabled);
    void onEraseToggled(bool enabled);
    void onAddMarkerToggled(bool enabled);
    void onSubMarkerToggled(bool enabled);
    void onArrowToggled(bool enabled);
    void onSubArrowToggled(bool enabled);
    void onSelectBlockToggled(bool enabled);
    void cancelModes();
};

#endif // MAINWINDOW_H
