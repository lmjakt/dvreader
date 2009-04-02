/****************************************************************************
** DistChooser meta object code from reading C++ file 'distChooser.h'
**
** Created: Tue Mar 13 13:53:30 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.3   edited Aug 5 16:40 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "distChooser.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *DistChooser::className() const
{
    return "DistChooser";
}

QMetaObject *DistChooser::metaObj = 0;
static QMetaObjectCleanUp cleanUp_DistChooser( "DistChooser", &DistChooser::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString DistChooser::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "DistChooser", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString DistChooser::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "DistChooser", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* DistChooser::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "vf", &static_QUType_ptr, "vector<float>", QUParameter::In },
	{ "minV", &static_QUType_ptr, "float", QUParameter::In },
	{ "maxV", &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_0 = {"setData", 3, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ "vf", &static_QUType_ptr, "vector<float>", QUParameter::In },
	{ "minV", &static_QUType_ptr, "float", QUParameter::In },
	{ "maxV", &static_QUType_ptr, "float", QUParameter::In },
	{ "updateThresholds", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"setData", 4, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ "as", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"setAxis", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ "dmin", &static_QUType_ptr, "float", QUParameter::In },
	{ "dmax", &static_QUType_ptr, "float", QUParameter::In },
	{ "tmin", &static_QUType_ptr, "float", QUParameter::In },
	{ "tmax", &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_3 = {"setParams", 4, param_slot_3 };
    static const QUMethod slot_4 = {"saveValues", 0, 0 };
    static const QUMethod slot_5 = {"zoomToSelection", 0, 0 };
    static const QUMethod slot_6 = {"zoomOut", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "setData(vector<float>,float,float)", &slot_0, QMetaData::Public },
	{ "setData(vector<float>,float,float,bool)", &slot_1, QMetaData::Public },
	{ "setAxis(int)", &slot_2, QMetaData::Public },
	{ "setParams(float,float,float,float)", &slot_3, QMetaData::Public },
	{ "saveValues()", &slot_4, QMetaData::Private },
	{ "zoomToSelection()", &slot_5, QMetaData::Private },
	{ "zoomOut()", &slot_6, QMetaData::Private }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_0 = {"newRanges", 2, param_signal_0 };
    static const QUParameter param_signal_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod signal_1 = {"axisWish", 1, param_signal_1 };
    static const QUMethod signal_2 = {"copyRanges", 0, 0 };
    static const QUMethod signal_3 = {"pasteRanges", 0, 0 };
    static const QUMethod signal_4 = {"saveRanges", 0, 0 };
    static const QUMethod signal_5 = {"readRangesFromFile", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "newRanges(float,float)", &signal_0, QMetaData::Private },
	{ "axisWish(int)", &signal_1, QMetaData::Private },
	{ "copyRanges()", &signal_2, QMetaData::Private },
	{ "pasteRanges()", &signal_3, QMetaData::Private },
	{ "saveRanges()", &signal_4, QMetaData::Private },
	{ "readRangesFromFile()", &signal_5, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"DistChooser", parentObject,
	slot_tbl, 7,
	signal_tbl, 6,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_DistChooser.setMetaObject( metaObj );
    return metaObj;
}

void* DistChooser::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "DistChooser" ) )
	return this;
    return QWidget::qt_cast( clname );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL newRanges
void DistChooser::newRanges( float t0, float t1 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 0 );
    if ( !clist )
	return;
    QUObject o[3];
    static_QUType_ptr.set(o+1,&t0);
    static_QUType_ptr.set(o+2,&t1);
    activate_signal( clist, o );
}

// SIGNAL axisWish
void DistChooser::axisWish( int t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 1, t0 );
}

// SIGNAL copyRanges
void DistChooser::copyRanges()
{
    activate_signal( staticMetaObject()->signalOffset() + 2 );
}

// SIGNAL pasteRanges
void DistChooser::pasteRanges()
{
    activate_signal( staticMetaObject()->signalOffset() + 3 );
}

// SIGNAL saveRanges
void DistChooser::saveRanges()
{
    activate_signal( staticMetaObject()->signalOffset() + 4 );
}

// SIGNAL readRangesFromFile
void DistChooser::readRangesFromFile()
{
    activate_signal( staticMetaObject()->signalOffset() + 5 );
}

bool DistChooser::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setData((vector<float>)(*((vector<float>*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2))),(float)(*((float*)static_QUType_ptr.get(_o+3)))); break;
    case 1: setData((vector<float>)(*((vector<float>*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2))),(float)(*((float*)static_QUType_ptr.get(_o+3))),(bool)static_QUType_bool.get(_o+4)); break;
    case 2: setAxis((int)static_QUType_int.get(_o+1)); break;
    case 3: setParams((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2))),(float)(*((float*)static_QUType_ptr.get(_o+3))),(float)(*((float*)static_QUType_ptr.get(_o+4)))); break;
    case 4: saveValues(); break;
    case 5: zoomToSelection(); break;
    case 6: zoomOut(); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool DistChooser::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: newRanges((float)(*((float*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 1: axisWish((int)static_QUType_int.get(_o+1)); break;
    case 2: copyRanges(); break;
    case 3: pasteRanges(); break;
    case 4: saveRanges(); break;
    case 5: readRangesFromFile(); break;
    default:
	return QWidget::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool DistChooser::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool DistChooser::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
