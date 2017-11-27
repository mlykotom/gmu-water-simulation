#ifndef MAINWINDOW_H
#define MAINWINDOW_H


// Qt 3D
#include <QCamera>
#include <QInputAspect>
#include <QAspectEngine>
#include <QRenderAspect>
#include <QForwardRenderer>
#include <Qt3DWindow>
#include <QOrbitCameraController>
#include <QMainWindow>
#include <QKeyEvent>
#include <QApplication>

#include "CParticleSimulator.h"

namespace Ui
{
class MainWindow;
}

//forward declarations
namespace Qt3DExtras
{
class Qt3DWindow;
}
class CScene;

class MainWindow: public QMainWindow
{
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    Qt3DExtras::Qt3DWindow *getView() { return m_view; }

private:
    //UI
    Ui::MainWindow *ui;

    Qt3DExtras::Qt3DWindow *m_view;
    CScene *m_scene;

    CParticleSimulator *m_simulator;

private slots:
    void onCameraChanged(const QVector3D &viewVector);

protected:
    void keyPressEvent(QKeyEvent *event) override;
};


#endif // MAINWINDOW_H