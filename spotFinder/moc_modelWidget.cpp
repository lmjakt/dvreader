/****************************************************************************
** ModelWidget meta object code from reading C++ file 'modelWidget.h'
**
** Created: Tue Mar 13 13:53:45 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.3   edited Aug 5 16:40 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "modelWidget.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *ModelWidget::className() const
{
    return "ModelWidget";
}

QMetaObject *ModelWidget::metaObj = 0;
static QMetaObjectCleanUp cleanUp_ModelWidget( "ModelWidget", &ModelWidget::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString ModelWidget::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "ModelWidget", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString ModelWidget::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "ModelWidget", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* ModelWidget::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUMethod slot_0 = {"requestModel", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "requestModel()", &slot_0, QMetaData::Private }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "set<int>", QUParameter::In }
    };
    static const QUMethod signal_0 = {"makeModel", 7, param_signal_0 };
    static const QUMethod signal_1 = {"recalculateSpotVolumes", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "makeModel(int,int,int,int,int,int,set<int>)", &signal_0, QMetaData::Public },
	{ "recalculateSpotVolumes()", &signal_1, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"ModelWidget", parentObject,
	slot_tbl, 1,
	signal_tbl, 2,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_ModelWidget.setMetaObject( metaObj );
    return metaObj;
}

void* ModelWidget::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "ModelWidget" ) )
	return this;
    return QWidget::qt_cast( clname );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL makeModel
void ModelWidget::makeModel( int t0, int t1, int t2, int t3, int t4, int t5, set<int> t6 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 0 );
    if ( !clist )
	return;
    QUObject o[8];
    static_QUType_int.set(o+1,t0);
    static_QUType_int.set(o+2,t1);
    static_QUType_int.set(o+3,t2);
    static_QUType_int.set(o+4,t3);
    static_QUType_int.set(o+5,t4);
    static_QUType_int.set(o+6,t5);
    static_QUType_ptr.set(o+7,&t6);
    activate_signal( clist, o );
}

// SIGNAL recalculateSpotVolumes
void ModelWidget::recalculateSpotVolumes()
{
    activate_signal( staticMetaObject()->signalOffset() + 1 );
}

bool ModelWidget::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: requestModel(); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool ModelWidget::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: makeModel((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(int)static_QUType_int.get(_o+3),(int)static_QUType_int.get(_o+4),(int)static_QUType_int.get(_o+5),(int)static_QUType_int.get(_o+6),(set<int>)(*((set<int>*)static_QUType_ptr.get(_o+7)))); break;
    case 1: recalculateSpotVolumes(); break;
    default:
	return QWidget::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool ModelWidget::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool ModelWidget::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
