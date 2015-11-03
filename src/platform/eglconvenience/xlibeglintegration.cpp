/****************************************************************************
 * This file is part of Green Island.
 *
 * Copyright (C) 2015 Pier Luigi Fiorini
 * Copyright (C) 2015 The Qt Company Ltd.
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPL213$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QtCore/QLoggingCategory>

#include "xlibeglintegration.h"
#include "logging.h"

namespace GreenIsland {

namespace Platform {

VisualID XlibEglIntegration::getCompatibleVisualId(Display *display, EGLDisplay eglDisplay, EGLConfig config)
{
    VisualID    visualId = 0;
    EGLint      eglValue = 0;

    EGLint configRedSize = 0;
    eglGetConfigAttrib(eglDisplay, config, EGL_RED_SIZE, &configRedSize);

    EGLint configGreenSize = 0;
    eglGetConfigAttrib(eglDisplay, config, EGL_GREEN_SIZE, &configGreenSize);

    EGLint configBlueSize = 0;
    eglGetConfigAttrib(eglDisplay, config, EGL_BLUE_SIZE, &configBlueSize);

    EGLint configAlphaSize = 0;
    eglGetConfigAttrib(eglDisplay, config, EGL_ALPHA_SIZE, &configAlphaSize);

    eglGetConfigAttrib(eglDisplay, config, EGL_CONFIG_ID, &eglValue);
    int configId = eglValue;

    // See if EGL provided a valid VisualID:
    eglGetConfigAttrib(eglDisplay, config, EGL_NATIVE_VISUAL_ID, &eglValue);
    visualId = (VisualID)eglValue;
    if (visualId) {
        // EGL has suggested a visual id, so get the rest of the visual info for that id:
        XVisualInfo visualInfoTemplate;
        memset(&visualInfoTemplate, 0, sizeof(XVisualInfo));
        visualInfoTemplate.visualid = visualId;

        XVisualInfo *chosenVisualInfo;
        int matchingCount = 0;
        chosenVisualInfo = XGetVisualInfo(display, VisualIDMask, &visualInfoTemplate, &matchingCount);
        if (chosenVisualInfo) {
            // Skip size checks if implementation supports non-matching visual
            // and config (QTBUG-9444).
            if (EglUtils::hasEglExtension(eglDisplay,"EGL_NV_post_convert_rounding")) {
                XFree(chosenVisualInfo);
                return visualId;
            }
            // Skip also for i.MX6 where 565 visuals are suggested for the default 444 configs and it works just fine.
            const char *vendor = eglQueryString(eglDisplay, EGL_VENDOR);
            if (vendor && strstr(vendor, "Vivante")) {
                XFree(chosenVisualInfo);
                return visualId;
            }

            int visualRedSize = qPopulationCount(chosenVisualInfo->red_mask);
            int visualGreenSize = qPopulationCount(chosenVisualInfo->green_mask);
            int visualBlueSize = qPopulationCount(chosenVisualInfo->blue_mask);
            int visualAlphaSize = chosenVisualInfo->depth == 32 ? 8 : 0;

            const bool visualMatchesConfig = visualRedSize == configRedSize
                    && visualGreenSize == configGreenSize
                    && visualBlueSize == configBlueSize
                    && visualAlphaSize == configAlphaSize;

            // In some cases EGL tends to suggest a 24-bit visual for 8888
            // configs. In such a case we have to fall back to XGetVisualInfo.
            if (!visualMatchesConfig) {
                visualId = 0;
                qCDebug(lcEglConvenience,
                        "EGL suggested using X Visual ID %d (%d %d %d depth %d) for EGL config %d"
                        "(%d %d %d %d), but this is incompatible",
                        (int)visualId, visualRedSize, visualGreenSize, visualBlueSize, chosenVisualInfo->depth,
                        configId, configRedSize, configGreenSize, configBlueSize, configAlphaSize);
            }
        } else {
            qCDebug(lcEglConvenience, "EGL suggested using X Visual ID %d for EGL config %d, but that isn't a valid ID",
                    (int)visualId, configId);
            visualId = 0;
        }
        XFree(chosenVisualInfo);
    }
    else
        qCDebug(lcEglConvenience, "EGL did not suggest a VisualID (EGL_NATIVE_VISUAL_ID was zero) for EGLConfig %d", configId);

    if (visualId) {
        qCDebug(lcEglConvenience, configAlphaSize > 0
                ? "Using ARGB Visual ID %d provided by EGL for config %d"
                : "Using Opaque Visual ID %d provided by EGL for config %d", (int)visualId, configId);
        return visualId;
    }

    // Finally, try to use XGetVisualInfo and only use the bit depths to match on:
    if (!visualId) {
        XVisualInfo visualInfoTemplate;
        memset(&visualInfoTemplate, 0, sizeof(XVisualInfo));
        XVisualInfo *matchingVisuals;
        int matchingCount = 0;

        visualInfoTemplate.depth = configRedSize + configGreenSize + configBlueSize + configAlphaSize;
        matchingVisuals = XGetVisualInfo(display,
                                         VisualDepthMask,
                                         &visualInfoTemplate,
                                         &matchingCount);
        if (!matchingVisuals) {
            // Try again without taking the alpha channel into account:
            visualInfoTemplate.depth = configRedSize + configGreenSize + configBlueSize;
            matchingVisuals = XGetVisualInfo(display,
                                             VisualDepthMask,
                                             &visualInfoTemplate,
                                             &matchingCount);
        }

        if (matchingVisuals) {
            visualId = matchingVisuals[0].visualid;
            XFree(matchingVisuals);
        }
    }

    if (visualId) {
        qCDebug(lcEglConvenience, "Using Visual ID %d provided by XGetVisualInfo for EGL config %d", (int)visualId, configId);
        return visualId;
    }

    qCWarning(lcEglConvenience, "Unable to find an X11 visual which matches EGL config %d", configId);
    return (VisualID)0;
}

} // namespace Platform

} // namespace GreenIsland
