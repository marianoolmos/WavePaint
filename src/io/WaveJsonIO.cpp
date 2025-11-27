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
#include "io/WaveJsonIO.h"
#include "SignalModel.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

static constexpr int UNDEFINED_VALUE = -1;

bool WaveJsonIO::saveToFile(const WaveDocument &doc, const QString &fileName)
{
    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;

    QJsonObject root;
    root["sampleCount"] = doc.m_sampleCount;

    QJsonArray sigArray;
    for (const auto &s : doc.m_signals) {
        QJsonObject so;
        so["name"] = s.name;
        so["type"] = (s.type == SignalType::Bit ? "bit" : "vector");
        so["color"] = s.color.name(QColor::HexArgb);

        QJsonArray vals;
        for (int v : s.values)
            vals.append(v);
        so["values"] = vals;

        QJsonArray labs;
        for (const auto &lab : s.labels)
            labs.append(lab);
        so["labels"] = labs;

        sigArray.append(so);
    }
    root["signals"] = sigArray;

    QJsonDocument docJson(root);
    f.write(docJson.toJson(QJsonDocument::Indented));
    return true;
}

bool WaveJsonIO::loadFromFile(WaveDocument &doc, const QString &fileName)
{
    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly))
        return false;

    QByteArray data = f.readAll();
    QJsonParseError err;
    QJsonDocument jdoc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !jdoc.isObject())
        return false;

    QJsonObject root = jdoc.object();

    int samples = root.value("sampleCount").toInt(0);
    if (samples <= 0)
        return false;

    doc.m_sampleCount = samples;
    doc.m_signals.clear();
    doc.m_vcdSignals.clear();

    QJsonArray sigArray = root.value("signals").toArray();
    for (const QJsonValue &v : sigArray) {
        if (!v.isObject()) continue;
        QJsonObject so = v.toObject();

        Signal s;
        s.name = so.value("name").toString();
        QString typeStr = so.value("type").toString("bit");
        s.type = (typeStr == "vector") ? SignalType::Vector : SignalType::Bit;
        s.color = QColor(so.value("color").toString("#009600"));

        QJsonArray vals = so.value("values").toArray();
        s.values.resize(doc.m_sampleCount, UNDEFINED_VALUE);
        for (int i = 0; i < vals.size() && i < doc.m_sampleCount; ++i) {
            s.values[i] = vals[i].toInt(UNDEFINED_VALUE);
        }

        QJsonArray labs = so.value("labels").toArray();
        s.labels.resize(doc.m_sampleCount);
        for (int i = 0; i < labs.size() && i < doc.m_sampleCount; ++i) {
            s.labels[i] = labs[i].toString();
        }

        doc.m_signals.push_back(std::move(s));
    }

    emit doc.dataChanged();
    return true;
}
