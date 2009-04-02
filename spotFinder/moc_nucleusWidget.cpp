/****************************************************************************
** NucleusWidget meta object code from reading C++ file 'nucleusWidget.h'
**
** Created: Tue Mar 13 13:53:47 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.3   edited Aug 5 16:40 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "nucleusWidget.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *NucleusWidget::className() const
{
    return "NucleusWidget";
}

QMetaObject *NucleusWidget::metaObj = 0;
static QMetaObjectCleanUp cleanUp_NucleusWidget( "NucleusWidget", &NucleusWidget::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString NucleusWidget::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "NucleusWidget", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString NucleusWidget::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "NucleusWidget", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* NucleusWidget::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUMethod slot_0 = {"findNuclei", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "findNuclei()", &slot_0, QMetaData::Protected }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_0 = {"findNuclearPerimeters", 2, param_signal_0 };
    static const QMetaData signal_tbl[] = {
	{ "findNuclearPerimeters(int,float)", &signal_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"NucleusWidget", parentObject,
	slot_tbl, 1,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_NucleusWidget.setMetaObject( metaObj );
    return metaObj;
}

void* NucleusWidget::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "NucleusWidget" ) )
	return this;
    return QWidget::qt_cast( clname );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL findNuclearPerimeters
void NucleusWidget::findNuclearPerimeters( int t0, float t1 )
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

bool NucleusWidget::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: findNuclei(); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool NucleusWidget::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: findNuclearPerimeters((int)static_QUType_int.get(_o+1),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    default:
	return QWidget::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool NucleusWidget::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool NucleusWidget::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
