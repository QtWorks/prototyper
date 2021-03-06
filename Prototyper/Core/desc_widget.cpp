
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
#include "desc_widget.hpp"
#include "text_opts_bar.hpp"
#include "text_editor.hpp"
#include "form.hpp"

// Qt include.
#include <QTextDocument>
#include <QTextEdit>
#include <QVBoxLayout>


namespace Prototyper {

namespace Core {

//
// DescDockWidgetPrivate
//

class DescDockWidgetPrivate {
public:
	explicit DescDockWidgetPrivate( DescDockWidget * parent )
		:	m_editor( Q_NULLPTR )
		,	m_bar( Q_NULLPTR )
		,	m_form( Q_NULLPTR )
		,	q( parent )
	{
	}

	~DescDockWidgetPrivate()
	{
		m_editor->setDocument( Q_NULLPTR );
	}

	//! Init.
	void init();

	//! Document.
	QSharedPointer< QTextDocument > m_doc;
	//! Editor.
	TextEditor * m_editor;
	//! Tool bar.
	TextOptsBar * m_bar;
	//! Form.
	Form * m_form;
	//! Parent.
	DescDockWidget * q;
}; // class DescDockWidgetPrivate

void
DescDockWidgetPrivate::init()
{
	QWidget * w = new QWidget( q );

	QVBoxLayout * l = new QVBoxLayout( w );
	l->setMargin( 0 );

	m_bar = new TextOptsBar( TextOptsBar::Small, w );
	m_bar->setEnabled( false );

	l->addWidget( m_bar );

	m_editor = new TextEditor( w );
	m_editor->setEnabled( false );

	l->addWidget( m_editor );

	m_editor->setFontPointSize( 10.0 );

	q->setWidget( w );

	QTextCursor c = m_editor->textCursor();

	QTextCharFormat fmt = c.charFormat();
	fmt.setFontPointSize( 10.0 );

	c.setCharFormat( fmt );

	m_editor->setTextCursor( c );

	DescDockWidget::connect( m_bar, &TextOptsBar::lessFontSize,
		m_editor, &TextEditor::lessFontSize );
	DescDockWidget::connect( m_bar, &TextOptsBar::moreFontSize,
		m_editor, &TextEditor::moreFontSize );
	DescDockWidget::connect( m_bar, &TextOptsBar::bold,
		m_editor, &TextEditor::bold );
	DescDockWidget::connect( m_bar, &TextOptsBar::italic,
		m_editor, &TextEditor::italic );
	DescDockWidget::connect( m_bar, &TextOptsBar::underline,
		m_editor, &TextEditor::underline );
	DescDockWidget::connect( m_bar, &TextOptsBar::textColor,
		m_editor, &TextEditor::changeTextColor );
	DescDockWidget::connect( m_bar, &TextOptsBar::clearFormat,
		m_editor, &TextEditor::clearFormat );
	DescDockWidget::connect( m_bar, &TextOptsBar::alignLeft,
		m_editor, &TextEditor::alignLeft );
	DescDockWidget::connect( m_bar, &TextOptsBar::alignCenter,
		m_editor, &TextEditor::alignCenter );
	DescDockWidget::connect( m_bar, &TextOptsBar::alignRight,
		m_editor, &TextEditor::alignRight );
}


//
// DescDockWidget
//

DescDockWidget::DescDockWidget( QWidget * parent )
	:	QDockWidget( parent )
	,	d( new DescDockWidgetPrivate( this ) )
{
	d->init();
}

DescDockWidget::~DescDockWidget()
{
}

void
DescDockWidget::setDocument( QSharedPointer< QTextDocument > doc,
	const QString & id, Form * form )
{
	if( d->m_doc.data() )
		disconnect( d->m_doc.data(), 0, this, 0 );

	d->m_doc = doc;

	d->m_form = form;

	disconnect( d->m_editor, 0, this, 0 );

	if( d->m_doc.data() )
	{
		setWindowTitle( id );

		d->m_bar->setEnabled( true );
		d->m_editor->setEnabled( true );

		d->m_editor->setDocument( d->m_doc.data() );

		connect( d->m_editor, &TextEditor::changed,
			this, &DescDockWidget::changed );

		connect( d->m_doc.data(), &QTextDocument::cursorPositionChanged,
			d->m_bar, &TextOptsBar::updateState );

		d->m_bar->updateState( d->m_editor->textCursor() );
	}
	else
	{
		setWindowTitle( QString() );

		d->m_editor->setDocument( Q_NULLPTR );

		d->m_bar->setEnabled( false );
		d->m_editor->setEnabled( false );
	}
}

Form *
DescDockWidget::form() const
{
	return d->m_form;
}

} /* namespace Core */

} /* namespace Prototyper */
