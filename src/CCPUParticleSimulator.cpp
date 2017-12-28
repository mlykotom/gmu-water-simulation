#include "CCPUParticleSimulator.h"

CCPUParticleSimulator::CCPUParticleSimulator(CScene *scene, float boxSize, SimulationScenario scenario, QObject *parent)
    : CBaseParticleSimulator(scene, boxSize, scenario, parent)
{

}

double CCPUParticleSimulator::Wpoly6(double radiusSquared)
{
    static double coefficient = 315.0 / (64.0 * M_PI * pow(CParticle::h, 9));
    static double hSquared = CParticle::h * CParticle::h;

    return coefficient * pow(hSquared - radiusSquared, 3);
}

QVector3D CCPUParticleSimulator::WspikyGradient(QVector3D &diffPosition, double radiusSquared)
{
    static double coefficient = -45.0 / (M_PI * pow(CParticle::h, 6));
    double radius = sqrt(radiusSquared);
    return coefficient * pow(CParticle::h - radius, 2) * diffPosition / radius;
}

double CCPUParticleSimulator::WviscosityLaplacian(double radiusSquared)
{
    static double coefficient = 45.0 / (M_PI * pow(CParticle::h, 6));
    double radius = sqrt(radiusSquared);

    return coefficient * (CParticle::h - radius);
}

double CCPUParticleSimulator::updateGrid()
{
    QElapsedTimer timer;
    timer.start();

    for (int x = 0; x < m_grid->xRes(); x++) {
        for (int y = 0; y < m_grid->yRes(); y++) {
            for (int z = 0; z < m_grid->zRes(); z++) {

                std::vector<CParticle *> &particles = m_grid->at(x, y, z);

                for (unsigned long p = 0; p < particles.size(); p++) {
                    CParticle *particle = particles[p];

                    int newGridCellX = (int) floor((particle->position().x() + m_boxSize.x() / 2.0) / CParticle::h);
                    int newGridCellY = (int) floor((particle->position().y() + m_boxSize.y() / 2.0) / CParticle::h);
                    int newGridCellZ = (int) floor((particle->position().z() + m_boxSize.z() / 2.0) / CParticle::h);

                    if (newGridCellX < 0) {
                        newGridCellX = 0;
                    }
                    else if (newGridCellX >= m_grid->xRes()) {
                        newGridCellX = m_grid->xRes() - 1;
                    }

                    if (newGridCellY < 0) {
                        newGridCellY = 0;
                    }
                    else if (newGridCellY >= m_grid->yRes()) {
                        newGridCellY = m_grid->yRes() - 1;
                    }

                    if (newGridCellZ < 0) {
                        newGridCellZ = 0;
                    }
                    else if (newGridCellZ >= m_grid->zRes()) {
                        newGridCellZ = m_grid->zRes() - 1;
                    }


                    // check if particle has moved
                    if (x != newGridCellX || y != newGridCellY || z != newGridCellZ) {
                        // move the particle to the new grid cell

                        std::vector<CParticle *> &what = m_grid->at(newGridCellX, newGridCellY, newGridCellZ);
                        what.push_back(particle);
                        // remove it from it's previous grid cell

                        particles.at(p) = particles.back();
                        particles.pop_back();

                        p--; // important! make sure to redo this index, since a new particle will (probably) be there
                    }
                }
            }
        }
    }

    return timer.elapsed();
}

