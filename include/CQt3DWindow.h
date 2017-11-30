#ifndef CQT3DWINDOW_H
#define CQT3DWINDOW_H

#include <Qt3DExtras/qt3dwindow.h>


class CQt3DWindow : public Qt3DExtras::Qt3DWindow
{
    Q_OBJECT

public:
    CQt3DWindow(QScreen *screen = 0);
    ~CQt3DWindow();

signals:
    void keyPressed(Qt::Key key);

protected:
    void keyPressEvent(QKeyEvent *event) override;
};

#endif