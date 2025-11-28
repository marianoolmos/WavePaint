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
#ifndef WAVEVIEW_H
#define WAVEVIEW_H

#include <QWidget> 
#include <QColor>
#include <QSize>
#include "core/core.h"

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

    //marker
    void setMarkerAddModeEnabled(bool en);     
    void setMarkerSubModeEnabled(bool en);

    //arrow
    void setArrowModeEnabled(bool en);
    void setArrowSubModeEnabled(bool en); 
    QPointF signalSampleToPoint(int signalIndex, int sampleIndex) const;

    //Selection
    void setSelectionModeEnabled(bool en);


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
    void keyPressEvent(QKeyEvent *event) override;


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
        Erasing,
        MarkerAdd,
        MarkerSub,
        ArrowAdd,
        ArrowSub
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
    
    int  m_markerPreviewSample;


    //Arrow
    bool m_arrowHasStart = false;
    int  m_arrowStartSignal = -1;
    int  m_arrowStartSample = -1;
    int  m_arrowPreviewSignal = -1;
    int  m_arrowPreviewSample = -1;

    // Optional size for export (exact content width/height)
    QSize m_exportSize;
    // Background color for export (if invalid, use palette background)
    QColor m_exportBackground;

    // Modo selección de bloque
    bool m_selectionModeEnabled = false;
    bool m_blockSelectionActive = false;
    bool m_blockSelecting       = false;
    int  m_blockSelStartSignal  = -1;
    int  m_blockSelStartSample  = -1;
    int  m_blockSelEndSignal    = -1;
    int  m_blockSelEndSample    = -1;

    // Preview de pegado (posición donde se pegará el bloque)
    bool m_blockPastePreviewActive = false;
    int  m_blockPasteSignal        = -1;  // signal top
    int  m_blockPasteSample        = -1;  // sample de inicio

    // Helpers
    bool normalizedBlockSelection(int &topSignal, int &bottomSignal,
                                  int &startSample, int &endSample) const;
    bool pointInBlockSelection(int signalIndex, int sampleIndex) const;


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
