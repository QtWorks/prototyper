
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
#include "form.hpp"
#include "top_gui.hpp"
#include "project_window.hpp"
#include "project_cfg.hpp"
#include "form_actions.hpp"
#include "form_line.hpp"
#include "grid_snap.hpp"
#include "form_polyline.hpp"
#include "form_group.hpp"
#include "form_rect_placer.hpp"
#include "form_text.hpp"
#include "form_image.hpp"

// Qt include.
#include <QPainter>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QApplication>
#include <QTextCursor>
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QImage>
#include <QVariant>


namespace Prototyper {

namespace Core {

//
// FormPrivate
//

class FormPrivate {
public:
	FormPrivate( Cfg::Form & cfg, Form * parent )
		:	q( parent )
		,	m_gridMode( Form::ShowGrid )
		,	m_gridStepAction( 0 )
		,	m_cfg( cfg )
		,	m_pressed( false )
		,	m_current( 0 )
		,	m_id( 0 )
		,	m_snap( 0 )
		,	m_polyline( false )
		,	m_currentPoly( 0 )
	{
	}

	//! Init.
	void init();
	//! \return Current Z-value.
	qreal currentZValue() const;
	//! \return Current Z-value.
	void currentZValue( const QList< QGraphicsItem* > & items,
		qreal & z ) const;
	//! \return Start point for line.
	QPointF lineStartPoint( const QPointF & point,
		bool & intersected, bool & intersectedEnds,
		FormLine* & intersectedLine ) const;
	//! Clear current lines.
	void clearCurrentLines();
	//! Handle mouse move in current lines.
	void handleMouseMoveInCurrentLines( const QPointF & point );
	//! Handle mouse move in current polyline.
	void handleMouseMoveInCurrentPolyLine( const QPointF & point );
	//! Ungroup.
	void ungroup( QGraphicsItem * group );

