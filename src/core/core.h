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

#ifndef CORE_H
#define CORE_H

#include <QObject>
#include <QString> 
#include <QColor>
#include <vector>

class JsonIO;
class VcdImporter;

enum class SignalType {
    Bit,
    Vector
};
static constexpr int UNDEFINED_VALUE = -1;
struct Signal
{
    QString name;
    SignalType type;
    std::vector<int> values;      // -1 = undefined, >=0 valid value
    std::vector<QString> labels;  // optional labels per sample (for vectors)
    QColor color;                 // drawing color of the signal

    Signal(const QString &n = QString(),
           SignalType t = SignalType::Bit,

           int samples = 0)
        : name(n),
          type(t),
          values(samples, -1),
          labels(samples),
          color(Qt::black)
    {
        if (t == SignalType::Bit)
            color = QColor(0, 150, 0);
        else
            color = QColor(0, 0, 180);
    }
};
struct Marker {
    int id;
    int sample;
};
struct Arrow {
    int id;           // identificador unico
    int startSignal;  // Indice de senyal origen
    int startSample;  // timestamp origen
    int endSignal;    // Indice de señal destino
    int endSample;    // timestamp destino
};

class WaveDocument : public QObject
{
    Q_OBJECT
    friend class JsonIO;
    friend class WaveVcdImporter;
public:
    explicit WaveDocument(QObject *parent = nullptr);

    int sampleCount() const { return m_sampleCount; }

    const std::vector<Signal> &signalList() const { return m_signals; }
    std::vector<Signal> &signalList() { return m_signals; }

    // Signals coming from a VCD library (not all are necessarily shown)
    const std::vector<Signal> &vcdSignalList() const { return m_vcdSignals; }


    // High-level API
    int addBitSignal(const QString &name);
    int addVectorSignal(const QString &name);
    int addClockSignal(const QString &name, int pulses, int highSamples, int lowSamples);

    void toggleBitValue(int signalIndex, int sampleIndex);          // still available if needed
    void setBitValue(int signalIndex, int sampleIndex, int value);  // 0 or 1
    void setVectorRange(int signalIndex, int startSample, int endSample, int value, const QString &label = QString());
    void clearSample(int signalIndex, int sampleIndex);
    void setSignalColor(int signalIndex, const QColor &c);
    void renameSignal(int signalIndex, const QString &name);
    void cutRange(int startSample, int endSample);
    void removeSignal(int signalIndex);
    void moveSignal(int fromIndex, int toIndex);

    bool hasBlockClipboard() const { return m_hasBlockClipboard; }
    int  blockClipboardSignalCount() const { return m_blockClipboardSignalCount; }
    int  blockClipboardSampleCount() const { return m_blockClipboardSampleCount; }

    void copyBlock(int topSignal, int bottomSignal,
                   int startSample, int endSample);
    void pasteBlock(int destTopSignal, int destStartSample);
    void clearBlock(int topSignal, int bottomSignal,
                    int startSample, int endSample);

    const std::vector<Marker> &markerList() const { return m_markers; }


    int  addMarker(int sampleIndex);          
    void subMarkerById(int markerId);      

    void clearMarkers();
    void addMarkerFromLoad(int id, int sampleIndex);

    const std::vector<Arrow> &arrowList() const { return m_arrows; }
    int  addArrow(int startSignal, int startSample,
                  int endSignal,   int endSample);
    void subArrowById(int arrowId);
    void clearArrows();


    void copySignal(int signalIndex);
    int  pasteSignal(int destIndex);
    bool hasClipboardSignal() const { return m_hasClipboardSignal; }
    void clear();
    void clearSignals();

    void undo();
    void redo();
    bool canUndo() const;
    bool canRedo() const;
    

    // Add a visible signal from the VCD library
    int addSignalFromVcd(const QString &fullName);

    // Persistence to disk
    bool saveToFile(const QString &fileName) const;
    bool loadFromFile(const QString &fileName);
    bool loadFromVcd(const QString &fileName);



    const std::vector<std::vector<int>>     &blockClipboardValues() const { return m_blockClipboardValues; }
    const std::vector<std::vector<QString>> &blockClipboardLabels() const { return m_blockClipboardLabels; }
    const std::vector<SignalType>           &blockClipboardTypes()  const { return m_blockClipboardTypes; }
    const std::vector<QColor>               &blockClipboardColors() const { return m_blockClipboardColors; }


public slots:
    void setSampleCount(int count);

signals:
    void dataChanged();
    void undoRedoStateChanged(); 

private:
    int m_sampleCount;
    std::vector<Signal> m_signals;      // visible signals in the waveform
    std::vector<Signal> m_vcdSignals;   // library of signals loaded from VCD

    std::vector<Marker> m_markers;
    int m_nextMarkerId = 1;

    std::vector<Arrow> m_arrows;
    int m_nextArrowId = 1;

    bool   m_hasClipboardSignal = false;
    Signal m_clipboardSignal;


    bool m_hasBlockClipboard = false;
    int  m_blockClipboardSignalCount = 0;
    int  m_blockClipboardSampleCount = 0;
    std::vector<std::vector<int>>       m_blockClipboardValues;
    std::vector<std::vector<QString>>   m_blockClipboardLabels;

    std::vector<SignalType>             m_blockClipboardTypes;   
    std::vector<QColor>                 m_blockClipboardColors;

    void resizeSignals(int newSampleCount);

    // Undo/Redo system
    struct Snapshot {
        int sampleCount;
        std::vector<Signal> m_signals;
        std::vector<Signal> m_vcdSignals;
        std::vector<Marker> m_markers;
        int m_nextMarkerId;
        std::vector<Arrow> m_arrows;
        int m_nextArrowId;
    };

    std::vector<Snapshot> m_undoStack;
    std::vector<Snapshot> m_redoStack;
    int m_maxUndoSteps = 50;  

    void pushUndoSnapshot();  
    void clearHistory();     


};

#endif // CORE_H