double CCPUParticleSimulator::updateDensityPressure()
{
    QElapsedTimer timer;
    timer.start();

    for (int x = 0; x < m_grid->xRes(); x++) {
        for (int y = 0; y < m_grid->yRes(); y++) {
            for (int z = 0; z < m_grid->zRes(); z++) {

                auto &particles = m_grid->at(x, y, z);
                for (auto &particle : particles) {

                    particle->density() = 0.0;

                    // neighbors
                    for (int offsetX = -1; offsetX <= 1; offsetX++) {
                        if (x + offsetX < 0) continue;
                        if (x + offsetX >= m_grid->xRes()) break;

                        for (int offsetY = -1; offsetY <= 1; offsetY++) {
                            if (y + offsetY < 0) continue;
                            if (y + offsetY >= m_grid->yRes()) break;

                            for (int offsetZ = -1; offsetZ <= 1; offsetZ++) {
                                if (z + offsetZ < 0) continue;
                                if (z + offsetZ >= m_grid->zRes()) break;

                                auto &neighborGridCellParticles = m_grid->at(x + offsetX, y + offsetY, z + offsetZ);
                                for (auto &neighbor : neighborGridCellParticles) {
                                    QVector3D distance = (particle->position() - neighbor->position());
                                    double radiusSquared = distance.lengthSquared();

                                    if (radiusSquared <= CParticle::h * CParticle::h) {
                                        particle->density() += Wpoly6(radiusSquared);
                                    }
                                }
                            }
                        }
                    }

                    particle->density() *= CParticle::mass;
                    particle->pressure() = CParticle::gas_stiffness * (particle->density() - CParticle::rest_density);
                }
            }
        }
    }

    return timer.elapsed();
}

double CCPUParticleSimulator::updateForces()
{
    QElapsedTimer timer;
    timer.start();

    for (int x = 0; x < m_grid->xRes(); x++) {
        for (int y = 0; y < m_grid->yRes(); y++) {
            for (int z = 0; z < m_grid->zRes(); z++) {

                auto &particles = m_grid->at(x, y, z);

                for (auto &particle : particles) {
                    QVector3D f_gravity = gravity * particle->density();
                    QVector3D f_pressure, f_viscosity;

                    // neighbors
                    for (int offsetX = -1; offsetX <= 1; offsetX++) {
                        if (x + offsetX < 0) continue;
                        if (x + offsetX >= m_grid->xRes()) break;

                        for (int offsetY = -1; offsetY <= 1; offsetY++) {
                            if (y + offsetY < 0) continue;
                            if (y + offsetY >= m_grid->yRes()) break;

                            for (int offsetZ = -1; offsetZ <= 1; offsetZ++) {
                                if (z + offsetZ < 0) continue;
                                if (z + offsetZ >= m_grid->zRes()) break;

                                auto &neighborGridCellParticles = m_grid->at(x + offsetX, y + offsetY, z + offsetZ);
                                for (auto &neighbor : neighborGridCellParticles) {

                                    QVector3D distance = (particle->position() - neighbor->position());
                                    double radiusSquared = distance.lengthSquared();

                                    if (radiusSquared <= CParticle::h * CParticle::h && particle->getId() != neighbor->getId()) {
                                        QVector3D spikyGradient = WspikyGradient(distance, radiusSquared);
                                        double viscosityLaplacian = WviscosityLaplacian(radiusSquared);

                                        f_pressure += (particle->pressure() / pow(particle->density(), 2) + (neighbor->pressure() / pow(neighbor->density(), 2))) * spikyGradient;
                                        f_viscosity += (neighbor->velocity() - particle->velocity()) * viscosityLaplacian / neighbor->density();
                                    }
                                }
                            }
                        }
                    }

                    f_pressure *= -CParticle::mass * particle->density();
                    f_viscosity *= CParticle::viscosity * CParticle::mass;

                    // ADD IN SPH FORCES
                    particle->acceleration() = (f_pressure + f_viscosity + f_gravity) / particle->density();

                    // collision force
                    particle->acceleration() += m_grid->getCollisionGeometry()->inverseBoundingBoxBounce(particle->position(), particle->velocity());
                }
            }
        }
    }

    return timer.elapsed();
}

double CCPUParticleSimulator::updateCollisions()
{
    // is part of updateForces
    return 0;
}

double CCPUParticleSimulator::integrate()
{
    QElapsedTimer timer;
    timer.start();

    for (unsigned int gridCellIndex = 0; gridCellIndex < m_grid->getCellCount(); gridCellIndex++) {
        auto &particles = m_grid->getData()[gridCellIndex];

        for (auto &particle : particles) {
            QVector3D newPosition = particle->position() + (particle->velocity() * dt) + particle->acceleration() * dt * dt;
            QVector3D newVelocity = (newPosition - particle->position()) / dt;

            particle->translate(newPosition);
            particle->velocity() = newVelocity;
        }
    }

    return timer.elapsed();
}
