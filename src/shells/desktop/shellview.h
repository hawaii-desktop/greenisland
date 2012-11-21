
#ifndef SHELLVIEW_H
#define SHELLVIEW_H

#include "shellquickview.h"

class ShellView : public ShellQuickView
{
    Q_OBJECT
public:
    explicit ShellView(DesktopShell *shell);
};

#endif // SHELLVIEW_H
