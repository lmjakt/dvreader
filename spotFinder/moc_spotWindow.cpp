/****************************************************************************
** SpotWindow meta object code from reading C++ file 'spotWindow.h'
**
** Created: Tue Mar 13 13:53:40 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.3   edited Aug 5 16:40 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "spotWindow.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *SpotWindow::className() const
{
    return "SpotWindow";
}

QMetaObject *SpotWindow::metaObj = 0;
static QMetaObjectCleanUp cleanUp_SpotWindow( "SpotWindow", &SpotWindow::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString SpotWindow::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "SpotWindow", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString SpotWindow::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "SpotWindow", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* SpotWindow::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "id", &static_QUType_int, 0, QUParameter::In },
	{ "pv", &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_0 = {"newPeakValue", 2, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ "id", &static_QUType_int, 0, QUParameter::In },
	{ "ev", &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_1 = {"newMaxEdgeValue", 2, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ "id", &static_QUType_int, 0, QUParameter::In },
	{ "sf", &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_2 = {"newScaleFactor", 2, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ "id", &static_QUType_int, 0, QUParameter::In },
	{ "wsize", &static_QUType_int, 0, QUParameter::In },
	{ "minPeakValue", &static_QUType_ptr, "float", QUParameter::In },
	{ "maxEdgeValue", &static_QUType_ptr, "float", QUParameter::In },
	{ "minCorrelation", &static_QUType_ptr, "float", QUParameter::In },
	{ "r", &static_QUType_int, 0, QUParameter::In },
	{ "g", &static_QUType_int, 0, QUParameter::In },
	{ "b", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"findSpots", 8, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ "id", &static_QUType_int, 0, QUParameter::In },
	{ "wsize", &static_QUType_int, 0, QUParameter::In },
	{ "minPeakValue", &static_QUType_ptr, "float", QUParameter::In },
	{ "maxEdgeValue", &static_QUType_ptr, "float", QUParameter::In },
	{ "minCorrelation", &static_QUType_ptr, "float", QUParameter::In },
	{ "r", &static_QUType_int, 0, QUParameter::In },
	{ "g", &static_QUType_int, 0, QUParameter::In },
	{ "b", &static_QUType_int, 0, QUParameter::In },
	{ "K", &static_QUType_int, 0, QUParameter::In },
	{ "bgm", &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_4 = {"findAllSpots", 10, param_slot_4 };
    static const QMetaData slot_tbl[] = {
	{ "newPeakValue(int,float)", &slot_0, QMetaData::Private },
	{ "newMaxEdgeValue(int,float)", &slot_1, QMetaData::Private },
	{ "newScaleFactor(int,float)", &slot_2, QMetaData::Private },
	{ "findSpots(int,int,float,float,float,int,int,int)", &slot_3, QMetaData::Private },
	{ "findAllSpots(int,int,float,float,float,int,int,int,int,float)", &slot_4, QMetaData::Private }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_0 = {"findLocalMaxima", 4, param_signal_0 };
    static const QUParameter param_signal_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_1 = {"findAllLocalMaxima", 6, param_signal_1 };
    static const QUParameter param_signal_2[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod signal_2 = {"increment_x_line", 1, param_signal_2 };
    static const QUParameter param_signal_3[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod signal_3 = {"increment_y_line", 1, param_signal_3 };
    static const QUParameter param_signal_4[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod signal_4 = {"setUseProjection", 1, param_signal_4 };
    static const QUParameter param_signal_5[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "set<int>", QUParameter::In }
    };
    static const QUMethod signal_5 = {"makeModel", 7, param_signal_5 };
    static const QUMethod signal_6 = {"recalculateSpotVolumes", 0, 0 };
    static const QUParameter param_signal_7[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_7 = {"findNuclearPerimeters", 2, param_signal_7 };
    static const QUParameter param_signal_8[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_8 = {"findContrasts", 2, param_signal_8 };
    static const QUParameter param_signal_9[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_9 = {"findSets", 4, param_signal_9 };
    static const QMetaData signal_tbl[] = {
	{ "findLocalMaxima(int,int,float,float)", &signal_0, QMetaData::Private },
	{ "findAllLocalMaxima(int,int,float,float,int,float)", &signal_1, QMetaData::Private },
	{ "increment_x_line(int)", &signal_2, QMetaData::Private },
	{ "increment_y_line(int)", &signal_3, QMetaData::Private },
	{ "setUseProjection(bool)", &signal_4, QMetaData::Private },
	{ "makeModel(int,int,int,int,int,int,set<int>)", &signal_5, QMetaData::Private },
	{ "recalculateSpotVolumes()", &signal_6, QMetaData::Private },
	{ "findNuclearPerimeters(int,float)", &signal_7, QMetaData::Private },
	{ "findContrasts(int,float)", &signal_8, QMetaData::Private },
	{ "findSets(int,int,int,float)", &signal_9, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"SpotWindow", parentObject,
	slot_tbl, 5,
	signal_tbl, 10,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_SpotWindow.setMetaObject( metaObj );
    return metaObj;
}

void* SpotWindow::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "SpotWindow" ) )
	return this;
    return QWidget::qt_cast( clname );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL findLocalMaxima
void SpotWindow::findLocalMaxima( int t0, int t1, float t2, float t3 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 0 );
    if ( !clist )
	return;
    QUObject o[5];
    static_QUType_int.set(o+1,t0);
    static_QUType_int.set(o+2,t1);
    static_QUType_ptr.set(o+3,&t2);
    static_QUType_ptr.set(o+4,&t3);
    activate_signal( clist, o );
}

// SIGNAL findAllLocalMaxima
void SpotWindow::findAllLocalMaxima( int t0, int t1, float t2, float t3, int t4, float t5 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 1 );
    if ( !clist )
	return;
    QUObject o[7];
    static_QUType_int.set(o+1,t0);
    static_QUType_int.set(o+2,t1);
    static_QUType_ptr.set(o+3,&t2);
    static_QUType_ptr.set(o+4,&t3);
    static_QUType_int.set(o+5,t4);
    static_QUType_ptr.set(o+6,&t5);
    activate_signal( clist, o );
}

// SIGNAL increment_x_line
void SpotWindow::increment_x_line( int t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 2, t0 );
}

// SIGNAL increment_y_line
void SpotWindow::increment_y_line( int t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 3, t0 );
}

// SIGNAL setUseProjection
void SpotWindow::setUseProjection( bool t0 )
{
    activate_signal_bool( staticMetaObject()->signalOffset() + 4, t0 );
}

// SIGNAL makeModel
void SpotWindow::makeModel( int t0, int t1, int t2, int t3, int t4, int t5, set<int> t6 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 5 );
    if ( !clist )
	return;
    QUObject o[8];
    static_QUType_int.set(o+1,t0);
    static_QUType_int.set(o+2,t1);
    static_QUType_int.set(o+3,t2);
    static_QUType_int.set(o+4,t3);
    static_QUType_int.set(o+5,t4);
    static_QUType_int.set(o+6,t5);
    static_QUType_ptr.set(o+7,&t6);
    activate_signal( clist, o );
}

// SIGNAL recalculateSpotVolumes
void SpotWindow::recalculateSpotVolumes()
{
    activate_signal( staticMetaObject()->signalOffset() + 6 );
}

// SIGNAL findNuclearPerimeters
void SpotWindow::findNuclearPerimeters( int t0, float t1 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 7 );
    if ( !clist )
	return;
    QUObject o[3];
    static_QUType_int.set(o+1,t0);
    static_QUType_ptr.set(o+2,&t1);
    activate_signal( clist, o );
}

// SIGNAL findContrasts
void SpotWindow::findContrasts( int t0, float t1 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 8 );
    if ( !clist )
	return;
    QUObject o[3];
    static_QUType_int.set(o+1,t0);
    static_QUType_ptr.set(o+2,&t1);
    activate_signal( clist, o );
}

// SIGNAL findSets
void SpotWindow::findSets( int t0, int t1, int t2, float t3 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 9 );
    if ( !clist )
	return;
    QUObject o[5];
    static_QUType_int.set(o+1,t0);
    static_QUType_int.set(o+2,t1);
    static_QUType_int.set(o+3,t2);
    static_QUType_ptr.set(o+4,&t3);
    activate_signal( clist, o );
}

bool SpotWindow::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: newPeakValue((int)static_QUType_int.get(_o+1),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 1: newMaxEdgeValue((int)static_QUType_int.get(_o+1),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 2: newScaleFactor((int)static_QUType_int.get(_o+1),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 3: findSpots((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(float)(*((float*)static_QUType_ptr.get(_o+3))),(float)(*((float*)static_QUType_ptr.get(_o+4))),(float)(*((float*)static_QUType_ptr.get(_o+5))),(int)static_QUType_int.get(_o+6),(int)static_QUType_int.get(_o+7),(int)static_QUType_int.get(_o+8)); break;
    case 4: findAllSpots((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(float)(*((float*)static_QUType_ptr.get(_o+3))),(float)(*((float*)static_QUType_ptr.get(_o+4))),(float)(*((float*)static_QUType_ptr.get(_o+5))),(int)static_QUType_int.get(_o+6),(int)static_QUType_int.get(_o+7),(int)static_QUType_int.get(_o+8),(int)static_QUType_int.get(_o+9),(float)(*((float*)static_QUType_ptr.get(_o+10)))); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool SpotWindow::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: findLocalMaxima((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(float)(*((float*)static_QUType_ptr.get(_o+3))),(float)(*((float*)static_QUType_ptr.get(_o+4)))); break;
    case 1: findAllLocalMaxima((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(float)(*((float*)static_QUType_ptr.get(_o+3))),(float)(*((float*)static_QUType_ptr.get(_o+4))),(int)static_QUType_int.get(_o+5),(float)(*((float*)static_QUType_ptr.get(_o+6)))); break;
    case 2: increment_x_line((int)static_QUType_int.get(_o+1)); break;
    case 3: increment_y_line((int)static_QUType_int.get(_o+1)); break;
    case 4: setUseProjection((bool)static_QUType_bool.get(_o+1)); break;
    case 5: makeModel((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(int)static_QUType_int.get(_o+3),(int)static_QUType_int.get(_o+4),(int)static_QUType_int.get(_o+5),(int)static_QUType_int.get(_o+6),(set<int>)(*((set<int>*)static_QUType_ptr.get(_o+7)))); break;
    case 6: recalculateSpotVolumes(); break;
    case 7: findNuclearPerimeters((int)static_QUType_int.get(_o+1),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 8: findContrasts((int)static_QUType_int.get(_o+1),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 9: findSets((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(int)static_QUType_int.get(_o+3),(float)(*((float*)static_QUType_ptr.get(_o+4)))); break;
    default:
	return QWidget::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool SpotWindow::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool SpotWindow::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
