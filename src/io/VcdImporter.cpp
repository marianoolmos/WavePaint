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
#include "io/VcdImporter.h"
#include "core.h"

#include <QFile>
#include <QTextStream>
#include <QHash>
#include <QVector>
#include <QStringList>
#include <algorithm>


bool WaveVcdImporter::loadFromVcd(WaveDocument &doc, const QString &fileName)
{
    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream ts(&f);

    const int MAX_SAMPLES = 200000; // sample limit for large VCDs

    struct TmpSignal {
        QString name;
        QString id;
        int width = 1;
        SignalType type = SignalType::Bit;
        std::vector<int> values;
        std::vector<QString> labels;
    };

    QVector<TmpSignal> tmpSignals;
    QHash<QString, int> idToIndex;
    QStringList scopeStack;

    bool inHeader = true;
    int sampleIdx = -1;
    int maxSampleIdx = -1;

    while (!ts.atEnd()) {
        QString line = ts.readLine();
        if (line.isEmpty())
            continue;
        line = line.trimmed();
        if (line.isEmpty())
            continue;

        if (inHeader) {
            if (line.startsWith("$scope")) {
                // $scope module top $end
                QStringList parts = line.split(' ', Qt::SkipEmptyParts);
                if (parts.size() >= 3) {
                    QString scopeName = parts[2];
                    if (scopeName != "$end")
                        scopeStack.append(scopeName);
                }
            } else if (line.startsWith("$upscope")) {
                if (!scopeStack.isEmpty())
                    scopeStack.removeLast();
            } else if (line.startsWith("$var")) {
                // $var wire 1 ! clk $end
                QStringList parts = line.split(' ', Qt::SkipEmptyParts);
                if (parts.size() >= 5) {
                    QString typeStr = parts[1];
                    bool okWidth = false;
                    int width = parts[2].toInt(&okWidth);
                    if (!okWidth || width <= 0)
                        width = 1;
                    QString id = parts[3];

                    QString name;
                    for (int i = 4; i < parts.size(); ++i) {
                        if (parts[i] == "$end")
                            break;
                        if (!name.isEmpty())
                            name += " ";
                        name += parts[i];
                    }
                    if (name.isEmpty())
                        name = id;

                    QString fullName;
                    if (!scopeStack.isEmpty())
                        fullName = scopeStack.join(".") + "." + name;
                    else
                        fullName = name;

                    TmpSignal tmp;
                    tmp.name = fullName;
                    tmp.id = id;
                    tmp.width = width;
                    tmp.type = (width == 1 ? SignalType::Bit : SignalType::Vector);
                    tmpSignals.append(tmp);
                    idToIndex.insert(id, tmpSignals.size() - 1);
                }
            } else if (line.startsWith("$enddefinitions")) {
                inHeader = false;
            }
            continue;
        }

        // Data body
        if (line[0] == '#') {
            // new logical time -> new compressed sample index
            sampleIdx++;
            if (sampleIdx > maxSampleIdx)
                maxSampleIdx = sampleIdx;

            if (sampleIdx >= MAX_SAMPLES) {
                // Avoid continuing to read a huge VCD: keep only the first MAX_SAMPLES samples.
                break;
            }
            continue;
        }

        if (line.startsWith("$")) {
            // Ignore directives like $dumpvars, $end, etc.
            continue;
        }

        if (sampleIdx < 0) {
            // If changes appear before any '#', associate them with sample 0
            sampleIdx = 0;
            maxSampleIdx = std::max(maxSampleIdx, sampleIdx);
        }

        QChar c = line[0];
        if (c == 'b' || c == 'B') {
            // Format: b<bits> <id>
            QStringList parts = line.split(' ', Qt::SkipEmptyParts);
            if (parts.size() < 2)
                continue;
            QString bits = parts[0].mid(1);
            QString id = parts[1];
            if (!idToIndex.contains(id))
                continue;
            TmpSignal &tmp = tmpSignals[idToIndex.value(id)];
            if (sampleIdx >= static_cast<int>(tmp.values.size())) {
                tmp.values.resize(sampleIdx + 1, UNDEFINED_VALUE);
                tmp.labels.resize(sampleIdx + 1);
            }

            if (bits.contains('x', Qt::CaseInsensitive) ||
                bits.contains('z', Qt::CaseInsensitive)) {
                tmp.values[sampleIdx] = UNDEFINED_VALUE;
            } else {
                // For very wide buses we don't try to convert to int,
                // we just store a marker in the label.
                if (tmp.width > 32 || bits.size() > 32) {
                    tmp.values[sampleIdx] = 0;
                    tmp.labels[sampleIdx] = QString("[%1 bits]").arg(tmp.width);
                } else {
                    bool okVal = false;
                    int val = bits.toInt(&okVal, 2);
                    tmp.values[sampleIdx] = okVal ? val : UNDEFINED_VALUE;
                }
            }
        } else if (c == '0' || c == '1' || c == 'x' || c == 'X' || c == 'z' || c == 'Z') {
            // Scalars: 0id / 1id / xid / zid
            QString id = line.mid(1).trimmed();
            if (!idToIndex.contains(id))
                continue;
            TmpSignal &tmp = tmpSignals[idToIndex.value(id)];
            if (sampleIdx >= static_cast<int>(tmp.values.size())) {
                tmp.values.resize(sampleIdx + 1, UNDEFINED_VALUE);
                tmp.labels.resize(sampleIdx + 1);
            }
            if (c == 'x' || c == 'X' || c == 'z' || c == 'Z') {
                tmp.values[sampleIdx] = UNDEFINED_VALUE;
            } else {
                int val = (c == '1') ? 1 : 0;
                tmp.values[sampleIdx] = val;
            }
        } else {
            // Other lines (not interpreted)
            continue;
        }
    }

    if (maxSampleIdx < 0)
        return false;

    int sampleCount = maxSampleIdx + 1;

    // Normalize length and fill with last known value
    for (TmpSignal &tmp : tmpSignals) {
        if (static_cast<int>(tmp.values.size()) < sampleCount) {
            tmp.values.resize(sampleCount, UNDEFINED_VALUE);
            tmp.labels.resize(sampleCount);
        }

        int last = UNDEFINED_VALUE;
        for (int i = 0; i < sampleCount; ++i) {
            int v = tmp.values[i];
            if (v != UNDEFINED_VALUE) {
                last = v;
            } else {
                tmp.values[i] = last;
            }
        }
    }

    // Copy to internal VCD library (without adding to visible waveform yet)
    doc.m_vcdSignals.clear();
    doc.m_signals.clear(); // don't show any signals by default
    doc.m_sampleCount = sampleCount;
    doc.m_vcdSignals.reserve(tmpSignals.size());

    for (const TmpSignal &tmp : tmpSignals) {
        Signal s;
        s.name = tmp.name;
        s.type = tmp.type;
        s.values = tmp.values;
        s.labels = tmp.labels;
        if (s.type == SignalType::Bit)
            s.color = QColor(0, 150, 0);
        else
            s.color = QColor(0, 0, 180);
        doc.m_vcdSignals.push_back(std::move(s));
    }

    emit doc.dataChanged();
    return true;
}
