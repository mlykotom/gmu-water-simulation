#include <include/CLPlatforms.h>
#include "CGPUParticleSimulator.h"

// TODO update this shit! yeah


CGPUParticleSimulator::CGPUParticleSimulator(CScene *scene, QObject *parent)
    : CBaseParticleSimulator(scene, parent)
{
    CLPlatforms::printInfoAll();

    auto clDevice = CLPlatforms::getBestGPU();
    m_cl_wrapper = new CLWrapper(clDevice);
    qDebug() << "Selected device: " << CLPlatforms::getDeviceInfo(m_cl_wrapper->getDevice());
}

void CGPUParticleSimulator::updateGrid()
{
    for (int x = 0; x < m_grid->xRes(); x++) {
        for (int y = 0; y < m_grid->yRes(); y++) {
            for (int z = 0; z < m_grid->zRes(); z++) {

                std::vector<CParticle *> &particles = m_grid->at(x, y, z);

                for (unsigned long p = 0; p < particles.size(); p++) {
                    CParticle *particle = particles[p];

                    int newGridCellX = (int) floor((particle->position().x() + boxSize.x() / 2.0) / CParticle::h);
                    int newGridCellY = (int) floor((particle->position().y() + boxSize.y() / 2.0) / CParticle::h);
                    int newGridCellZ = (int) floor((particle->position().z() + boxSize.z() / 2.0) / CParticle::h);
                    //                        qDebug() << x << y << z << "NEW" << newGridCellX << newGridCellY << newGridCellZ;
                    //cout << "particle position: " << particle->position() << endl;
                    //cout << "particle cell pos: " << newGridCellX << " " << newGridCellY << " " << newGridCellZ << endl;

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

                    //cout << "particle cell pos: " << newGridCellX << " " << newGridCellY << " " << newGridCellZ << endl;


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
}

void CGPUParticleSimulator::updateDensityPressure()
{
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
                                    double radiusSquared = particle->diffPosition(neighbor).lengthSquared();

                                    if (radiusSquared <= CParticle::h * CParticle::h) {
                                        particle->density() += Wpoly6(radiusSquared);
                                    }
                                }
                            }
                        }
                    }

                    particle->density() *= CParticle::mass;
                    // p = k(density - density_rest)
                    particle->pressure() = CParticle::gas_stiffness * (particle->density() - CParticle::rest_density);
                }
            }
        }
    }
}

void CGPUParticleSimulator::updateForces()
{
    for (int x = 0; x < m_grid->xRes(); x++) {
        for (int y = 0; y < m_grid->yRes(); y++) {
            for (int z = 0; z < m_grid->zRes(); z++) {

                auto &particles = m_grid->at(x, y, z);

                for (auto &particle : particles) {
                    QVector3D f_gravity = gravity * particle->density();
                    QVector3D f_pressure, f_viscosity, f_surface;

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

                                    QVector3D distance = particle->diffPosition(neighbor);
                                    double radiusSquared = distance.lengthSquared();

                                    if (radiusSquared <= CParticle::h * CParticle::h) {
                                        QVector3D poly6Gradient = Wpoly6Gradient(distance, radiusSquared);
                                        QVector3D spikyGradient = WspikyGradient(distance, radiusSquared);

                                        if (particle->getId() != neighbor->getId()) {
                                            f_pressure += (particle->pressure() / pow(particle->density(), 2) + neighbor->pressure() / pow(neighbor->density(), 2)) * spikyGradient;
                                            f_viscosity += (neighbor->velocity() - particle->velocity()) * WviscosityLaplacian(radiusSquared) / neighbor->density();
                                        }
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
}

void CGPUParticleSimulator::updateNewPositionVelocity()
{
    for (unsigned int gridCellIndex = 0; gridCellIndex < m_grid->getCellCount(); gridCellIndex++) {
        auto &particles = m_grid->getData()[gridCellIndex];

        for (auto &particle : particles) {
            QVector3D newPosition = particle->position() + (particle->velocity() * dt) + particle->acceleration() * dt * dt;
            QVector3D newVelocity = (newPosition - particle->position()) / dt;

            particle->translate(newPosition);
            particle->velocity() = newVelocity;
        }
    }
}
