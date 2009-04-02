/****************************************************************************
** ClusterWidget meta object code from reading C++ file 'clusterWidget.h'
**
** Created: Tue Mar 13 13:54:08 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.3   edited Aug 5 16:40 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "clusterWidget.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *ClusterWidget::className() const
{
    return "ClusterWidget";
}

QMetaObject *ClusterWidget::metaObj = 0;
static QMetaObjectCleanUp cleanUp_ClusterWidget( "ClusterWidget", &ClusterWidget::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString ClusterWidget::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "ClusterWidget", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString ClusterWidget::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "ClusterWidget", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* ClusterWidget::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "on", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"plot_members", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ "on", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"plot_center", 1, param_slot_1 };
    static const QMetaData slot_tbl[] = {
	{ "plot_members(bool)", &slot_0, QMetaData::Private },
	{ "plot_center(bool)", &slot_1, QMetaData::Private }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<vector<float>>", QUParameter::In }
    };
    static const QUMethod signal_0 = {"plotMembers", 2, param_signal_0 };
    static const QUParameter param_signal_1[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<dropVolume>", QUParameter::In }
    };
    static const QUMethod signal_1 = {"plotMembers", 2, param_signal_1 };
    static const QUParameter param_signal_2[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_ptr, "vector<float>", QUParameter::In }
    };
    static const QUMethod signal_2 = {"plotCenter", 2, param_signal_2 };
    static const QUParameter param_signal_3[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod signal_3 = {"unPlotMembers", 1, param_signal_3 };
    static const QUParameter param_signal_4[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod signal_4 = {"unPlotCenter", 1, param_signal_4 };
    static const QMetaData signal_tbl[] = {
	{ "plotMembers(int,vector<vector<float>>)", &signal_0, QMetaData::Public },
	{ "plotMembers(int,vector<dropVolume>)", &signal_1, QMetaData::Public },
	{ "plotCenter(int,vector<float>)", &signal_2, QMetaData::Public },
	{ "unPlotMembers(int)", &signal_3, QMetaData::Public },
	{ "unPlotCenter(int)", &signal_4, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"ClusterWidget", parentObject,
	slot_tbl, 2,
	signal_tbl, 5,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_ClusterWidget.setMetaObject( metaObj );
    return metaObj;
}

void* ClusterWidget::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "ClusterWidget" ) )
	return this;
    return QWidget::qt_cast( clname );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL plotMembers
void ClusterWidget::plotMembers( int t0, vector<vector<float> > t1 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 0 );
    if ( !clist )
	return;
    QUObject o[3];
    static_QUType_int.set(o+1,t0);
    static_QUType_ptr.set(o+2,&t1);
    activate_signal( clist, o );
}

// SIGNAL plotMembers
void ClusterWidget::plotMembers( int t0, vector<dropVolume> t1 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 1 );
    if ( !clist )
	return;
    QUObject o[3];
    static_QUType_int.set(o+1,t0);
    static_QUType_ptr.set(o+2,&t1);
    activate_signal( clist, o );
}

// SIGNAL plotCenter
void ClusterWidget::plotCenter( int t0, vector<float> t1 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 2 );
    if ( !clist )
	return;
    QUObject o[3];
    static_QUType_int.set(o+1,t0);
    static_QUType_ptr.set(o+2,&t1);
    activate_signal( clist, o );
}

// SIGNAL unPlotMembers
void ClusterWidget::unPlotMembers( int t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 3, t0 );
}

// SIGNAL unPlotCenter
void ClusterWidget::unPlotCenter( int t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 4, t0 );
}

bool ClusterWidget::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: plot_members((bool)static_QUType_bool.get(_o+1)); break;
    case 1: plot_center((bool)static_QUType_bool.get(_o+1)); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool ClusterWidget::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: plotMembers((int)static_QUType_int.get(_o+1),(vector<vector<float> >)(*((vector<vector<float> >*)static_QUType_ptr.get(_o+2)))); break;
    case 1: plotMembers((int)static_QUType_int.get(_o+1),(vector<dropVolume>)(*((vector<dropVolume>*)static_QUType_ptr.get(_o+2)))); break;
    case 2: plotCenter((int)static_QUType_int.get(_o+1),(vector<float>)(*((vector<float>*)static_QUType_ptr.get(_o+2)))); break;
    case 3: unPlotMembers((int)static_QUType_int.get(_o+1)); break;
    case 4: unPlotCenter((int)static_QUType_int.get(_o+1)); break;
    default:
	return QWidget::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool ClusterWidget::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool ClusterWidget::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
