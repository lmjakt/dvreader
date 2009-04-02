/****************************************************************************
** DropButton meta object code from reading C++ file 'dropButton.h'
**
** Created: Tue Mar 13 13:54:05 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.3   edited Aug 5 16:40 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "dropButton.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *DropButton::className() const
{
    return "DropButton";
}

QMetaObject *DropButton::metaObj = 0;
static QMetaObjectCleanUp cleanUp_DropButton( "DropButton", &DropButton::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString DropButton::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "DropButton", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString DropButton::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "DropButton", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* DropButton::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QButton::staticMetaObject();
    metaObj = QMetaObject::new_metaobject(
	"DropButton", parentObject,
	0, 0,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_DropButton.setMetaObject( metaObj );
    return metaObj;
}

void* DropButton::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "DropButton" ) )
	return this;
    return QButton::qt_cast( clname );
}

bool DropButton::qt_invoke( int _id, QUObject* _o )
{
    return QButton::qt_invoke(_id,_o);
}

bool DropButton::qt_emit( int _id, QUObject* _o )
{
    return QButton::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool DropButton::qt_property( int id, int f, QVariant* v)
{
    return QButton::qt_property( id, f, v);
}

bool DropButton::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
