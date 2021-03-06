
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
#include "text_editor.hpp"
#include "utils.hpp"

#include <QTextCursor>
#include <QTextDocumentFragment>
#include <QTextDocument>
#include <QTextBlockFormat>


namespace Prototyper {

namespace Core {

//
// TextEditorPrivate
//

class TextEditorPrivate {
public:
	explicit TextEditorPrivate( TextEditor * parent )
		:	q( parent )
	{
	}

	//! Init.
	void init();

	//! Parent.
	TextEditor * q;
}; // class TextEditorPrivate;

void
TextEditorPrivate::init()
{
	q->setWordWrapMode( QTextOption::WordWrap );
	q->setUndoRedoEnabled( true );

	q->setFontPointSize( 10.0 );

	QTextCursor c = q->textCursor();

	QTextCharFormat fmt = c.charFormat();
	fmt.setFontPointSize( 10.0 );

	c.setCharFormat( fmt );

	q->setTextCursor( c );

	TextEditor::connect( q, &QTextEdit::cursorPositionChanged,
		q, &TextEditor::slotCursorChanged );
	TextEditor::connect( q, &QTextEdit::textChanged,
		q, &TextEditor::changed );
}


//
// TextEditor
//

TextEditor::TextEditor( QWidget * parent )
	:	QTextEdit( parent )
	,	d( new TextEditorPrivate( this ) )
{
	d->init();
}

TextEditor::~TextEditor()
{
}



std::vector< Cfg::TextStyle >
TextEditor::text() const
{
	return Cfg::text( textCursor(), toPlainText() );
}

void
TextEditor::setText( const std::vector< Cfg::TextStyle > & blocks )
{
	reset();

	foreach( const Cfg::TextStyle & s, blocks )
	{
		if( std::find( s.style().cbegin(), s.style().cend(),
			Cfg::c_normalStyle ) != s.style().cend() )
		{
			setFontWeight( QFont::Normal );
			setFontItalic( false );
			setFontUnderline( false );
		}
		else
		{
			if( std::find( s.style().cbegin(), s.style().cend(),
				Cfg::c_boldStyle ) != s.style().cend() )
					setFontWeight( QFont::Bold );
			else
				setFontWeight( QFont::Normal );

			if( std::find( s.style().cbegin(), s.style().cend(),
				Cfg::c_italicStyle ) != s.style().cend() )
					setFontItalic( true );
			else
				setFontItalic( false );

			if( std::find( s.style().cbegin(), s.style().cend(),
				Cfg::c_underlineStyle ) != s.style().cend() )
					setFontUnderline( true );
			else
				setFontUnderline( false );
		}

		if( std::find( s.style().cbegin(), s.style().cend(),
			Cfg::c_left ) != s.style().cend() )
				setAlignment( Qt::AlignLeft );
		else if( std::find( s.style().cbegin(), s.style().cend(),
			Cfg::c_right ) != s.style().cend() )
				setAlignment( Qt::AlignRight );
		else if( std::find( s.style().cbegin(), s.style().cend(),
			Cfg::c_center ) != s.style().cend() )
				setAlignment( Qt::AlignCenter );

		setFontPointSize( s.fontSize() );

		insertPlainText( s.text() );

		QTextCursor cursor = textCursor();
		cursor.movePosition( QTextCursor::End );
		setTextCursor( cursor );
	}

	QTextCursor cursor = textCursor();
	cursor.setPosition( 0 );
	setTextCursor( cursor );

	document()->clearUndoRedoStacks();
}

void
TextEditor::lessFontSize()
{
	QTextCursor c = textCursor();

	if( c.hasSelection() )
	{
		if( c.position() != c.selectionEnd() )
			c.setPosition( c.selectionEnd() );

		QTextCharFormat fmt = c.charFormat();

		qreal s = fmt.fontPointSize();

		if( s < 1.0 )
			s = 10.0;

		s -= 1.0;

		if( s < 5.0 )
			s = 5.0;

		fmt.setFontPointSize( s );

		textCursor().setCharFormat( fmt );
	}
	else
	{
		qreal s = fontPointSize();
		s -= 1.0;

		if( s < 5.0 )
			s = 5.0;

		setFontPointSize( s );
	}

	emit changed();
}

void
TextEditor::moreFontSize()
{
	QTextCursor c = textCursor();

	if( c.hasSelection() )
	{
		if( c.position() != c.selectionEnd() )
			c.setPosition( c.selectionEnd() );

		QTextCharFormat fmt = c.charFormat();

		qreal s = fmt.fontPointSize();

		if( s < 1.0 )
			s = 10.0;

		s += 1.0;
		fmt.setFontPointSize( s );

		textCursor().setCharFormat( fmt );
	}
	else
	{
		qreal s = fontPointSize();
		s += 1.0;
		setFontPointSize( s );
	}

	emit changed();
}

void
TextEditor::bold( bool on )
{
	QTextCursor c = textCursor();

	if( c.hasSelection() )
	{
		if( c.position() != c.selectionEnd() )
			c.setPosition( c.selectionEnd() );

		QTextCharFormat fmt = c.charFormat();

		fmt.setFontWeight( on ? QFont::Bold : QFont::Normal );

		textCursor().setCharFormat( fmt );
	}
	else
		setFontWeight( on ? QFont::Bold : QFont::Normal );

	emit changed();
}

void
TextEditor::italic( bool on )
{
	QTextCursor c = textCursor();

	if( c.hasSelection() )
	{
		if( c.position() != c.selectionEnd() )
			c.setPosition( c.selectionEnd() );

		QTextCharFormat fmt = c.charFormat();

		fmt.setFontItalic( on );

		textCursor().setCharFormat( fmt );
	}
	else
		setFontItalic( on );

	emit changed();
}

void
TextEditor::underline( bool on )
{
	QTextCursor c = textCursor();

	if( c.hasSelection() )
	{
		if( c.position() != c.selectionEnd() )
			c.setPosition( c.selectionEnd() );

		QTextCharFormat fmt = c.charFormat();

		fmt.setFontUnderline( on );

		textCursor().setCharFormat( fmt );
	}
	else
		setFontUnderline( on );

	emit changed();
}

void
TextEditor::changeTextColor()
{
	emit changed();
}

void
TextEditor::slotCursorChanged()
{
	emit cursorChanged( textCursor() );
}

void
TextEditor::clearFormat()
{
	QTextCursor c = textCursor();

	if( c.hasSelection() )
	{
		QTextCharFormat fmt = c.charFormat();

		fmt.setFontUnderline( false );
		fmt.setFontItalic( false );
		fmt.setFontWeight( QFont::Normal );
		fmt.setFontPointSize( 10 );

		textCursor().setCharFormat( fmt );
	}
	else
	{
		setFontUnderline( false );
		setFontItalic( false );
		setFontWeight( QFont::Normal );
		setFontPointSize( 10 );
	}

	emit changed();
}

void
TextEditor::reset()
{
	clear();

	clearFormat();
}

void
TextEditor::alignLeft()
{
	QTextCursor c = textCursor();

	if( c.hasSelection() )
	{
		QTextBlockFormat b = c.blockFormat();
		b.setAlignment( Qt::AlignLeft );
		c.setBlockFormat( b );
		setTextCursor( c );
	}
	else
		setAlignment( Qt::AlignLeft );
}

void
TextEditor::alignCenter()
{
	QTextCursor c = textCursor();

	if( c.hasSelection() )
	{
		QTextBlockFormat b = c.blockFormat();
		b.setAlignment( Qt::AlignCenter );
		c.setBlockFormat( b );
		setTextCursor( c );
	}
	else
		setAlignment( Qt::AlignCenter );
}

void
TextEditor::alignRight()
{
	QTextCursor c = textCursor();

	if( c.hasSelection() )
	{
		QTextBlockFormat b = c.blockFormat();
		b.setAlignment( Qt::AlignRight );
		c.setBlockFormat( b );
		setTextCursor( c );
	}
	else
		setAlignment( Qt::AlignRight );
}

} /* namespace Core */

} /* namespace Prototyper */
