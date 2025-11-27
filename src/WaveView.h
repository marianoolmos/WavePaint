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
#ifndef WAVEVIEW_H
#define WAVEVIEW_H

#include <QWidget>
#include <QColor>
#include <QSize>
#include "SignalModel.h"

class WaveView : public QWidget
{
    Q_OBJECT
public:
    explicit WaveView(WaveDocument *doc, QWidget *parent = nullptr);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    // Export the current content to PNG with the specified background
    bool exportToPng(const QString &fileName, const QColor &background);

public slots:
    // Activate cut mode: the user chooses two points and the range is cut
    void startCutMode();              // shortcut for setCutModeEnabled(true)
    void setCutModeEnabled(bool en);  // enable/disable from the UI
    void setEraseModeEnabled(bool en);

    // Horizontal zoom (changes the cell width)
    void zoomIn();
    void zoomOut();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void onDocumentChanged();

private:
    WaveDocument *m_doc;

    int m_rowHeight;
    int m_cellWidth;
    int m_leftMargin;
    int m_topMargin;

    enum class Mode {
        None,
        VectorSelecting,
        BitPainting,
        CutSelecting,
        Erasing
    };

    Mode m_mode;

    // For vector selection
    int m_selSignal;
    int m_selStartSample;
    int m_selCurrentSample;

    // For continuous bit painting
    int m_bitPaintSignal;
    int m_bitPaintValue;   // 0 or 1
    int m_bitLastSample;

    // For cut mode
    int m_cutStartSample;
    int m_cutCurrentSample;

    //Moving Signals
    bool m_isMovingSignal;
    int m_moveSignalIndex;  

    // Optional size for export (exact content width/height)
    QSize m_exportSize;
    // Background color for export (if invalid, use palette background)
    QColor m_exportBackground;

    bool mapToSignalSample(const QPoint &pos, int &signalIndex, int &sampleIndex) const;
    int mapToSignalIndexFromY(int y) const;

    void drawSignal(QPainter &p, const Signal &sig, int index);
    void drawBitSignal(QPainter &p, const Signal &sig, int index);
    void drawVectorSignal(QPainter &p, const Signal &sig, int index);
    void drawVectorSelection(QPainter &p);

    void addBitSignal();
    void addVectorSignal();
    void addClockSignal();
};

#endif // WAVEVIEW_H
