
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
#include "form_combobox.hpp"
#include "utils.hpp"
#include "form.hpp"
#include "form_undo_commands.hpp"

// Qt include.
#include <QPainter>
#include <QPainterPath>
#include <QUndoStack>
#include <QGraphicsScene>


namespace Prototyper {

namespace Core {

//
// FormComboBoxPrivate
//

class FormComboBoxPrivate {
public:
	FormComboBoxPrivate( FormComboBox * parent, const QRectF & rect )
		:	q( parent )
		,	m_rect( QRectF( rect.x(), rect.y(), rect.width(), 25.0 ) )
		,	m_proxy( 0 )
	{
	}

	//! Init.
	void init();
	//! Set rect.
	void setRect( const QRectF & rect );

	//! Parent.
	FormComboBox * q;
	//! Rect.
	QRectF m_rect;
	//! Resizable proxy.
	QScopedPointer< FormResizableProxy > m_proxy;
}; // class FormComboBoxPrivate

void
FormComboBoxPrivate::init()
{
	m_proxy.reset( new FormResizableProxy( q, q->parentItem(), q->form() ) );

	setRect( m_rect );

	m_proxy->setMinSize( QSizeF( 45.0, 25.0 ) );
}

void
FormComboBoxPrivate::setRect( const QRectF & rect )
{
	m_rect = rect;

	q->setPos( m_rect.topLeft() );

	m_proxy->setRect( m_rect );

	m_rect.moveTopLeft( QPointF( 0.0, 0.0 ) );
}


//
// FormComboBox
//

FormComboBox::FormComboBox( const QRectF & rect, Form * form,
	QGraphicsItem * parent )
	:	QGraphicsItem( parent )
	,	FormObject( FormObject::ComboBoxType, form )
	,	d( new FormComboBoxPrivate( this, rect ) )
{
	d->init();
}

FormComboBox::~FormComboBox()
{
}

void
FormComboBox::paint( QPainter * painter, const QStyleOptionGraphicsItem * option,
	QWidget * widget )
{
	Q_UNUSED( widget )
	Q_UNUSED( option )

	draw( painter, d->m_rect, objectPen() );

	if( isSelected() && !group() )
		d->m_proxy->show();
	else
		d->m_proxy->hide();
}

void
FormComboBox::draw( QPainter * painter, const QRectF & rect,
	const QPen & pen )
{
	painter->setPen( pen );

	painter->drawRoundedRect( rect, 2.0, 2.0 );

	const qreal h = rect.height();
	const qreal leftX = rect.x() + rect.width() - h;

	painter->drawLine( QLineF( leftX, rect.y(), leftX, rect.y() + h ) );

	QPainterPath path;
	path.moveTo( leftX + 5.0, rect.y() + 5.0 );
	path.lineTo( leftX + h - 5.0, rect.y() + 5.0 );
	path.lineTo( leftX + h / 2.0, rect.y() + h - 5.0 );

	painter->setBrush( QBrush( pen.color() ) );

	painter->drawPath( path );
}

void
FormComboBox::setObjectPen( const QPen & p, bool pushUndoCommand )
{
	FormObject::setObjectPen( p, pushUndoCommand );

	update();
}

Cfg::ComboBox
FormComboBox::cfg() const
{
	Cfg::ComboBox c;

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

	c.set_link( link() );

	return c;
}

void
FormComboBox::setCfg( const Cfg::ComboBox & c )
{
	setObjectId( c.objectId() );
	setLink( c.link() );
	setObjectPen( Cfg::fromPen( c.pen() ), false );
	d->setRect( QRectF( c.pos().x(), c.pos().y(),
		c.size().width(), c.size().height() ) );

	update();
}

QRectF
FormComboBox::boundingRect() const
{
	return d->m_rect;
}

void
FormComboBox::setPosition( const QPointF & pos, bool pushUndoCommand )
{
	if( pushUndoCommand )
		form()->undoStack()->push( new UndoMove< FormComboBox > ( form(),
			objectId(), pos - position() ) );

	QRectF r = boundingRect();
	r.moveTopLeft( pos );

	d->setRect( r );
}

QPointF
FormComboBox::position() const
{
	return pos();
}

QRectF
FormComboBox::rectangle() const
{
	QRectF r = boundingRect();
	r.moveTopLeft( position() );

	return r;
}

void
FormComboBox::setRectangle( const QRectF & rect, bool pushUndoCommand )
{
	Q_UNUSED( pushUndoCommand )

	resize( rect );

	scene()->update();
}

void
FormComboBox::resize( const QRectF & rect )
{
	d->setRect( rect );

	form()->update();
}

void
FormComboBox::moveResizable( const QPointF & delta )
{
	moveBy( delta.x(), delta.y() );
}

} /* namespace Core */

} /* namespace Prototyper */
