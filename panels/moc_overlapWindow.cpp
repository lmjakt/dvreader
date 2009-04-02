/****************************************************************************
** OverlapWindow meta object code from reading C++ file 'overlapWindow.h'
**
** Created: Tue Mar 13 13:54:15 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.3   edited Aug 5 16:40 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "overlapWindow.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *OverlapWindow::className() const
{
    return "OverlapWindow";
}

QMetaObject *OverlapWindow::metaObj = 0;
static QMetaObjectCleanUp cleanUp_OverlapWindow( "OverlapWindow", &OverlapWindow::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString OverlapWindow::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "OverlapWindow", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString OverlapWindow::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "OverlapWindow", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* OverlapWindow::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "n", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"setOverlap", 1, param_slot_0 };
    static const QMetaData slot_tbl[] = {
	{ "setOverlap(int)", &slot_0, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"OverlapWindow", parentObject,
	slot_tbl, 1,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_OverlapWindow.setMetaObject( metaObj );
    return metaObj;
}

void* OverlapWindow::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "OverlapWindow" ) )
	return this;
    return QWidget::qt_cast( clname );
}

bool OverlapWindow::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setOverlap((int)static_QUType_int.get(_o+1)); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool OverlapWindow::qt_emit( int _id, QUObject* _o )
{
    return QWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool OverlapWindow::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool OverlapWindow::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
