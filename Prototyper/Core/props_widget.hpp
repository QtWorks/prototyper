
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

#ifndef PROTOTYPER__CORE__PROPS_WIDGET_HPP__INCLUDED
#define PROTOTYPER__CORE__PROPS_WIDGET_HPP__INCLUDED

// Qt include.
#include <QWidget>
#include <QScopedPointer>


namespace Prototyper {

namespace Core {

//
// PropsWidget
//

class PropsWidgetPrivate;

//! Main window with project.
class PropsWidget
	:	public QWidget
{
	Q_OBJECT

public:
	PropsWidget( QWidget * parent = 0, Qt::WindowFlags f = 0 );
	~PropsWidget();

private:
	Q_DISABLE_COPY( PropsWidget )

	QScopedPointer< PropsWidgetPrivate > d;
}; // class PropsWidget

} /* namespace Core */

} /* namespace Prototyper */

#endif // PROTOTYPER__CORE__PROPS_WIDGET_HPP__INCLUDED
