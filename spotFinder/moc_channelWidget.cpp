/****************************************************************************
** ChannelWidget meta object code from reading C++ file 'channelWidget.h'
**
** Created: Tue Mar 13 13:53:43 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.3   edited Aug 5 16:40 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "channelWidget.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *ChannelWidget::className() const
{
    return "ChannelWidget";
}

QMetaObject *ChannelWidget::metaObj = 0;
static QMetaObjectCleanUp cleanUp_ChannelWidget( "ChannelWidget", &ChannelWidget::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString ChannelWidget::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "ChannelWidget", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString ChannelWidget::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "ChannelWidget", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* ChannelWidget::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUMethod slot_0 = {"findSpots", 0, 0 };
    static const QUMethod slot_1 = {"findAllSpots", 0, 0 };
    static const QUMethod slot_2 = {"pvChanged", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ "me", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"meChanged", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ "sf", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"sfChanged", 1, param_slot_4 };
    static const QUMethod slot_5 = {"changeColor", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "findSpots()", &slot_0, QMetaData::Private },
	{ "findAllSpots()", &slot_1, QMetaData::Private },
	{ "pvChanged()", &slot_2, QMetaData::Private },
	{ "meChanged(int)", &slot_3, QMetaData::Private },
	{ "sfChanged(int)", &slot_4, QMetaData::Private },
	{ "changeColor()", &slot_5, QMetaData::Private }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_0 = {"newPeakValue", 2, param_signal_0 };
    static const QUParameter param_signal_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_1 = {"newMaxEdgeValue", 2, param_signal_1 };
    static const QUParameter param_signal_2[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_2 = {"newScaleFactor", 2, param_signal_2 };
    static const QUParameter param_signal_3[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod signal_3 = {"findspots", 8, param_signal_3 };
    static const QUParameter param_signal_4[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_4 = {"findallspots", 10, param_signal_4 };
    static const QMetaData signal_tbl[] = {
	{ "newPeakValue(int,float)", &signal_0, QMetaData::Private },
	{ "newMaxEdgeValue(int,float)", &signal_1, QMetaData::Private },
	{ "newScaleFactor(int,float)", &signal_2, QMetaData::Private },
	{ "findspots(int,int,float,float,float,int,int,int)", &signal_3, QMetaData::Private },
	{ "findallspots(int,int,float,float,float,int,int,int,int,float)", &signal_4, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"ChannelWidget", parentObject,
	slot_tbl, 6,
	signal_tbl, 5,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_ChannelWidget.setMetaObject( metaObj );
    return metaObj;
}

void* ChannelWidget::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "ChannelWidget" ) )
	return this;
    return QWidget::qt_cast( clname );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL newPeakValue
void ChannelWidget::newPeakValue( int t0, float t1 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 0 );
    if ( !clist )
	return;
    QUObject o[3];
    static_QUType_int.set(o+1,t0);
    static_QUType_ptr.set(o+2,&t1);
    activate_signal( clist, o );
}

// SIGNAL newMaxEdgeValue
void ChannelWidget::newMaxEdgeValue( int t0, float t1 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 1 );
    if ( !clist )
	return;
    QUObject o[3];
    static_QUType_int.set(o+1,t0);
    static_QUType_ptr.set(o+2,&t1);
    activate_signal( clist, o );
}

// SIGNAL newScaleFactor
void ChannelWidget::newScaleFactor( int t0, float t1 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 2 );
    if ( !clist )
	return;
    QUObject o[3];
    static_QUType_int.set(o+1,t0);
    static_QUType_ptr.set(o+2,&t1);
    activate_signal( clist, o );
}

// SIGNAL findspots
void ChannelWidget::findspots( int t0, int t1, float t2, float t3, float t4, int t5, int t6, int t7 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 3 );
    if ( !clist )
	return;
    QUObject o[9];
    static_QUType_int.set(o+1,t0);
    static_QUType_int.set(o+2,t1);
    static_QUType_ptr.set(o+3,&t2);
    static_QUType_ptr.set(o+4,&t3);
    static_QUType_ptr.set(o+5,&t4);
    static_QUType_int.set(o+6,t5);
    static_QUType_int.set(o+7,t6);
    static_QUType_int.set(o+8,t7);
    activate_signal( clist, o );
}

// SIGNAL findallspots
void ChannelWidget::findallspots( int t0, int t1, float t2, float t3, float t4, int t5, int t6, int t7, int t8, float t9 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 4 );
    if ( !clist )
	return;
    QUObject o[11];
    static_QUType_int.set(o+1,t0);
    static_QUType_int.set(o+2,t1);
    static_QUType_ptr.set(o+3,&t2);
    static_QUType_ptr.set(o+4,&t3);
    static_QUType_ptr.set(o+5,&t4);
    static_QUType_int.set(o+6,t5);
    static_QUType_int.set(o+7,t6);
    static_QUType_int.set(o+8,t7);
    static_QUType_int.set(o+9,t8);
    static_QUType_ptr.set(o+10,&t9);
    activate_signal( clist, o );
}

bool ChannelWidget::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: findSpots(); break;
    case 1: findAllSpots(); break;
    case 2: pvChanged(); break;
    case 3: meChanged((int)static_QUType_int.get(_o+1)); break;
    case 4: sfChanged((int)static_QUType_int.get(_o+1)); break;
    case 5: changeColor(); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool ChannelWidget::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: newPeakValue((int)static_QUType_int.get(_o+1),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 1: newMaxEdgeValue((int)static_QUType_int.get(_o+1),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 2: newScaleFactor((int)static_QUType_int.get(_o+1),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 3: findspots((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(float)(*((float*)static_QUType_ptr.get(_o+3))),(float)(*((float*)static_QUType_ptr.get(_o+4))),(float)(*((float*)static_QUType_ptr.get(_o+5))),(int)static_QUType_int.get(_o+6),(int)static_QUType_int.get(_o+7),(int)static_QUType_int.get(_o+8)); break;
    case 4: findallspots((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(float)(*((float*)static_QUType_ptr.get(_o+3))),(float)(*((float*)static_QUType_ptr.get(_o+4))),(float)(*((float*)static_QUType_ptr.get(_o+5))),(int)static_QUType_int.get(_o+6),(int)static_QUType_int.get(_o+7),(int)static_QUType_int.get(_o+8),(int)static_QUType_int.get(_o+9),(float)(*((float*)static_QUType_ptr.get(_o+10)))); break;
    default:
	return QWidget::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool ChannelWidget::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool ChannelWidget::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
