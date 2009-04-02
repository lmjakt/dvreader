/****************************************************************************
** PointViewWidget meta object code from reading C++ file 'pointViewWidget.h'
**
** Created: Tue Mar 13 13:54:01 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.3   edited Aug 5 16:40 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "pointViewWidget.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *PointViewWidget::className() const
{
    return "PointViewWidget";
}

QMetaObject *PointViewWidget::metaObj = 0;
static QMetaObjectCleanUp cleanUp_PointViewWidget( "PointViewWidget", &PointViewWidget::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString PointViewWidget::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "PointViewWidget", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString PointViewWidget::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "PointViewWidget", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* PointViewWidget::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QGLWidget::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "volume", &static_QUType_ptr, "voxelVolume", QUParameter::InOut }
    };
    static const QUMethod slot_0 = {"setModel", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ "w", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"generateNullModel", 1, param_slot_1 };
    static const QMetaData slot_tbl[] = {
	{ "setModel(voxelVolume&)", &slot_0, QMetaData::Public },
	{ "generateNullModel(int)", &slot_1, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"PointViewWidget", parentObject,
	slot_tbl, 2,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_PointViewWidget.setMetaObject( metaObj );
    return metaObj;
}

void* PointViewWidget::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "PointViewWidget" ) )
	return this;
    return QGLWidget::qt_cast( clname );
}

bool PointViewWidget::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setModel((voxelVolume&)*((voxelVolume*)static_QUType_ptr.get(_o+1))); break;
    case 1: generateNullModel((int)static_QUType_int.get(_o+1)); break;
    default:
	return QGLWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool PointViewWidget::qt_emit( int _id, QUObject* _o )
{
    return QGLWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool PointViewWidget::qt_property( int id, int f, QVariant* v)
{
    return QGLWidget::qt_property( id, f, v);
}

bool PointViewWidget::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
