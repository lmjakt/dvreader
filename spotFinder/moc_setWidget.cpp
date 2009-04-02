/****************************************************************************
** SetWidget meta object code from reading C++ file 'setWidget.h'
**
** Created: Tue Mar 13 13:53:49 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.3   edited Aug 5 16:40 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "setWidget.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *SetWidget::className() const
{
    return "SetWidget";
}

QMetaObject *SetWidget::metaObj = 0;
static QMetaObjectCleanUp cleanUp_SetWidget( "SetWidget", &SetWidget::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString SetWidget::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "SetWidget", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString SetWidget::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "SetWidget", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* SetWidget::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = NucleusWidget::staticMetaObject();
    static const QUMethod slot_0 = {"findNuclei", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "findNuclei()", &slot_0, QMetaData::Private }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_0 = {"findSets", 4, param_signal_0 };
    static const QMetaData signal_tbl[] = {
	{ "findSets(int,int,int,float)", &signal_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"SetWidget", parentObject,
	slot_tbl, 1,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_SetWidget.setMetaObject( metaObj );
    return metaObj;
}

void* SetWidget::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "SetWidget" ) )
	return this;
    return NucleusWidget::qt_cast( clname );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL findSets
void SetWidget::findSets( int t0, int t1, int t2, float t3 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 0 );
    if ( !clist )
	return;
    QUObject o[5];
    static_QUType_int.set(o+1,t0);
    static_QUType_int.set(o+2,t1);
    static_QUType_int.set(o+3,t2);
    static_QUType_ptr.set(o+4,&t3);
    activate_signal( clist, o );
}

bool SetWidget::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: findNuclei(); break;
    default:
	return NucleusWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool SetWidget::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: findSets((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(int)static_QUType_int.get(_o+3),(float)(*((float*)static_QUType_ptr.get(_o+4)))); break;
    default:
	return NucleusWidget::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool SetWidget::qt_property( int id, int f, QVariant* v)
{
    return NucleusWidget::qt_property( id, f, v);
}

bool SetWidget::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
