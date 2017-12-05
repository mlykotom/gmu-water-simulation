#ifndef CPARTICLESIMULATOR_H
#define CPARTICLESIMULATOR_H

#include <QVector3D>
#include <QTimer>
#include <QPlaneMesh>
#include <QtMath>

#include <iostream>

#include "CScene.h"
#include "CParticle.h"
#include "CGrid.h"

#define GRAVITY_ACCELERATION (-9.80665f)
#define WALL_K 10000.0 // wall spring constant
#define WALL_DAMPING (-0.9) // wall damping constant


class CScene;

class CParticleSimulator: public QObject
{
    Q_OBJECT



public:
    CParticleSimulator(QObject *parent = 0);
    explicit CParticleSimulator(CScene *scene, unsigned long particlesCount = 20, QObject *parent = 0);

    ~CParticleSimulator();

    void setup();
    void start();

    void toggleSimulation();
    void toggleGravity();

    void updateGrid();
    void updateDensityPressure();
    void updateForces();
    void updateNewPositionVelocity();

    void step(double dt);

    double Wpoly6(double radiusSquared);
    QVector3D Wpoly6Gradient(QVector3D &diffPosition, double radiusSquared);
    QVector3D WspikyGradient(QVector3D &diffPosition, double radiusSquared);
    double WviscosityLaplacian(double radiusSquared);

public slots:
    void onKeyPressed(Qt::Key key);

private slots:
    void doWork();

private: //attributes

    QTimer m_timer;
    CScene *m_scene;
    QVector3D gravity;

    unsigned long m_particles_count;
    std::vector<CParticle *> *m_particles;
    QVector3D m_cellSize;

    double dt;

    std::vector<QPair<QVector3D, QVector3D>> _walls = std::vector<QPair<QVector3D, QVector3D>>();

    CGrid *m_grid;

    unsigned long iteration;
    QVector3D boxSize;
    double surfaceThreshold;



signals:
    void iterationChanged(unsigned long iteration);

};

#endif //WATERSURFACESIMULATION_PARTICLESIMULATOR_H
