/****************************************************************************
** JpgView meta object code from reading C++ file 'jpgView.h'
**
** Created: Tue Mar 13 13:53:27 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.3   edited Aug 5 16:40 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "jpgView.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *JpgView::className() const
{
    return "JpgView";
}

QMetaObject *JpgView::metaObj = 0;
static QMetaObjectCleanUp cleanUp_JpgView( "JpgView", &JpgView::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString JpgView::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "JpgView", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString JpgView::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "JpgView", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* JpgView::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "data", &static_QUType_varptr, "\x1d", QUParameter::In }
    };
    static const QUMethod slot_0 = {"setImage", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ "img", &static_QUType_varptr, "\x0f", QUParameter::In }
    };
    static const QUMethod slot_1 = {"setImage", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ "f", &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_2 = {"scale", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ "infile", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"setImageFromFile", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ "label", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"setLabel", 1, param_slot_4 };
    static const QMetaData slot_tbl[] = {
	{ "setImage(QByteArray)", &slot_0, QMetaData::Public },
	{ "setImage(QImage*)", &slot_1, QMetaData::Public },
	{ "scale(float)", &slot_2, QMetaData::Public },
	{ "setImageFromFile(QString)", &slot_3, QMetaData::Public },
	{ "setLabel(QString)", &slot_4, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"JpgView", parentObject,
	slot_tbl, 5,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_JpgView.setMetaObject( metaObj );
    return metaObj;
}

void* JpgView::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "JpgView" ) )
	return this;
    return QWidget::qt_cast( clname );
}

bool JpgView::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setImage((QByteArray)(*((QByteArray*)static_QUType_ptr.get(_o+1)))); break;
    case 1: setImage((QImage*)static_QUType_varptr.get(_o+1)); break;
    case 2: scale((float)(*((float*)static_QUType_ptr.get(_o+1)))); break;
    case 3: setImageFromFile((QString)static_QUType_QString.get(_o+1)); break;
    case 4: setLabel((QString)static_QUType_QString.get(_o+1)); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool JpgView::qt_emit( int _id, QUObject* _o )
{
    return QWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool JpgView::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool JpgView::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
