/*
  remoteviewserver.cpp

  This file is part of GammaRay, the Qt application inspection and
  manipulation tool.

  Copyright (C) 2015 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: Volker Krause <volker.krause@kdab.com>

  Licensees holding valid commercial KDAB GammaRay licenses may use this file in
  accordance with GammaRay Commercial License Agreement provided with the Software.

  Contact info@kdab.com if any conditions of this licensing are not clear to you.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "remoteviewserver.h"

#include <QCoreApplication>
#include <QDebug>
#include <QMouseEvent>

using namespace GammaRay;

RemoteViewServer::RemoteViewServer(const QString& name, QObject* parent):
    RemoteViewInterface(name, parent),
    m_eventReceiver(Q_NULLPTR)
{
}

void RemoteViewServer::setEventReceiver(QObject* receiver)
{
    m_eventReceiver = receiver;
}

void RemoteViewServer::pickElementAt(const QPoint& pos)
{
    emit doPickElement(pos);
}

void RemoteViewServer::resetView()
{
    emit reset();
}

void RemoteViewServer::sendKeyEvent(int type, int key, int modifiers, const QString& text, bool autorep, ushort count)
{
    if (!m_eventReceiver)
        return;

    auto event = new QKeyEvent((QEvent::Type)type, key, (Qt::KeyboardModifiers)modifiers, text, autorep, count);
    QCoreApplication::postEvent(m_eventReceiver, event);
}

void RemoteViewServer::sendMouseEvent(int type, const QPoint& localPos, int button, int buttons, int modifiers)
{
    if (!m_eventReceiver)
        return;

    auto event = new QMouseEvent((QEvent::Type)type, localPos, (Qt::MouseButton)button, (Qt::MouseButtons)buttons, (Qt::KeyboardModifiers)modifiers);
    QCoreApplication::postEvent(m_eventReceiver, event);
}

void RemoteViewServer::sendWheelEvent(const QPoint& localPos, QPoint pixelDelta, QPoint angleDelta, int buttons, int modifiers)
{
    if (!m_eventReceiver)
        return;

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    auto event = new QWheelEvent(localPos, localPos /*FIXME*/, pixelDelta, angleDelta, 0, /*not used*/ Qt::Vertical, /*not used*/ (Qt::MouseButtons)buttons, (Qt::KeyboardModifiers)modifiers);
#else
    Q_UNUSED(pixelDelta);
    auto orientation = angleDelta.x() == 0 ? Qt::Vertical : Qt::Horizontal;
    auto delta = orientation == Qt::Horizontal ? angleDelta.x() :angleDelta.y();
    auto event = new QWheelEvent(localPos, delta, (Qt::MouseButtons)buttons, (Qt::KeyboardModifiers)modifiers, orientation);
#endif
    QCoreApplication::sendEvent(m_eventReceiver, event);
}