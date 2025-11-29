/****************************************************************************
** Meta object code from reading C++ file 'WaveView.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/ui/WaveView.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'WaveView.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_WaveView_t {
    uint offsetsAndSizes[34];
    char stringdata0[9];
    char stringdata1[13];
    char stringdata2[1];
    char stringdata3[18];
    char stringdata4[3];
    char stringdata5[20];
    char stringdata6[24];
    char stringdata7[24];
    char stringdata8[20];
    char stringdata9[23];
    char stringdata10[20];
    char stringdata11[12];
    char stringdata12[12];
    char stringdata13[24];
    char stringdata14[7];
    char stringdata15[8];
    char stringdata16[18];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_WaveView_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_WaveView_t qt_meta_stringdata_WaveView = {
    {
        QT_MOC_LITERAL(0, 8),  // "WaveView"
        QT_MOC_LITERAL(9, 12),  // "startCutMode"
        QT_MOC_LITERAL(22, 0),  // ""
        QT_MOC_LITERAL(23, 17),  // "setCutModeEnabled"
        QT_MOC_LITERAL(41, 2),  // "en"
        QT_MOC_LITERAL(44, 19),  // "setEraseModeEnabled"
        QT_MOC_LITERAL(64, 23),  // "setMarkerAddModeEnabled"
        QT_MOC_LITERAL(88, 23),  // "setMarkerSubModeEnabled"
        QT_MOC_LITERAL(112, 19),  // "setArrowModeEnabled"
        QT_MOC_LITERAL(132, 22),  // "setArrowSubModeEnabled"
        QT_MOC_LITERAL(155, 19),  // "signalSampleToPoint"
        QT_MOC_LITERAL(175, 11),  // "signalIndex"
        QT_MOC_LITERAL(187, 11),  // "sampleIndex"
        QT_MOC_LITERAL(199, 23),  // "setSelectionModeEnabled"
        QT_MOC_LITERAL(223, 6),  // "zoomIn"
        QT_MOC_LITERAL(230, 7),  // "zoomOut"
        QT_MOC_LITERAL(238, 17)   // "onDocumentChanged"
    },
    "WaveView",
    "startCutMode",
    "",
    "setCutModeEnabled",
    "en",
    "setEraseModeEnabled",
    "setMarkerAddModeEnabled",
    "setMarkerSubModeEnabled",
    "setArrowModeEnabled",
    "setArrowSubModeEnabled",
    "signalSampleToPoint",
    "signalIndex",
    "sampleIndex",
    "setSelectionModeEnabled",
    "zoomIn",
    "zoomOut",
    "onDocumentChanged"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_WaveView[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   86,    2, 0x0a,    1 /* Public */,
       3,    1,   87,    2, 0x0a,    2 /* Public */,
       5,    1,   90,    2, 0x0a,    4 /* Public */,
       6,    1,   93,    2, 0x0a,    6 /* Public */,
       7,    1,   96,    2, 0x0a,    8 /* Public */,
       8,    1,   99,    2, 0x0a,   10 /* Public */,
       9,    1,  102,    2, 0x0a,   12 /* Public */,
      10,    2,  105,    2, 0x10a,   14 /* Public | MethodIsConst  */,
      13,    1,  110,    2, 0x0a,   17 /* Public */,
      14,    0,  113,    2, 0x0a,   19 /* Public */,
      15,    0,  114,    2, 0x0a,   20 /* Public */,
      16,    0,  115,    2, 0x08,   21 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::QPointF, QMetaType::Int, QMetaType::Int,   11,   12,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject WaveView::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_WaveView.offsetsAndSizes,
    qt_meta_data_WaveView,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_WaveView_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<WaveView, std::true_type>,
        // method 'startCutMode'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'setCutModeEnabled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'setEraseModeEnabled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'setMarkerAddModeEnabled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'setMarkerSubModeEnabled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'setArrowModeEnabled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'setArrowSubModeEnabled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'signalSampleToPoint'
        QtPrivate::TypeAndForceComplete<QPointF, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'setSelectionModeEnabled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'zoomIn'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'zoomOut'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDocumentChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void WaveView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<WaveView *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->startCutMode(); break;
        case 1: _t->setCutModeEnabled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 2: _t->setEraseModeEnabled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 3: _t->setMarkerAddModeEnabled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 4: _t->setMarkerSubModeEnabled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 5: _t->setArrowModeEnabled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 6: _t->setArrowSubModeEnabled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 7: { QPointF _r = _t->signalSampleToPoint((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])));
            if (_a[0]) *reinterpret_cast< QPointF*>(_a[0]) = std::move(_r); }  break;
        case 8: _t->setSelectionModeEnabled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 9: _t->zoomIn(); break;
        case 10: _t->zoomOut(); break;
        case 11: _t->onDocumentChanged(); break;
        default: ;
        }
    }
}

const QMetaObject *WaveView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WaveView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_WaveView.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int WaveView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 12;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
