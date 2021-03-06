
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
#include "form_button_properties.hpp"
#include "ui_form_button_properties.h"
#include "utils.hpp"

// Qt include.
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QDialogButtonBox>


namespace Prototyper {

namespace Core {

//
// ButtonPropertiesPrivate
//

class ButtonPropertiesPrivate {
public:
	explicit ButtonPropertiesPrivate( ButtonProperties * parent )
		:	q( parent )
	{
	}

	//! Init.
	void init();

	//! Parent.
	ButtonProperties * q;
	//! Ui.
	Ui::ButtonProperties m_ui;
}; // class ButtonPropertiesPrivate

void
ButtonPropertiesPrivate::init()
{
	m_ui.setupUi( q );
}


//
// ButtonProperties
//

ButtonProperties::ButtonProperties( QWidget * parent, Qt::WindowFlags f )
	:	QDialog( parent, f )
	,	d( new ButtonPropertiesPrivate( this ) )
{
	d->init();
}

ButtonProperties::~ButtonProperties()
{
}

Cfg::TextStyle
ButtonProperties::cfg() const
{
	Cfg::TextStyle c;

	std::vector< QString > style;

	if( d->m_ui.m_bold->isChecked() )
		style.push_back( Cfg::c_boldStyle );

	if( d->m_ui.m_italic->isChecked() )
		style.push_back( Cfg::c_italicStyle );

	if( d->m_ui.m_underline->isChecked() )
		style.push_back( Cfg::c_underlineStyle );

	if( style.empty() )
		style.push_back( Cfg::c_normalStyle );

	c.set_style( style );
	c.set_fontSize( d->m_ui.m_fontSize->value() );
	c.set_text( d->m_ui.m_text->text() );

	return c;
}

void
ButtonProperties::setCfg( const Cfg::TextStyle & c )
{
	if( std::find( c.style().cbegin(), c.style().cend(),
		Cfg::c_boldStyle ) != c.style().cend() )
			d->m_ui.m_bold->setChecked( true );
	else
		d->m_ui.m_bold->setChecked( false );

	if( std::find( c.style().cbegin(), c.style().cend(),
		Cfg::c_italicStyle ) != c.style().cend() )
			d->m_ui.m_italic->setChecked( true );
	else
		d->m_ui.m_italic->setChecked( false );

	if( std::find( c.style().cbegin(), c.style().cend(),
		Cfg::c_underlineStyle ) != c.style().cend() )
			d->m_ui.m_underline->setChecked( true );
	else
		d->m_ui.m_underline->setChecked( false );

	d->m_ui.m_fontSize->setValue( c.fontSize() );

	d->m_ui.m_text->setText( c.text() );
}

} /* namespace Core */

} /* namespace Prototyper */
