/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (c) 2012 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:GPL3+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QGuiApplication>
#include <QDir>
#include <QDebug>
#include <QPluginLoader>
#include <QProcess>
#include <QStringList>
#include <QScreen>

#include <VShellPlugin>

#include "cmakedirs.h"
#include "compositor.h"

class GreenIsland : public QGuiApplication
{
    Q_OBJECT
public:
    GreenIsland(int &argc, char **argv)
        : QGuiApplication(argc, argv)
        , m_shellProcess(0) {
        // Set application information
        setApplicationName("Green Island");
        setApplicationVersion("0.1.0");
        setOrganizationName("Maui Project");
        setOrganizationDomain("maui-project.org");
    }

#if 0
    VShell *loadShell(const QString &name) {
        // Load plugins
        QDir pluginsDir(QStringLiteral("%1/greenisland/shells").arg(INSTALL_PLUGINSDIR));
        foreach(QString fileName, pluginsDir.entryList(QDir::Files)) {
            QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
            VShellPlugin *plugin = qobject_cast<VShellPlugin *>(
                                       loader.instance());
            if (!plugin)
                continue;

            foreach(QString key, plugin->keys()) {
                if (key == name)
                    return plugin->create(key);
            }
        }

        return 0;
    }
#else
    void runShell() {
        // Force Wayland as a QPA plugin and GTK+ backend and reuse XDG_RUNTIME_DIR
        QProcessEnvironment env;
        env.insert(QLatin1String("QT_QPA_PLATFORM"), QLatin1String("wayland"));
        env.insert(QLatin1String("GDK_BACKEND"), QLatin1String("wayland"));
        env.insert(QLatin1String("XDG_RUNTIME_DIR"), qgetenv("XDG_RUNTIME_DIR"));

        // Run the shell client process
        QStringList arguments;
        arguments << "-platform" << "wayland";
        m_shellProcess = new QProcess(this);
        connect(m_shellProcess, SIGNAL(started()), this, SLOT(shellStarted()));
        connect(m_shellProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(shellFailed(QProcess::ProcessError)));
        m_shellProcess->setProcessEnvironment(env);
        m_shellProcess->start(QLatin1String(INSTALL_BINDIR "/greenisland-desktop-shell"), arguments, QIODevice::ReadOnly);
    }

private:
    QProcess *m_shellProcess;

private slots:
    void shellStarted() {
        if (m_shellProcess)
            qDebug() << "Shell is ready!";
    }

    void shellFailed(QProcess::ProcessError error) {
        switch (error) {
        case QProcess::FailedToStart:
            qWarning("The shell process failed to start.\n"
                     "Either the invoked program is missing, or you may have insufficient permissions to run it.\n");
            break;
        case QProcess::Crashed:
            qWarning("The shell process crashed some time after starting successfully.\n");
            break;
        case QProcess::Timedout:
            qWarning("The shell process timedout.\n");
            break;
        case QProcess::WriteError:
            qWarning("An error occurred when attempting to write to the shell process.\n");
            break;
        case QProcess::ReadError:
            qWarning("An error occurred when attempting to read from the shell process.\n");
            break;
        case QProcess::UnknownError:
            qWarning("Unknown error starting the shell process!\n");
            break;
        }

        // Print shell output
        if (m_shellProcess) {
            qDebug() << "Standard output:" << m_shellProcess->readAllStandardOutput();
            qDebug() << "Standard error:" << m_shellProcess->readAllStandardError();
        }

        // Don't need it anymore because it failed
        delete m_shellProcess;
        m_shellProcess = 0;
    }
#endif
};

int main(int argc, char *argv[])
{
    // Assume the xcb platform if the DISPLAY environment variable is defined,
    // otherwise go for kms
    if (!qgetenv("DISPLAY").isEmpty())
        setenv("QT_QPA_PLATFORM", "xcb", 0);
    else
        setenv("QT_QPA_PLATFORM", "kms", 0);

    GreenIsland app(argc, argv);

    // Shell plugin (defaults to desktop for the moment)
    QString pluginName = QLatin1String("desktop");

    // Command line arguments
    QStringList arguments = QCoreApplication::instance()->arguments();

    // Usage instructions
    if (arguments.contains(QStringLiteral("-h")) || arguments.contains(QStringLiteral("--help"))) {
        qDebug() << "Usage: greenisland [options]";
        qDebug() << "Arguments are:";
        qDebug() << "\t--fullscreen\t\trun in fullscreen mode";
        qDebug() << "\t--synthesize-touch\tsynthesize touch for unhandled mouse events";
        qDebug() << "\t--plugin NAME\t\tuse the NAME shell plugin (default 'desktop')";
        return 0;
    }

    // Synthesize touch for unhandled mouse events
    if (arguments.contains(QStringLiteral("--synthesize-touch")))
        app.setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents, true);

    // Shell plugin argument
    int pluginArg = arguments.indexOf(QLatin1String("--plugin"));
    if (pluginArg != -1 && pluginArg + 1 < arguments.size())
        pluginName = arguments.at(pluginArg + 1).toLocal8Bit();

    // Start the compositor and set it up
    Compositor compositor;
    compositor.setWindowTitle("Green Island");
    if (arguments.contains(QStringLiteral("--fullscreen"))) {
        compositor.setGeometry(QGuiApplication::primaryScreen()->geometry());
        compositor.showFullScreen();
    } else {
        compositor.setGeometry(QGuiApplication::primaryScreen()->availableGeometry());
        compositor.showMaximized();
    }

#if 0
    // Load the shell plugin
    VShell *shell = app.loadShell(pluginName);
    if (!shell)
        qFatal("Unable to run the shell because the '%s' plugin was not found",
               pluginName.toLocal8Bit().constData());
#else
    app.runShell();
#endif

    return app.exec();
}

#include "main.moc"
