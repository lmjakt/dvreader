/****************************************************************************
** FlatView meta object code from reading C++ file 'flatView.h'
**
** Created: Tue Mar 13 13:54:06 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.3   edited Aug 5 16:40 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "flatView.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *FlatView::className() const
{
    return "FlatView";
}

QMetaObject *FlatView::metaObj = 0;
static QMetaObjectCleanUp cleanUp_FlatView( "FlatView", &FlatView::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString FlatView::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "FlatView", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString FlatView::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "FlatView", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* FlatView::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "clusters", &static_QUType_ptr, "vector<Cluster>", QUParameter::InOut }
    };
    static const QUMethod slot_0 = {"setClusterDrops", 1, param_slot_0 };
    static const QMetaData slot_tbl[] = {
	{ "setClusterDrops(vector<Cluster>&)", &slot_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"FlatView", parentObject,
	slot_tbl, 1,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_FlatView.setMetaObject( metaObj );
    return metaObj;
}

void* FlatView::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "FlatView" ) )
	return this;
    return QWidget::qt_cast( clname );
}

bool FlatView::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setClusterDrops((vector<Cluster>&)*((vector<Cluster>*)static_QUType_ptr.get(_o+1))); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool FlatView::qt_emit( int _id, QUObject* _o )
{
    return QWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool FlatView::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool FlatView::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
