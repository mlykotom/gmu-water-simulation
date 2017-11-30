#include "CQt3DWindow.h"

#include<QKeyEvent>

CQt3DWindow::CQt3DWindow(QScreen *screen)
    :Qt3DWindow(screen)
{

}

CQt3DWindow::~CQt3DWindow()
{

}

void CQt3DWindow::keyPressEvent(QKeyEvent * event)
{
    emit keyPressed((Qt::Key)event->key());
}