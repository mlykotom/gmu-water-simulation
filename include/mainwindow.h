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
#include <QStandardItemModel>
#include <Qt3DWindow>
#include <QMessageBox>

#include <Qt3DRender/QRenderSettings>
#include <Qt3DRender/QViewport>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QRenderSurfaceSelector>

// local
#include "CLWrapper.h"
#include "CLPlatforms.h"
#include "CQt3DWindow.h"
#include "CBaseParticleSimulator.h"
#include "CGPUParticleSimulator.h"
#include "CGPUBruteParticleSimulator.h"
#include "CCPUParticleSimulator.h"
#include "config.h"

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

    enum eComboBoxRole
    {
        platformRole = Qt::UserRole + 1, deviceRole, simulationTypeRole
    };
    enum eSimulationType
    {
        GPUGrid = 0, GPUBrute, CPU
    };

    struct sSimulationOptions
    {
        eSimulationType type;
        float boxSize;
        int platformIndex;
        int deviceIndex;
    };

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow() override;

    CQt3DWindow *getView() { return m_mainView; }

signals:
    void keyPressed(Qt::Key key);

public slots:
    void onSimulationIterationChanged(unsigned long iteration);
    void onError(const char *error);
    void keyPressEvent(QKeyEvent *event)
    {
        emit keyPressed((Qt::Key) event->key());
    }

private: //members
    //UI
    Ui::MainWindow *ui;

    CQt3DWindow *m_mainView;
    CScene *m_scene;
    FrameGraph *m_pFrameGraph;

    CBaseParticleSimulator *m_simulator;
    sSimulationOptions m_simulationOptions;
    bool m_simulationIsReady;
private: //methods
    void setupUI();
    void setupDevicesComboBox();
    void setupSimulationTypesComboBox();
    void setup3DWidget();
    void setupScene();
    void resetScene();

    void createSimulator();
    void togglePushButtons(bool value);
private slots:
    void onDevicesComboBoxIndexChanged(int index);
    void onSimulationTypeComboBoxIndexChanged(int index);
    void onCubeSizeSliderValueChanged(int value);
    void onStartSimulationClicked();
    void onPauseSimulationClicked();
    void onStopSimulationClicked();
    void onSetupSimulationClicked();

    void exportLogs();
    void onKeyPressed(Qt::Key key);
    void resetCamera(Qt3DRender::QCamera *pCamera);
};

#endif // MAINWINDOW_H