/****************************************************************************
** SpotsWidget meta object code from reading C++ file 'spotsWidget.h'
**
** Created: Tue Mar 13 13:53:55 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.3   edited Aug 5 16:40 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "spotsWidget.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *SpotsWidget::className() const
{
    return "SpotsWidget";
}

QMetaObject *SpotsWidget::metaObj = 0;
static QMetaObjectCleanUp cleanUp_SpotsWidget( "SpotsWidget", &SpotsWidget::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString SpotsWidget::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "SpotsWidget", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString SpotsWidget::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "SpotsWidget", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* SpotsWidget::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUMethod slot_0 = {"setColor", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ "on", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"repButtonToggled", 1, param_slot_1 };
    static const QMetaData slot_tbl[] = {
	{ "setColor()", &slot_0, QMetaData::Private },
	{ "repButtonToggled(bool)", &slot_1, QMetaData::Private }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_0 = {"colorChanged", 4, param_signal_0 };
    static const QUParameter param_signal_1[] = {
	{ 0, &static_QUType_ptr, "DropRepresentation", QUParameter::In }
    };
    static const QUMethod signal_1 = {"repTypeChanged", 1, param_signal_1 };
    static const QMetaData signal_tbl[] = {
	{ "colorChanged(int,float,float,float)", &signal_0, QMetaData::Private },
	{ "repTypeChanged(DropRepresentation)", &signal_1, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"SpotsWidget", parentObject,
	slot_tbl, 2,
	signal_tbl, 2,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_SpotsWidget.setMetaObject( metaObj );
    return metaObj;
}

void* SpotsWidget::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "SpotsWidget" ) )
	return this;
    return QWidget::qt_cast( clname );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL colorChanged
void SpotsWidget::colorChanged( int t0, float t1, float t2, float t3 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 0 );
    if ( !clist )
	return;
    QUObject o[5];
    static_QUType_int.set(o+1,t0);
    static_QUType_ptr.set(o+2,&t1);
    static_QUType_ptr.set(o+3,&t2);
    static_QUType_ptr.set(o+4,&t3);
    activate_signal( clist, o );
}

// SIGNAL repTypeChanged
void SpotsWidget::repTypeChanged( DropRepresentation t0 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 1 );
    if ( !clist )
	return;
    QUObject o[2];
    static_QUType_ptr.set(o+1,&t0);
    activate_signal( clist, o );
}

bool SpotsWidget::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setColor(); break;
    case 1: repButtonToggled((bool)static_QUType_bool.get(_o+1)); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool SpotsWidget::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: colorChanged((int)static_QUType_int.get(_o+1),(float)(*((float*)static_QUType_ptr.get(_o+2))),(float)(*((float*)static_QUType_ptr.get(_o+3))),(float)(*((float*)static_QUType_ptr.get(_o+4)))); break;
    case 1: repTypeChanged((DropRepresentation)(*((DropRepresentation*)static_QUType_ptr.get(_o+1)))); break;
    default:
	return QWidget::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool SpotsWidget::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool SpotsWidget::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
