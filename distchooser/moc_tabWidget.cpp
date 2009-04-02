/****************************************************************************
** TabWidget meta object code from reading C++ file 'tabWidget.h'
**
** Created: Tue Mar 13 13:53:32 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.3   edited Aug 5 16:40 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "tabWidget.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *TabWidget::className() const
{
    return "TabWidget";
}

QMetaObject *TabWidget::metaObj = 0;
static QMetaObjectCleanUp cleanUp_TabWidget( "TabWidget", &TabWidget::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString TabWidget::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "TabWidget", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString TabWidget::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "TabWidget", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* TabWidget::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUMethod signal_0 = {"copyRanges", 0, 0 };
    static const QUMethod signal_1 = {"pasteRanges", 0, 0 };
    static const QUMethod signal_2 = {"saveRanges", 0, 0 };
    static const QUMethod signal_3 = {"readRangesFromFile", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "copyRanges()", &signal_0, QMetaData::Public },
	{ "pasteRanges()", &signal_1, QMetaData::Public },
	{ "saveRanges()", &signal_2, QMetaData::Public },
	{ "readRangesFromFile()", &signal_3, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"TabWidget", parentObject,
	0, 0,
	signal_tbl, 4,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_TabWidget.setMetaObject( metaObj );
    return metaObj;
}

void* TabWidget::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "TabWidget" ) )
	return this;
    return QWidget::qt_cast( clname );
}

// SIGNAL copyRanges
void TabWidget::copyRanges()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

// SIGNAL pasteRanges
void TabWidget::pasteRanges()
{
    activate_signal( staticMetaObject()->signalOffset() + 1 );
}

// SIGNAL saveRanges
void TabWidget::saveRanges()
{
    activate_signal( staticMetaObject()->signalOffset() + 2 );
}

// SIGNAL readRangesFromFile
void TabWidget::readRangesFromFile()
{
    activate_signal( staticMetaObject()->signalOffset() + 3 );
}

bool TabWidget::qt_invoke( int _id, QUObject* _o )
{
    return QWidget::qt_invoke(_id,_o);
}

bool TabWidget::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: copyRanges(); break;
    case 1: pasteRanges(); break;
    case 2: saveRanges(); break;
    case 3: readRangesFromFile(); break;
    default:
	return QWidget::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool TabWidget::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool TabWidget::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
