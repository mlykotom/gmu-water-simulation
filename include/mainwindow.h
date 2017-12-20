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

#include "CBaseParticleSimulator.h"
#include "CLWrapper.h"


#include <Qt3DRender/QRenderSettings>
#include <Qt3DRender/QViewport>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QRenderSurfaceSelector>


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
class FrameGraph;
class CQt3DWindow;

class MainWindow: public QMainWindow
{
Q_OBJECT

enum eComboBoxRole { platformRole = Qt::UserRole +1, deviceRole, simulationTypeRole};
enum eSimulationType { CPU = 0, GPUBrute, GPUGrid };

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow() override;

    CQt3DWindow *getView() { return m_mainView; }

signals:
    void keyPressed(Qt::Key key);

public slots:
    void onSimulationIterationChanged(unsigned long iteration);
    void keyPressEvent(QKeyEvent *event)
    {
        emit keyPressed((Qt::Key)event->key());
    }

private: //members
    //UI
    Ui::MainWindow *ui;

    CQt3DWindow *m_mainView;
    CScene *m_scene;
    FrameGraph *m_pFrameGraph;

    CBaseParticleSimulator *m_simulator;
   // CLWrapper *m_cl_wrapper;

private: //methods
    void setupUI();
    void setupDevicesComboBox();
    void setupSimulationTypesComboBox();

private slots:
    void onDevicesComboBoxIndexChanged(int index);
    void onSimulationTypeComboBoxIndexChanged(int index);


};

#endif // MAINWINDOW_H