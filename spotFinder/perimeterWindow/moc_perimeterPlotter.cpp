/****************************************************************************
** PerimeterPlotter meta object code from reading C++ file 'perimeterPlotter.h'
**
** Created: Tue Mar 13 13:53:57 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.3   edited Aug 5 16:40 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "perimeterPlotter.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *PerimeterPlotter::className() const
{
    return "PerimeterPlotter";
}

QMetaObject *PerimeterPlotter::metaObj = 0;
static QMetaObjectCleanUp cleanUp_PerimeterPlotter( "PerimeterPlotter", &PerimeterPlotter::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString PerimeterPlotter::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "PerimeterPlotter", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString PerimeterPlotter::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "PerimeterPlotter", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* PerimeterPlotter::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    metaObj = QMetaObject::new_metaobject(
	"PerimeterPlotter", parentObject,
	0, 0,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_PerimeterPlotter.setMetaObject( metaObj );
    return metaObj;
}

void* PerimeterPlotter::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "PerimeterPlotter" ) )
	return this;
    return QWidget::qt_cast( clname );
}

bool PerimeterPlotter::qt_invoke( int _id, QUObject* _o )
{
    return QWidget::qt_invoke(_id,_o);
}

bool PerimeterPlotter::qt_emit( int _id, QUObject* _o )
{
    return QWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool PerimeterPlotter::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool PerimeterPlotter::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
