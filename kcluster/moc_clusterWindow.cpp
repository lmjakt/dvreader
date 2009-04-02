/****************************************************************************
** ClusterWindow meta object code from reading C++ file 'clusterWindow.h'
**
** Created: Tue Mar 13 13:54:11 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.3   edited Aug 5 16:40 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "clusterWindow.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *ClusterWindow::className() const
{
    return "ClusterWindow";
}

QMetaObject *ClusterWindow::metaObj = 0;
static QMetaObjectCleanUp cleanUp_ClusterWindow( "ClusterWindow", &ClusterWindow::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString ClusterWindow::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "ClusterWindow", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString ClusterWindow::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "ClusterWindow", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* ClusterWindow::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "clusterPr", &static_QUType_ptr, "KClusterProcess", QUParameter::In }
    };
    static const QUMethod slot_0 = {"setClusters", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ "id", &static_QUType_int, 0, QUParameter::In },
	{ "members", &static_QUType_ptr, "vector<dropVolume>", QUParameter::In }
    };
    static const QUMethod slot_1 = {"plotMembers", 2, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ "id", &static_QUType_int, 0, QUParameter::In },
	{ "center", &static_QUType_ptr, "vector<float>", QUParameter::In }
    };
    static const QUMethod slot_2 = {"plotCenter", 2, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ "id", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"unPlotMembers", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ "id", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"unPlotCenter", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ "lno", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"plotSingleLine", 1, param_slot_5 };
    static const QMetaData slot_tbl[] = {
	{ "setClusters(KClusterProcess*)", &slot_0, QMetaData::Public },
	{ "plotMembers(int,vector<dropVolume>)", &slot_1, QMetaData::Private },
	{ "plotCenter(int,vector<float>)", &slot_2, QMetaData::Private },
	{ "unPlotMembers(int)", &slot_3, QMetaData::Private },
	{ "unPlotCenter(int)", &slot_4, QMetaData::Private },
	{ "plotSingleLine(int)", &slot_5, QMetaData::Private }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_ptr, "vector<Cluster>", QUParameter::InOut }
    };
    static const QUMethod signal_0 = {"drawClusters", 1, param_signal_0 };
    static const QMetaData signal_tbl[] = {
	{ "drawClusters(vector<Cluster>&)", &signal_0, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"ClusterWindow", parentObject,
	slot_tbl, 6,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_ClusterWindow.setMetaObject( metaObj );
    return metaObj;
}

void* ClusterWindow::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "ClusterWindow" ) )
	return this;
    return QWidget::qt_cast( clname );
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL drawClusters
void ClusterWindow::drawClusters( vector<Cluster>& t0 )
{
    if ( signalsBlocked() )
	return;
    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + 0 );
    if ( !clist )
	return;
    QUObject o[2];
    static_QUType_ptr.set(o+1,&t0);
    activate_signal( clist, o );
}

bool ClusterWindow::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setClusters((KClusterProcess*)static_QUType_ptr.get(_o+1)); break;
    case 1: plotMembers((int)static_QUType_int.get(_o+1),(vector<dropVolume>)(*((vector<dropVolume>*)static_QUType_ptr.get(_o+2)))); break;
    case 2: plotCenter((int)static_QUType_int.get(_o+1),(vector<float>)(*((vector<float>*)static_QUType_ptr.get(_o+2)))); break;
    case 3: unPlotMembers((int)static_QUType_int.get(_o+1)); break;
    case 4: unPlotCenter((int)static_QUType_int.get(_o+1)); break;
    case 5: plotSingleLine((int)static_QUType_int.get(_o+1)); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool ClusterWindow::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: drawClusters((vector<Cluster>&)*((vector<Cluster>*)static_QUType_ptr.get(_o+1))); break;
    default:
	return QWidget::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool ClusterWindow::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool ClusterWindow::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
