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
// File:          WaveJsonIO.cpp
// Description:   Save / load WaveDocument to JSON (.wp / .json),
//                incluyendo señales y marcadores.
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

    // --- Información básica ---
    root["sampleCount"] = doc.m_sampleCount;

    // --- Señales ---
    QJsonArray sigArray;
    for (const auto &s : doc.m_signals) {
        QJsonObject so;
        so["name"]  = s.name;
        so["type"]  = (s.type == SignalType::Bit ? "bit" : "vector");
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

    // --- Marcadores ---
    // Cada marcador tiene un id fijo y una posición de sample.
    QJsonArray markerArray;
    for (const Marker &m : doc.m_markers) {
        QJsonObject mo;
        mo["id"]     = m.id;
        mo["sample"] = m.sample;
        markerArray.append(mo);
    }
    root["markers"] = markerArray;

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

    // --- Reset básico del documento ---
    doc.m_sampleCount = samples;
    doc.m_signals.clear();
    doc.m_vcdSignals.clear();
    doc.m_markers.clear();
    doc.m_nextMarkerId = 1;

    // --- Cargar señales ---
    QJsonArray sigArray = root.value("signals").toArray();
    for (const QJsonValue &v : sigArray) {
        if (!v.isObject())
            continue;

        QJsonObject so = v.toObject();

        Signal s;
        s.name = so.value("name").toString();
        QString typeStr = so.value("type").toString("bit");
        s.type  = (typeStr == "vector") ? SignalType::Vector : SignalType::Bit;
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

    // --- Cargar marcadores (si existen en el fichero) ---
    QJsonArray markerArray = root.value("markers").toArray();
    for (const QJsonValue &v : markerArray) {
        if (!v.isObject())
            continue;

        QJsonObject mo = v.toObject();

        int id     = mo.value("id").toInt(0);
        int sample = mo.value("sample").toInt(-1);

        if (id <= 0)
            continue;
        if (sample < 0 || sample >= doc.m_sampleCount)
            continue;

        Marker m;
        m.id     = id;
        m.sample = sample;
        doc.m_markers.push_back(m);

        if (id >= doc.m_nextMarkerId)
            doc.m_nextMarkerId = id + 1;
    }

    emit doc.dataChanged();
    return true;
}
