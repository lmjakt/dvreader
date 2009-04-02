/****************************************************************************
** OverlapViewer meta object code from reading C++ file 'overlapViewer.h'
**
** Created: Tue Mar 13 13:54:13 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.3   edited Aug 5 16:40 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "overlapViewer.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *OverlapViewer::className() const
{
    return "OverlapViewer";
}

QMetaObject *OverlapViewer::metaObj = 0;
static QMetaObjectCleanUp cleanUp_OverlapViewer( "OverlapViewer", &OverlapViewer::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString OverlapViewer::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "OverlapViewer", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString OverlapViewer::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "OverlapViewer", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* OverlapViewer::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "delta_x", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"set_dx", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ "delta_y", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"set_dy", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ "newScale", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"setScale", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ "newBias", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"setBias", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ "on", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"paintA", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ "on", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"paintB", 1, param_slot_5 };
    static const QMetaData slot_tbl[] = {
	{ "set_dx(int)", &slot_0, QMetaData::Private },
	{ "set_dy(int)", &slot_1, QMetaData::Private },
	{ "setScale(int)", &slot_2, QMetaData::Private },
	{ "setBias(int)", &slot_3, QMetaData::Private },
	{ "paintA(bool)", &slot_4, QMetaData::Private },
	{ "paintB(bool)", &slot_5, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"OverlapViewer", parentObject,
	slot_tbl, 6,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_OverlapViewer.setMetaObject( metaObj );
    return metaObj;
}

void* OverlapViewer::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "OverlapViewer" ) )
	return this;
    return QWidget::qt_cast( clname );
}

bool OverlapViewer::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: set_dx((int)static_QUType_int.get(_o+1)); break;
    case 1: set_dy((int)static_QUType_int.get(_o+1)); break;
    case 2: setScale((int)static_QUType_int.get(_o+1)); break;
    case 3: setBias((int)static_QUType_int.get(_o+1)); break;
    case 4: paintA((bool)static_QUType_bool.get(_o+1)); break;
    case 5: paintB((bool)static_QUType_bool.get(_o+1)); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool OverlapViewer::qt_emit( int _id, QUObject* _o )
{
    return QWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool OverlapViewer::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool OverlapViewer::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
