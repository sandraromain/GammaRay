/*
  helpcontroller.cpp

  This file is part of GammaRay, the Qt application inspection and
  manipulation tool.

  Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
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

#include <config-gammaray.h>

#include "helpcontroller.h"

#include <common/paths.h>

#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QProcess>
#include <QString>
#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
#include <QStandardPaths>
#endif

using namespace GammaRay;

namespace GammaRay {
struct HelpControllerPrivate
{
    HelpControllerPrivate()
        : proc(Q_NULLPTR) {}

    void startProcess();
    void sendCommand(const QByteArray &cmd);

    QString assistantPath;
    QString qhcPath;
    QProcess *proc;
};
}

void HelpControllerPrivate::startProcess()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    if (proc)
        return;

    proc = new QProcess(QCoreApplication::instance());
    QObject::connect(proc,
                     static_cast<void (QProcess::*)(int,
                                                    QProcess::ExitStatus)>(&QProcess::finished),
                     [this](){
        proc->deleteLater();
        proc = Q_NULLPTR;
    });
    proc->setProgram(assistantPath);
    proc->setArguments(QStringList()
                       <<QLatin1String("-collectionFile")
                       <<qhcPath
                       <<QLatin1String("-enableRemoteControl")
                       );
    proc->start();
    proc->waitForStarted();
    sendCommand("expandToc 2;");
#endif
}

void HelpControllerPrivate::sendCommand(const QByteArray &cmd)
{
    if (!proc)
        return;
    proc->write(cmd);
}

Q_GLOBAL_STATIC(HelpControllerPrivate, s_helpController)

bool HelpController::isAvailable()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
    auto d = s_helpController();
    if (!d->assistantPath.isEmpty() && !d->qhcPath.isEmpty())
        return true;

    d->assistantPath = QStandardPaths::findExecutable(QStringLiteral("assistant"));
    if (d->assistantPath.isEmpty()) {
        qDebug() << "Qt Assistant not found, help not available.";
        return false;
    }

    const QString qhcPath = Paths::documentationPath() + QLatin1String("/gammaray.qhc");
    if (QFileInfo::exists(qhcPath)) {
        d->qhcPath = qhcPath;
        return true;
    } else {
        qDebug() << "gammaray.qhc not found in" << Paths::documentationPath()
                 << " - help not available";
    }
#endif
    return false;
}

void HelpController::openContents()
{
    Q_ASSERT(isAvailable());
    auto d = s_helpController();
    d->startProcess();
    d->sendCommand(
        "setSource qthelp://com.kdab.GammaRay." GAMMARAY_PLUGIN_VERSION "/doc/index.html;syncContents\n");
}

void HelpController::openPage(const QString &page)
{
    Q_ASSERT(isAvailable());
    auto d = s_helpController();
    d->startProcess();
    d->sendCommand(QByteArray(
                       "setSource qthelp://com.kdab.GammaRay." GAMMARAY_PLUGIN_VERSION "/") + page.toUtf8()
                   + ";syncContents\n");
}