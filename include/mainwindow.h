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



#include <Qt3DRender/QRenderSettings>
#include <Qt3DRender/QViewport>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QRenderSurfaceSelector>


namespace Ui {
    class MainWindow;
}

//forward declarations
namespace Qt3DExtras
{
class Qt3DWindow;
}
class CScene;
class FrameGraph;
class CQt3DWindow;


class MainWindow: public QMainWindow
{
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    CQt3DWindow *getView() { return m_mainView; }

private:
    //UI
    Ui::MainWindow *ui;

    CQt3DWindow *m_mainView;
    CScene *m_scene;
    FrameGraph *m_pFrameGraph;

    CParticleSimulator *m_simulator;

public slots:
    void onSimulationIterationChanged(unsigned long iteration);

};

#endif // MAINWINDOW_H