/****************************************************************************
 * This file is part of Hawaii.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL$
 *
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1 or later as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPLv21 included in the packaging of
 * this file.  Please review the following information to ensure the
 * GNU Lesser General Public License version 2.1 requirements will be
 * met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * Alternatively, this file may be used under the terms of the GNU General
 * Public License version 2.0 or later as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 included in the
 * packaging of this file.  Please review the following information to ensure
 * the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QtGui/QOpenGLShaderProgram>

#include "eglfswaylandblitter.h"
#include "eglfswaylandcontext.h"
#include "eglfswaylandwindow.h"
#include "eglfswaylandlogging.h"

namespace GreenIsland {

namespace Platform {

EglFSWaylandBlitter::EglFSWaylandBlitter(EglFSWaylandContext *context)
    : QOpenGLFunctions()
    , m_context(context)
{
    initializeOpenGLFunctions();

    m_blitProgram = new QOpenGLShaderProgram();

    const QString vertex =
            QStringLiteral("attribute vec4 position;\n"
                           "attribute vec4 texCoords;\n"
                           "varying vec2 outTexCoords;\n"
                           "void main() {"
                           "gl_Position = position;\n"
                           "outTexCoords = texCoords.xy;\n"
                           "}");
    m_blitProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertex);

    const QString fragment =
            QStringLiteral("varying highp vec2 outTexCoords;\n"
                           "uniform sampler2D texture;\n"
                           "void main() {\n"
                           "gl_FragColor = texture2D(texture, outTexCoords);\n"
                           "}");
    m_blitProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragment);

    m_blitProgram->bindAttributeLocation("position", 0);
    m_blitProgram->bindAttributeLocation("texCoords", 1);

    if (!m_blitProgram->link()) {
        qCWarning(gLcEglFSWayland)
                << "Shader Program link failed:\n"
                << m_blitProgram->log();
    }
}

EglFSWaylandBlitter::~EglFSWaylandBlitter()
{
    delete m_blitProgram;
}

void EglFSWaylandBlitter::blit(EglFSWaylandWindow *window)
{
    QRect windowRect = window->window()->frameGeometry();
    int scale = window->scale();
    glViewport(0, 0, windowRect.width() * scale, windowRect.height() * scale);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    glDepthMask(GL_FALSE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    m_context->m_useNativeDefaultFbo = true;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_context->m_useNativeDefaultFbo = false;

    static const GLfloat squareVertices[] = {
        -1.f, -1.f,
        1.0f, -1.f,
        -1.f,  1.0f,
        1.0f,  1.0f
    };

    static const GLfloat textureVertices[] = {
        0.0f,  0.0f,
        1.0f,  0.0f,
        0.0f,  1.0f,
        1.0f,  1.0f,
    };

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    m_blitProgram->bind();

    m_blitProgram->enableAttributeArray(0);
    m_blitProgram->enableAttributeArray(1);
    m_blitProgram->setAttributeArray(1, textureVertices, 2);

    glActiveTexture(GL_TEXTURE0);

    // Draw
    m_blitProgram->setAttributeArray(0, squareVertices, 2);
    glBindTexture(GL_TEXTURE_2D, window->contentTexture());
    QRect r = window->contentsRect();
    glViewport(r.x(), r.y(), r.width() * scale, r.height() * scale);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Cleanup
    m_blitProgram->disableAttributeArray(0);
    m_blitProgram->disableAttributeArray(1);
}

} // namespace Platform

} // namespace GreenIsland
