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

class MainWindow: public QMainWindow
{
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    Qt3DExtras::Qt3DWindow *getView() { return m_mainView; }

private:
    //UI
    Ui::MainWindow *ui;

    Qt3DExtras::Qt3DWindow *m_mainView;
    CScene *m_scene;
    FrameGraph *m_pFrameGraph;

    CParticleSimulator *m_simulator;

private slots:
    void onCameraChanged(const QVector3D &viewVector);

public slots:
    void onSimulationIterationChanged(unsigned long iteration);

protected:
    void keyPressEvent(QKeyEvent *event) override;
};

class FrameGraph : public Qt3DRender::QRenderSettings
{
    Q_OBJECT

public:
    FrameGraph(Qt3DCore::QNode* parent = 0);
    ~FrameGraph();
    void setCamera(Qt3DRender::QCamera* camera);
    inline Qt3DRender::QLayer* layer() { return m_pLayerFilter->layers().at(0); };
    inline Qt3DRender::QViewport* viewport() { return m_pViewport; };

private:
    Qt3DRender::QViewport* m_pViewport;
    Qt3DRender::QClearBuffers* m_pClearBuffer;
    Qt3DRender::QCameraSelector* m_pCameraSelector;
    Qt3DRender::QLayerFilter* m_pLayerFilter;
    Qt3DRender::QRenderSurfaceSelector* m_pRenderSurfaceSelector;
};

#endif // MAINWINDOW_H