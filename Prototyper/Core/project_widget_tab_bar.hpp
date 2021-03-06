
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

#ifndef PROTOTYPER__CORE__PROJECT_WIDGET_TAB_BAR_HPP__INCLUDED
#define PROTOTYPER__CORE__PROJECT_WIDGET_TAB_BAR_HPP__INCLUDED

// Qt include.
#include <QTabBar>
#include <QScopedPointer>


namespace Prototyper {

namespace Core {

//
// ProjectTabBar
//

class ProjectTabBarPrivate;

//! Form view.
class ProjectTabBar
	:	public QTabBar
{
	Q_OBJECT

signals:
	//! Form rename request.
	void formRenameRequest( const QString & name );
	//! Form delete request.
	void formDeleteRequest( const QString & name );
	//! Add form request.
	void formAddRequest();

public:
	explicit ProjectTabBar( QWidget * parent = 0 );
	~ProjectTabBar();

private slots:
	//! Rename tab.
	void renameTab();
	//! Add form.
	void addForm();
	//! Delete form.
	void deleteForm();

protected:
	void contextMenuEvent( QContextMenuEvent * event ) Q_DECL_OVERRIDE;

private:
	Q_DISABLE_COPY( ProjectTabBar )

	QScopedPointer< ProjectTabBarPrivate > d;
}; // class ProjectTabBar

} /* namespace Core */

} /* namespace Prototyper */

#endif // PROTOTYPER__CORE__PROJECT_WIDGET_TAB_BAR_HPP__INCLUDED
