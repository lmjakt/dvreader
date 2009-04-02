/****************************************************************************
** PlotWidget meta object code from reading C++ file 'plotWidget.h'
**
** Created: Tue Mar 13 13:53:38 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.3   edited Aug 5 16:40 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "plotWidget.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *PlotWidget::className() const
{
    return "PlotWidget";
}

QMetaObject *PlotWidget::metaObj = 0;
static QMetaObjectCleanUp cleanUp_PlotWidget( "PlotWidget", &PlotWidget::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString PlotWidget::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "PlotWidget", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString PlotWidget::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "PlotWidget", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* PlotWidget::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "v", &static_QUType_ptr, "std::vector<std::vector<float>>", QUParameter::In },
	{ "m", &static_QUType_ptr, "std::vector<int>", QUParameter::In },
	{ "LMargin", &static_QUType_int, 0, QUParameter::In },
	{ "RMargin", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"setValues", 4, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ "v", &static_QUType_ptr, "std::vector<std::vector<float>>", QUParameter::In },
	{ "m", &static_QUType_ptr, "std::vector<int>", QUParameter::In },
	{ "LMargin", &static_QUType_int, 0, QUParameter::In },
	{ "RMargin", &static_QUType_int, 0, QUParameter::In },
	{ "MinY", &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_1 = {"setValues", 5, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ "v", &static_QUType_ptr, "std::vector<std::vector<float>>", QUParameter::In },
	{ "m", &static_QUType_ptr, "std::vector<int>", QUParameter::In },
	{ "LMargin", &static_QUType_int, 0, QUParameter::In },
	{ "RMargin", &static_QUType_int, 0, QUParameter::In },
	{ "MinY", &static_QUType_ptr, "float", QUParameter::In },
	{ "MaxY", &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_2 = {"setValues", 6, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ "v", &static_QUType_ptr, "std::vector<std::vector<float>>", QUParameter::In }
    };
    static const QUMethod slot_3 = {"setValues", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ "Colors", &static_QUType_ptr, "std::vector<QColor>", QUParameter::In }
    };
    static const QUMethod slot_4 = {"setColors", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ "id", &static_QUType_ptr, "unsigned int", QUParameter::In },
	{ "mpv", &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_5 = {"setMinPeakValue", 2, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ "id", &static_QUType_ptr, "unsigned int", QUParameter::In },
	{ "mev", &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_6 = {"setMaxEdgeValue", 2, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ "id", &static_QUType_ptr, "unsigned int", QUParameter::In },
	{ "s", &static_QUType_ptr, "float", QUParameter::In }
    };
    static const QUMethod slot_7 = {"setScale", 2, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ "Peaks", &static_QUType_ptr, "std::map<int,std::vector<int>>", QUParameter::In }
    };
    static const QUMethod slot_8 = {"setPeaks", 1, param_slot_8 };
    static const QUMethod slot_9 = {"clearPeaks", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "setValues(std::vector<std::vector<float>>,std::vector<int>,int,int)", &slot_0, QMetaData::Public },
	{ "setValues(std::vector<std::vector<float>>,std::vector<int>,int,int,float)", &slot_1, QMetaData::Public },
	{ "setValues(std::vector<std::vector<float>>,std::vector<int>,int,int,float,float)", &slot_2, QMetaData::Public },
	{ "setValues(std::vector<std::vector<float>>)", &slot_3, QMetaData::Public },
	{ "setColors(std::vector<QColor>)", &slot_4, QMetaData::Public },
	{ "setMinPeakValue(unsigned int,float)", &slot_5, QMetaData::Public },
	{ "setMaxEdgeValue(unsigned int,float)", &slot_6, QMetaData::Public },
	{ "setScale(unsigned int,float)", &slot_7, QMetaData::Public },
	{ "setPeaks(std::map<int,std::vector<int>>)", &slot_8, QMetaData::Public },
	{ "clearPeaks()", &slot_9, QMetaData::Public }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod signal_0 = {"incrementImage", 1, param_signal_0 };
    static const QMetaData signal_tbl[] = {
	{ "incrementImage(int)", &signal_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"PlotWidget", parentObject,
	slot_tbl, 10,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_PlotWidget.setMetaObject( metaObj );
    return metaObj;
}

void* PlotWidget::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "PlotWidget" ) )
	return this;
    return QWidget::qt_cast( clname );
}

// SIGNAL incrementImage
void PlotWidget::incrementImage( int t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 0, t0 );
}

bool PlotWidget::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setValues((std::vector<std::vector<float> >)(*((std::vector<std::vector<float> >*)static_QUType_ptr.get(_o+1))),(std::vector<int>)(*((std::vector<int>*)static_QUType_ptr.get(_o+2))),(int)static_QUType_int.get(_o+3),(int)static_QUType_int.get(_o+4)); break;
    case 1: setValues((std::vector<std::vector<float> >)(*((std::vector<std::vector<float> >*)static_QUType_ptr.get(_o+1))),(std::vector<int>)(*((std::vector<int>*)static_QUType_ptr.get(_o+2))),(int)static_QUType_int.get(_o+3),(int)static_QUType_int.get(_o+4),(float)(*((float*)static_QUType_ptr.get(_o+5)))); break;
    case 2: setValues((std::vector<std::vector<float> >)(*((std::vector<std::vector<float> >*)static_QUType_ptr.get(_o+1))),(std::vector<int>)(*((std::vector<int>*)static_QUType_ptr.get(_o+2))),(int)static_QUType_int.get(_o+3),(int)static_QUType_int.get(_o+4),(float)(*((float*)static_QUType_ptr.get(_o+5))),(float)(*((float*)static_QUType_ptr.get(_o+6)))); break;
    case 3: setValues((std::vector<std::vector<float> >)(*((std::vector<std::vector<float> >*)static_QUType_ptr.get(_o+1)))); break;
    case 4: setColors((std::vector<QColor>)(*((std::vector<QColor>*)static_QUType_ptr.get(_o+1)))); break;
    case 5: setMinPeakValue((unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 6: setMaxEdgeValue((unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 7: setScale((unsigned int)(*((unsigned int*)static_QUType_ptr.get(_o+1))),(float)(*((float*)static_QUType_ptr.get(_o+2)))); break;
    case 8: setPeaks((std::map<int,std::vector<int> >)(*((std::map<int,std::vector<int> >*)static_QUType_ptr.get(_o+1)))); break;
    case 9: clearPeaks(); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool PlotWidget::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: incrementImage((int)static_QUType_int.get(_o+1)); break;
    default:
	return QWidget::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool PlotWidget::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool PlotWidget::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
