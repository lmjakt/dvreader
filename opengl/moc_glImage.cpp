/****************************************************************************
** GLImage meta object code from reading C++ file 'glImage.h'
**
** Created: Tue Mar 13 13:53:28 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.3   edited Aug 5 16:40 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "glImage.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *GLImage::className() const
{
    return "GLImage";
}

QMetaObject *GLImage::metaObj = 0;
static QMetaObjectCleanUp cleanUp_GLImage( "GLImage", &GLImage::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString GLImage::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "GLImage", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString GLImage::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "GLImage", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* GLImage::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QGLWidget::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "data", &static_QUType_ptr, "float", QUParameter::In },
	{ "x", &static_QUType_int, 0, QUParameter::In },
	{ "y", &static_QUType_int, 0, QUParameter::In },
	{ "col", &static_QUType_int, 0, QUParameter::In },
	{ "row", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"setImage", 5, param_slot_0 };
    static const QUMethod slot_1 = {"setImage", 0, 0 };
    static const QUParameter param_slot_2[] = {
	{ "m", &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_2 = {"setMagnification", 1, param_slot_2 };
    static const QUMethod slot_3 = {"resetMagnification", 0, 0 };
    static const QUMethod slot_4 = {"resetOffsets", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "setImage(float*,int,int,int,int)", &slot_0, QMetaData::Public },
	{ "setImage()", &slot_1, QMetaData::Public },
	{ "setMagnification(float)", &slot_2, QMetaData::Public },
	{ "resetMagnification()", &slot_3, QMetaData::Public },
	{ "resetOffsets()", &slot_4, QMetaData::Public }
    };
    static const QUMethod signal_0 = {"nextImage", 0, 0 };
    static const QUMethod signal_1 = {"previousImage", 0, 0 };
    static const QUParameter param_signal_2[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod signal_2 = {"incrementImage", 1, param_signal_2 };
    static const QUParameter param_signal_3[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod signal_3 = {"offSetsSet", 2, param_signal_3 };
    static const QUParameter param_signal_4[] = {
	{ 0, &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod signal_4 = {"magnificationSet", 1, param_signal_4 };
    static const QUParameter param_signal_5[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod signal_5 = {"newPos", 2, param_signal_5 };
    static const QUParameter param_signal_6[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod signal_6 = {"newLine", 4, param_signal_6 };
    static const QMetaData signal_tbl[] = {
	{ "nextImage()", &signal_0, QMetaData::Private },
	{ "previousImage()", &signal_1, QMetaData::Private },
	{ "incrementImage(int)", &signal_2, QMetaData::Private },
	{ "offSetsSet(int,int)", &signal_3, QMetaData::Private },
	{ "magnificationSet(float)", &signal_4, QMetaData::Private },
	{ "newPos(int,int)", &signal_5, QMetaData::Private },
	{ "newLine(int,int,int,int)", &signal_6, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"GLImage", parentObject,
	slot_tbl, 5,
	signal_tbl, 7,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_GLImage.setMetaObject( metaObj );
    return metaObj;
}

void* GLImage::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "GLImage" ) )
	return this;
    return QGLWidget::qt_cast( clname );
}

// SIGNAL nextImage
void GLImage::nextImage()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

// SIGNAL previousImage
void GLImage::previousImage()
{
    activate_signal( staticMetaObject()->signalOffset() + 1 );
}

// SIGNAL incrementImage
void GLImage::incrementImage( int t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 2, t0 );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL offSetsSet
void GLImage::offSetsSet( int t0, int t1 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 3 );
    if ( !clist )
	return;
    QUObject o[3];
    static_QUType_int.set(o+1,t0);
    static_QUType_int.set(o+2,t1);
    activate_signal( clist, o );
}

// SIGNAL magnificationSet
void GLImage::magnificationSet( float t0 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 4 );
    if ( !clist )
	return;
    QUObject o[2];
    static_QUType_ptr.set(o+1,&t0);
    activate_signal( clist, o );
}

// SIGNAL newPos
void GLImage::newPos( int t0, int t1 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 5 );
    if ( !clist )
	return;
    QUObject o[3];
    static_QUType_int.set(o+1,t0);
    static_QUType_int.set(o+2,t1);
    activate_signal( clist, o );
}

// SIGNAL newLine
void GLImage::newLine( int t0, int t1, int t2, int t3 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 6 );
    if ( !clist )
	return;
    QUObject o[5];
    static_QUType_int.set(o+1,t0);
    static_QUType_int.set(o+2,t1);
    static_QUType_int.set(o+3,t2);
    static_QUType_int.set(o+4,t3);
    activate_signal( clist, o );
}

bool GLImage::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setImage((float*)static_QUType_ptr.get(_o+1),(int)static_QUType_int.get(_o+2),(int)static_QUType_int.get(_o+3),(int)static_QUType_int.get(_o+4),(int)static_QUType_int.get(_o+5)); break;
    case 1: setImage(); break;
    case 2: setMagnification((float)(*((float*)static_QUType_ptr.get(_o+1)))); break;
    case 3: resetMagnification(); break;
    case 4: resetOffsets(); break;
    default:
	return QGLWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool GLImage::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: nextImage(); break;
    case 1: previousImage(); break;
    case 2: incrementImage((int)static_QUType_int.get(_o+1)); break;
    case 3: offSetsSet((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 4: magnificationSet((float)(*((float*)static_QUType_ptr.get(_o+1)))); break;
    case 5: newPos((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 6: newLine((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(int)static_QUType_int.get(_o+3),(int)static_QUType_int.get(_o+4)); break;
    default:
	return QGLWidget::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool GLImage::qt_property( int id, int f, QVariant* v)
{
    return QGLWidget::qt_property( id, f, v);
}

bool GLImage::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
