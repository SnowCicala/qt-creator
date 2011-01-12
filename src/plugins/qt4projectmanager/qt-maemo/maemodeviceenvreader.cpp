/****************************************************************************
**
** Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Creator.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "maemodeviceenvreader.h"

#include "maemoglobal.h"
#include "maemorunconfiguration.h"

#include <coreplugin/ssh/sshconnection.h>
#include <coreplugin/ssh/sshremoteprocess.h>

namespace Qt4ProjectManager {
    namespace Internal {

MaemoDeviceEnvReader::MaemoDeviceEnvReader(QObject *parent, MaemoRunConfiguration *config)
    : QObject(parent)
    , m_stop(false)
    , m_devConfig(config->deviceConfig())
    , m_runConfig(config)
{
    connect(config, SIGNAL(deviceConfigurationChanged(ProjectExplorer::Target*)),
        this, SLOT(handleCurrentDeviceConfigChanged()));
}

MaemoDeviceEnvReader::~MaemoDeviceEnvReader()
{
}

void MaemoDeviceEnvReader::start()
{
    m_stop = false;
    if (m_connection)
        disconnect(m_connection.data(), 0, this, 0);

    const bool reuse = m_connection
        && m_connection->state() == Core::SshConnection::Connected
        && m_connection->connectionParameters() == m_devConfig.server;

    if (!reuse)
        m_connection = Core::SshConnection::create();

    connect(m_connection.data(), SIGNAL(connected()), this,
        SLOT(executeRemoteCall()));
    connect(m_connection.data(), SIGNAL(error(Core::SshError)), this,
        SLOT(handleConnectionFailure()));

    if (reuse)
        executeRemoteCall();
    else
        m_connection->connectToHost(m_devConfig.server);
}

void MaemoDeviceEnvReader::stop()
{
    m_stop = true;

    if (m_connection)
        disconnect(m_connection.data(), 0, this, 0);
    if (m_remoteProcess) {
        disconnect(m_remoteProcess.data());
        m_remoteProcess->closeChannel();
    }
}

void MaemoDeviceEnvReader::executeRemoteCall()
{
    if (m_stop)
        return;

    const QByteArray remoteCall = MaemoGlobal::remoteSourceProfilesCommand()
        .toUtf8() + "; env";
    m_remoteProcess = m_connection->createRemoteProcess(remoteCall);

    connect(m_remoteProcess.data(), SIGNAL(closed(int)), this,
        SLOT(remoteProcessFinished(int)));
    connect(m_remoteProcess.data(), SIGNAL(outputAvailable(QByteArray)), this,
        SLOT(remoteOutput(QByteArray)));
    connect(m_remoteProcess.data(), SIGNAL(errorOutputAvailable(QByteArray)), this,
        SLOT(remoteErrorOutput(QByteArray)));

    m_remoteOutput.clear();
    m_remoteErrorOutput.clear();
    m_remoteProcess->start();
}

void MaemoDeviceEnvReader::handleConnectionFailure()
{
    emit error(tr("Could not connect to host: %1")
        .arg(m_connection->errorString()));
    setFinished();
}

void MaemoDeviceEnvReader::handleCurrentDeviceConfigChanged()
{
    m_devConfig = m_runConfig->deviceConfig();

    m_env.clear();
    setFinished();
}

void MaemoDeviceEnvReader::remoteProcessFinished(int exitCode)
{
    Q_ASSERT(exitCode == Core::SshRemoteProcess::FailedToStart
        || exitCode == Core::SshRemoteProcess::KilledBySignal
        || exitCode == Core::SshRemoteProcess::ExitedNormally);

    if (m_stop)
        return;

    m_env.clear();
    if (exitCode == Core::SshRemoteProcess::ExitedNormally) {
        if (!m_remoteOutput.isEmpty()) {
            m_env = Utils::Environment(m_remoteOutput.split(QLatin1Char('\n'),
                QString::SkipEmptyParts));
        }
    } else {
        QString errorMsg = tr("Error running remote process: %1")
            .arg(m_remoteProcess->errorString());
        if (!m_remoteErrorOutput.isEmpty()) {
            errorMsg += tr("\nRemote stderr was: '%1'")
                .arg(QString::fromUtf8(m_remoteErrorOutput));
        }
        emit error(errorMsg);
    }
    setFinished();
}

void MaemoDeviceEnvReader::remoteOutput(const QByteArray &data)
{
    m_remoteOutput.append(QString::fromUtf8(data));
}

void MaemoDeviceEnvReader::remoteErrorOutput(const QByteArray &data)
{
    m_remoteErrorOutput += data;
}

void MaemoDeviceEnvReader::setFinished()
{
    stop();
    emit finished();
}

}   // Internal
}   // Qt4ProjectManager
