
/*!
	\file

	\author Igor Mironchik (igor.mironchik at gmail dot com).

	Copyright (c) 2016 Igor Mironchik

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Prototyper include.
#include "form_spinbox.hpp"
#include "utils.hpp"
#include "form.hpp"
#include "form_spinbox_properties.hpp"
#include "form_undo_commands.hpp"

// Qt include.
#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QUndoStack>
#include <QGraphicsScene>


namespace Prototyper {

namespace Core {

//
// FormSpinBoxPrivate
//

class FormSpinBoxPrivate {
public:
	FormSpinBoxPrivate( FormSpinBox * parent, const QRectF & rect )
		:	q( parent )
		,	m_rect( QRectF( rect.x(), rect.y(), rect.width(), 25.0 ) )
		,	m_proxy( 0 )
		,	m_text( FormSpinBox::tr( "1" ) )
	{
	}

	//! Init.
	void init();
	//! Set rect.
	void setRect( const QRectF & rect );

	//! Parent.
	FormSpinBox * q;
	//! Rect.
	QRectF m_rect;
	//! Resizable proxy.
	QScopedPointer< FormResizableProxy > m_proxy;
	//! Font.
	QFont m_font;
	//! Text.
	QString m_text;
}; // class FormSpinBoxPrivate

void
FormSpinBoxPrivate::init()
{
	m_proxy.reset( new FormResizableProxy( q, q->parentItem(), q->form() ) );

	setRect( m_rect );

	m_proxy->setMinSize( QSizeF( 45.0, 25.0 ) );

	m_font = QApplication::font();

	m_font.setPointSize( 10.0 );
}

void
FormSpinBoxPrivate::setRect( const QRectF & rect )
{
	m_rect = rect;

	q->setPos( m_rect.topLeft() );

	m_proxy->setRect( m_rect );

	m_rect.moveTopLeft( QPointF( 0.0, 0.0 ) );
}


//
// FormSpinBox
//

FormSpinBox::FormSpinBox( const QRectF & rect, Form * form,
	QGraphicsItem * parent )
	:	QGraphicsObject( parent )
	,	FormObject( FormObject::SpinBoxType, form )
	,	d( new FormSpinBoxPrivate( this, rect ) )
{
	d->init();
}

FormSpinBox::~FormSpinBox()
{
}

void
FormSpinBox::paint( QPainter * painter, const QStyleOptionGraphicsItem * option,
	QWidget * widget )
{
	Q_UNUSED( widget )
	Q_UNUSED( option )

	draw( painter, d->m_rect, objectPen(), d->m_font, d->m_text );

	if( isSelected() && !group() )
		d->m_proxy->show();
	else
		d->m_proxy->hide();
}

void
FormSpinBox::draw( QPainter * painter, const QRectF & rect,
	const QPen & pen, const QFont & font, const QString & text )
{
	painter->setPen( pen );

	painter->drawRoundedRect( rect, 2.0, 2.0 );

	const qreal h = rect.height();
	const qreal w = h * 0.75;
	const qreal leftX = rect.x() + rect.width() - w;

	painter->drawLine( QLineF( leftX, rect.y(), leftX, rect.y() + h ) );

	QPainterPath top;
	top.moveTo( leftX + 5.0, rect.y() + rect.height() / 2.0 - 2.5 );
	top.lineTo( leftX + w - 5.0, rect.y() + rect.height() / 2.0 - 2.5 );
	top.lineTo( leftX + w / 2.0, rect.y() + 5.0 );

	painter->setBrush( QBrush( pen.color() ) );

	painter->drawPath( top );

	QPainterPath bottom;
	bottom.moveTo( leftX + 5.0, rect.y() + rect.height() / 2.0 + 2.5 );
	bottom.lineTo( leftX + w - 5.0, rect.y() + rect.height() / 2.0 + 2.5 );
	bottom.lineTo( leftX + w / 2.0, rect.y() + h - 5.0 );

	painter->drawPath( bottom );

	QRectF textR = rect;
	textR.setRight( leftX - 5.0 );

	painter->setFont( font );

	painter->drawText( textR, Qt::AlignRight | Qt::AlignVCenter,
		text );
}

void
FormSpinBox::setObjectPen( const QPen & p, bool pushUndoCommand )
{
	FormObject::setObjectPen( p, pushUndoCommand );

	update();
}

Cfg::SpinBox
FormSpinBox::cfg() const
{
	Cfg::SpinBox c;

	c.set_objectId( objectId() );

	Cfg::Point p;
	p.set_x( pos().x() );
	p.set_y( pos().y() );

	c.set_pos( p );

	Cfg::Size s;
	s.set_width( d->m_rect.width() );
	s.set_height( d->m_rect.height() );

	c.set_size( s );

	c.set_pen( Cfg::pen( objectPen() ) );

	c.set_text( text() );

	c.set_link( link() );

	return c;
}

void
FormSpinBox::setCfg( const Cfg::SpinBox & c )
{
	setObjectId( c.objectId() );
	setLink( c.link() );
	setObjectPen( Cfg::fromPen( c.pen() ), false );
	d->setRect( QRectF( c.pos().x(), c.pos().y(),
		c.size().width(), c.size().height() ) );

	setText( c.text() );

	update();
}

Cfg::TextStyle
FormSpinBox::text() const
{
	Cfg::TextStyle textStyle = Cfg::textStyleFromFont( d->m_font );
	textStyle.style().push_back( Cfg::c_right );
	textStyle.set_text( d->m_text );

	return textStyle;
}

void
FormSpinBox::setText( const Cfg::TextStyle & c )
{
	if( std::find( c.style().cbegin(), c.style().cend(),
		Cfg::c_normalStyle ) != c.style().cend() )
	{
		d->m_font.setWeight( QFont::Normal );
		d->m_font.setItalic( false );
		d->m_font.setUnderline( false );
	}
	else
	{
		if( std::find( c.style().cbegin(), c.style().cend(),
			Cfg::c_boldStyle ) != c.style().cend() )
				d->m_font.setWeight( QFont::Bold );
		else
			d->m_font.setWeight( QFont::Normal );

		if( std::find( c.style().cbegin(), c.style().cend(),
			Cfg::c_italicStyle ) != c.style().cend() )
				d->m_font.setItalic( true );
		else
			d->m_font.setItalic( false );

		if( std::find( c.style().cbegin(), c.style().cend(),
			Cfg::c_underlineStyle ) != c.style().cend() )
				d->m_font.setUnderline( true );
		else
			d->m_font.setUnderline( false );
	}

	d->m_font.setPointSize( c.fontSize() );

	d->m_text = c.text();

	update();
}

QRectF
FormSpinBox::boundingRect() const
{
	return d->m_rect;
}

void
FormSpinBox::setPosition( const QPointF & pos, bool pushUndoCommand )
{
	if( pushUndoCommand )
		form()->undoStack()->push( new UndoMove< FormSpinBox > ( form(),
			objectId(), pos - position() ) );

	QRectF r = boundingRect();
	r.moveTopLeft( pos );

	d->setRect( r );
}

QPointF
FormSpinBox::position() const
{
	return pos();
}

QRectF
FormSpinBox::rectangle() const
{
	QRectF r = boundingRect();
	r.moveTopLeft( position() );

	return r;
}

void
FormSpinBox::setRectangle( const QRectF & rect,
	bool pushUndoCommand )
{
	Q_UNUSED( pushUndoCommand )

	resize( rect );

	scene()->update();
}

void
FormSpinBox::resize( const QRectF & rect )
{
	d->setRect( rect );

	form()->update();
}

void
FormSpinBox::moveResizable( const QPointF & delta )
{
	moveBy( delta.x(), delta.y() );
}

void
FormSpinBox::contextMenuEvent( QGraphicsSceneContextMenuEvent * event )
{
	QMenu menu;

	menu.addAction( QIcon( ":/Core/img/configure.png" ),
		tr( "Properties" ), this, &FormSpinBox::properties );

	menu.exec( event->screenPos() );
}

void
FormSpinBox::properties()
{
	SpinBoxProperties dlg;

	dlg.setCfg( cfg() );

	if( dlg.exec() == QDialog::Accepted )
	{
		form()->undoStack()->push( new UndoChangeTextWithOpts( form(),
			objectId(), text(), dlg.cfg().text() ) );

		setText( dlg.cfg().text() );

		update();
	}
}

} /* namespace Core */

} /* namespace Prototyper */
