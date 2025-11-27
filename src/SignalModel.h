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

#ifndef SIGNALMODEL_H
#define SIGNALMODEL_H

#include <QObject>
#include <QString>
#include <QColor>
#include <vector>

class WaveJsonIO;
class WaveVcdImporter;

enum class SignalType {
    Bit,
    Vector
};

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

class WaveDocument : public QObject
{
    Q_OBJECT
    friend class WaveJsonIO;
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

    void copySignal(int signalIndex);
    int  pasteSignal(int destIndex);
    bool hasClipboardSignal() const { return m_hasClipboardSignal; }
    void clear();
    void clearSignals();

    // Add a visible signal from the VCD library
    int addSignalFromVcd(const QString &fullName);

    // Persistence to disk
    bool saveToFile(const QString &fileName) const;
    bool loadFromFile(const QString &fileName);
    bool loadFromVcd(const QString &fileName);

public slots:
    void setSampleCount(int count);

signals:
    void dataChanged();

private:
    int m_sampleCount;
    std::vector<Signal> m_signals;      // visible signals in the waveform
    std::vector<Signal> m_vcdSignals;   // library of signals loaded from VCD

    bool   m_hasClipboardSignal = false;
    Signal m_clipboardSignal;

    void resizeSignals(int newSampleCount);
};

#endif // SIGNALMODEL_H
