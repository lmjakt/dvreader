/****************************************************************************
** SpotSectionView meta object code from reading C++ file 'spotSectionView.h'
**
** Created: Tue Mar 13 13:53:51 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.3   edited Aug 5 16:40 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "spotSectionView.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *SpotSectionView::className() const
{
    return "SpotSectionView";
}

QMetaObject *SpotSectionView::metaObj = 0;
static QMetaObjectCleanUp cleanUp_SpotSectionView( "SpotSectionView", &SpotSectionView::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString SpotSectionView::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "SpotSectionView", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString SpotSectionView::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "SpotSectionView", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* SpotSectionView::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    metaObj = QMetaObject::new_metaobject(
	"SpotSectionView", parentObject,
	0, 0,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_SpotSectionView.setMetaObject( metaObj );
    return metaObj;
}

void* SpotSectionView::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "SpotSectionView" ) )
	return this;
    return QWidget::qt_cast( clname );
}

bool SpotSectionView::qt_invoke( int _id, QUObject* _o )
{
    return QWidget::qt_invoke(_id,_o);
}

bool SpotSectionView::qt_emit( int _id, QUObject* _o )
{
    return QWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool SpotSectionView::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool SpotSectionView::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