	//! Parent.
	Form * q;
	//! Grid mode.
	Form::GridMode m_gridMode;
	//! Grid step action.
	QAction * m_gridStepAction;
	//! Cfg.
	Cfg::Form & m_cfg;
	//! Pressed.
	bool m_pressed;
	//! Current item.
	QGraphicsItem * m_current;
	//! Mouse pos.
	QPointF m_pos;
	//! ID.
	quint64 m_id;
	//! Current lines.
	QList< FormLine* > m_currentLines;
	//! Grid snap.
	GridSnap * m_snap;
	//! Make polyline.
	bool m_polyline;
	//! Current polyline.
	FormPolyline * m_currentPoly;
}; // class FormPrivate

void
FormPrivate::init()
{
	m_gridStepAction = new QAction( QIcon( ":/Core/img/measure.png" ),
		ProjectWindow::tr( "Grid Step" ), q );

	Form::connect( m_gridStepAction, &QAction::triggered,
		q, &Form::slotSetGridStep );

	m_snap = new GridSnap( q );
	m_snap->setGridStep( m_cfg.gridStep() );

	q->setAcceptHoverEvents( true );
}

qreal
FormPrivate::currentZValue() const
{
	qreal z = 0.0;

	currentZValue( q->childItems(), z );

	return z;
}

void
FormPrivate::currentZValue( const QList< QGraphicsItem* > & items,
	qreal & z ) const
{
	foreach( QGraphicsItem * item, items )
	{
		const QList< QGraphicsItem* > children = item->childItems();

		if( !children.isEmpty() )
			currentZValue( children, z );

		if( item->zValue() > z )
			z = item->zValue();
	}
}

QPointF
FormPrivate::lineStartPoint( const QPointF & point, bool & intersected,
	bool & intersectedEnds, FormLine* & intersectedLine ) const
{
	if( m_currentPoly )
	{
		const QPointF tmp =
			m_currentPoly->pointUnderHandle( point, intersected );

		intersectedEnds = intersected;

		return tmp;
	}
	else
	{
		foreach( FormLine * line, m_currentLines )
		{
			const QPointF tmp = line->pointUnderHandle( point, intersected,
				intersectedEnds );

			if( intersected )
			{
				return tmp;

				intersectedLine = line;
			}
		}
	}

	return point;
}

void
FormPrivate::clearCurrentLines()
{
	foreach( FormLine * line, m_currentLines )
		line->showHandles( false );

	m_currentLines.clear();
}

void
FormPrivate::handleMouseMoveInCurrentLines( const QPointF & point )
{
	foreach( FormLine * line, m_currentLines )
		line->handleMouseMoveInHandles( point );
}

void
FormPrivate::handleMouseMoveInCurrentPolyLine( const QPointF & point )
{
	m_currentPoly->handleMouseMoveInHandles( point );
}

void
FormPrivate::ungroup( QGraphicsItem * group )
{
	FormGroup * tmp = dynamic_cast< FormGroup* > ( group );

	if( tmp )
	{
		QList< QGraphicsItem* > items = tmp->childItems();

		foreach( QGraphicsItem * item, items )
		{
			tmp->removeFromGroup( item );

			if( FormAction::instance()->mode() == FormAction::Select )
			{
				item->setFlag( QGraphicsItem::ItemIsSelectable, true );

				item->setSelected( true );
			}
		}

		q->scene()->removeItem( tmp );

		delete tmp;
	}
}


//
// Form
//

Form::Form( Cfg::Form & c, QGraphicsItem * parent )
	:	QGraphicsObject( parent )
	,	d( 0 )
{
	QScopedPointer< FormPrivate > tmp( new FormPrivate( c, this ) );

	tmp->init();

	d.swap( tmp );
}

Form::~Form()
{
}

const Cfg::Size &
Form::size() const
{
	return d->m_cfg.size();
}

void
Form::setSize( const Cfg::Size & s )
{
	d->m_cfg.setSize( s );

	update();
}

Form::GridMode
Form::gridMode() const
{
	return d->m_gridMode;
}

void
Form::setGridMode( GridMode m )
{
	d->m_gridMode = m;

	update();
}

int
Form::gridStep() const
{
	return d->m_cfg.gridStep();
}

void
Form::setGridStep( int s )
{
	d->m_cfg.setGridStep( s );

	d->m_snap->setGridStep( s );

	update();
}

const Cfg::Form &
Form::cfg() const
{
	return d->m_cfg;
}

void
Form::switchToSelectMode()
{
	foreach( FormLine * line, d->m_currentLines )
		line->setSelected( true );

	if( d->m_current )
		d->m_current->setSelected( true );

	if( d->m_currentPoly )
	{
		d->m_currentPoly->setSelected( true );
		d->m_currentPoly->showHandles( false );
	}

	d->clearCurrentLines();
}

void
Form::switchToLineDrawingMode()
{
	d->m_currentLines.clear();
	d->m_current = 0;
	d->m_currentPoly = 0;
	d->m_polyline = false;
}

void
Form::enableSnap( bool on )
{
	if( on )
		d->m_snap->show();
	else
		d->m_snap->hide();
}

GridSnap *
Form::snapItem() const
{
	return d->m_snap;
}

void
Form::group()
{
	QList< QGraphicsItem* > items =
		scene()->selectedItems();

	FormGroup * group = 0;

	if( items.size() > 1 )
	{
		group = new FormGroup( this );

		foreach( QGraphicsItem * item, items )
		{
			item->setFlag( QGraphicsItem::ItemIsSelectable, false );

			item->setSelected( false );

			if( item->parentItem() == this )
				group->addToGroup( item );
		}
	}
	else if( d->m_currentLines.size() > 1 )
	{
		group = new FormGroup( this );

		foreach( FormLine * line, d->m_currentLines )
		{
			line->setFlag( QGraphicsItem::ItemIsSelectable, false );

			line->setSelected( false );

			group->addToGroup( line );
		}

		d->m_currentLines.clear();
	}

	if( group )
	{
		d->m_current = group;

		if( FormAction::instance()->mode() == FormAction::Select )
		{
			group->setFlag( QGraphicsItem::ItemIsSelectable );

			// Workaround for stay group selected.
			// Without this processEvents() after selecting group
			// its selected and next unknown to me event clears selection.
			QApplication::processEvents();

			group->setSelected( true );
		}

		group->setObjectId( ++d->m_id );
	}
}

void
Form::ungroup()
{
	QList< QGraphicsItem* > items =
		scene()->selectedItems();

	if( !items.isEmpty() )
	{
		foreach( QGraphicsItem * item, items )
			if( item->parentItem() == this )
				d->ungroup( item );
	}
	else if( d->m_current )
		d->ungroup( d->m_current );
}

QRectF
Form::boundingRect() const
{
	if( !d.isNull() )
		return QRectF( 0, 0, d->m_cfg.size().width(),
			d->m_cfg.size().height() );
	else
		return QRectF();
}

void
Form::paint( QPainter * painter, const QStyleOptionGraphicsItem * option,
	QWidget * widget )
{
	Q_UNUSED( option )
	Q_UNUSED( widget )

	const int w = d->m_cfg.size().width();
	const int h = d->m_cfg.size().height();

	painter->save();

	painter->setRenderHint( QPainter::Antialiasing, false );

	QRect r( 0, 0, w, h );

	static const QColor gridColor = Qt::gray;

	painter->setPen( gridColor );
	painter->setBrush( Qt::white );

	painter->drawRect( r );

	if( d->m_gridMode == ShowGrid )
	{
		for( int x = d->m_cfg.gridStep(); x < w; x += d->m_cfg.gridStep() )
			painter->drawLine( x, 0, x, h );

		for( int y = d->m_cfg.gridStep(); y < h; y += d->m_cfg.gridStep() )
			painter->drawLine( 0, y, w, y );
	}

	painter->restore();
}

void
Form::slotSetGridStep()
{
	TopGui::instance()->projectWindow()->setGridStep(
		d->m_cfg.gridStep(), false );
}

void
Form::contextMenuEvent( QGraphicsSceneContextMenuEvent * event )
{
	QMenu menu;
	menu.addAction( TopGui::instance()->projectWindow()->showHideGridAction() );
	menu.addAction( d->m_gridStepAction );

	menu.exec( event->screenPos() );

	event->accept();
}

void
Form::mouseMoveEvent( QGraphicsSceneMouseEvent * mouseEvent )
{
	if( d->m_pressed )
	{
		const QPointF delta = mouseEvent->pos() - d->m_pos;

		d->m_pos = mouseEvent->pos();

		d->m_snap->setSnapPos( d->m_pos );

		switch( FormAction::instance()->mode() )
		{
			case FormAction::DrawLine :
			{
				FormLine * line = dynamic_cast< FormLine* > ( d->m_current );

				if( line )
				{
					const QLineF l = line->line();

					line->setLine( l.p1().x(), l.p1().y(),
						l.p2().x() + delta.x(), l.p2().y() + delta.y() );

					d->handleMouseMoveInCurrentLines( mouseEvent->pos() );
				}

				if( d->m_currentPoly )
					d->handleMouseMoveInCurrentPolyLine( mouseEvent->pos() );

				mouseEvent->accept();

				return;
			}
				break;

			case FormAction::InsertText :
			{
				FormRectPlacer * rect = dynamic_cast< FormRectPlacer* >
					( d->m_current );

				if( rect )
					rect->setEndPos( mouseEvent->pos() );

				mouseEvent->accept();

				update();

				return;
			}
				break;

			default :
				break;
		}
	}

	mouseEvent->ignore();
}

void
Form::mousePressEvent( QGraphicsSceneMouseEvent * mouseEvent )
{
	if( mouseEvent->button() == Qt::LeftButton )
	{
		d->m_pos = mouseEvent->pos();

		d->m_snap->setSnapPos( d->m_pos );

		switch( FormAction::instance()->mode() )
		{
			case FormAction::DrawLine :
			{
				d->m_pressed = true;

				FormLine * line = new FormLine( this );

				bool intersected = false;
				bool intersectedEnds = false;
				FormLine * intersectedLine = 0;

				QPointF p = d->lineStartPoint( mouseEvent->pos(),
					intersected, intersectedEnds, intersectedLine );

				if( !intersected && FormAction::instance()->isSnapEnabled() )
					p = d->m_snap->snapPos();

				line->setLine( p.x(), p.y(), p.x(), p.y() );

				if( intersectedEnds && d->m_currentLines.size() == 1 &&
					FormAction::instance()->testFlag( FormAction::Polyline ) )
						d->m_polyline = true;

				line->setObjectId( ++d->m_id );

				d->m_current = line;

				if( !intersected )
				{
					d->clearCurrentLines();

					d->m_polyline = false;

					if( d->m_currentPoly )
					{
						d->m_currentPoly->showHandles( false );

						d->m_currentPoly = 0;
					}
				}

				mouseEvent->accept();

				return;
			}
				break;

			case FormAction::InsertText :
			{
				d->m_pressed = true;

				FormRectPlacer * rect = new FormRectPlacer( this );

				QPointF p = mouseEvent->pos();

				if( FormAction::instance()->isSnapEnabled() )
					p = d->m_snap->snapPos();

				rect->setStartPos( p );

				d->m_current = rect;

				mouseEvent->accept();

				return;
			}
				break;

			default :
				break;
		}
	}

	mouseEvent->ignore();
}

void
Form::mouseReleaseEvent( QGraphicsSceneMouseEvent * mouseEvent )
{
	if( mouseEvent->button() == Qt::LeftButton && d->m_pressed )
	{
		d->m_pressed = false;

		d->m_snap->setSnapPos( mouseEvent->pos() );

		switch( FormAction::instance()->mode() )
		{
			case FormAction::DrawLine :
			{
				FormLine * line = dynamic_cast< FormLine* > ( d->m_current );

				if( line )
				{
					bool intersected = false;
					bool intersectedEnds = false;
					FormLine * intersectedLine = 0;

					QPointF p = d->lineStartPoint( mouseEvent->pos(),
						intersected, intersectedEnds, intersectedLine );

					if( !intersected && FormAction::instance()->isSnapEnabled() )
						p = d->m_snap->snapPos();

					const QLineF l = line->line();

					line->setLine( l.p1().x(), l.p1().y(), p.x(), p.y() );

					if( d->m_polyline )
					{
						if( !d->m_currentLines.isEmpty() )
						{
							d->m_currentPoly = new FormPolyline( this );
							d->m_currentPoly->setObjectId( ++d->m_id );
							d->m_currentPoly->appendLine( d->m_currentLines.first()->line() );
							d->m_currentPoly->showHandles( true );

							scene()->removeItem( d->m_currentLines.first() );

							delete d->m_currentLines.first();

							d->m_currentLines.clear();
						}

						d->m_currentPoly->appendLine( line->line() );

						scene()->removeItem( line );

						delete line;

						d->m_current = 0;

						if( d->m_currentPoly->isClosed() )
						{
							d->m_currentPoly->showHandles( false );

							d->m_current = d->m_currentPoly;

							d->m_currentPoly = 0;

							d->m_polyline = false;
						}
					}
					else
					{
						d->m_currentLines.append( line );

						line->showHandles( true );
					}
				}

				d->m_pressed = false;

				mouseEvent->accept();

				emit changed();

				return;
			}
				break;

			case FormAction::InsertText :
			{
				scene()->removeItem( d->m_current );

				FormRectPlacer * rect = dynamic_cast< FormRectPlacer* >
					( d->m_current );

				FormText * text = 0;

				if( rect )
				{
					text = new FormText( rect->rect(), this );

					text->setFocus();

					QTextCursor c = text->textCursor();
					c.select( QTextCursor::Document );
					text->setTextCursor( c );
				}

				delete d->m_current;

				d->m_current = text;

				mouseEvent->accept();

				d->m_pressed = false;

				emit changed();

				return;
			}
				break;

			default :
				break;
		}

		d->m_current = 0;
	}

	mouseEvent->ignore();
}

void
Form::hoverEnterEvent( QGraphicsSceneHoverEvent * event )
{
	d->m_snap->setSnapPos( event->pos() );

	event->ignore();
}

void
Form::hoverMoveEvent( QGraphicsSceneHoverEvent * event )
{
	d->m_snap->setSnapPos( event->pos() );

	event->ignore();
}

void
Form::dragEnterEvent( QGraphicsSceneDragDropEvent * event )
{
	if( event->mimeData()->hasImage() )
		event->acceptProposedAction();
	else
		QGraphicsObject::dragEnterEvent( event );
}

void
Form::dropEvent( QGraphicsSceneDragDropEvent * event )
{
	if( event->mimeData()->hasImage() )
	{
		FormImage * image = new FormImage( this );
		image->setPos( event->pos() );
		image->setImage( qvariant_cast< QImage >
			( event->mimeData()->imageData() ) );

		event->acceptProposedAction();
	}
	else
		QGraphicsObject::dropEvent( event );
}

} /* namespace Core */

} /* namespace Prototyper */
