#include <include/CLPlatforms.h>
#include "CCPUBruteParticleSimulator.h"


CCPUBruteParticleSimulator::CCPUBruteParticleSimulator(CScene *scene, QObject *parent)
    : CBaseParticleSimulator(scene, parent)
{
    CLPlatforms::printInfoAll();

    auto clDevice = CLPlatforms::getBestGPU();
    m_cl_wrapper = new CLWrapper(clDevice);
    qDebug() << "Selected device: " << CLPlatforms::getDeviceInfo(m_cl_wrapper->getDevice());
}

void CCPUBruteParticleSimulator::updateGrid()
{
    // don't need to update the grid, since everything is in one cell
}

void CCPUBruteParticleSimulator::updateDensityPressure()
{
    auto &particles = m_grid->at(0, 0, 0);
    for (auto &particle : particles) {

        particle->density() = 0.0;
        auto &neighborGridCellParticles = m_grid->at(0, 0, 0);

        for (auto &neighbor : neighborGridCellParticles) {
            double radiusSquared = particle->diffPosition(neighbor).lengthSquared();

            if (radiusSquared <= CParticle::h * CParticle::h) {
                particle->density() += Wpoly6(radiusSquared);
            }
        }

        particle->density() *= CParticle::mass;
        // p = k(density - density_rest)
        particle->pressure() = CParticle::gas_stiffness * (particle->density() - CParticle::rest_density);
    }
}

void CCPUBruteParticleSimulator::updateForces()
{
    auto &particles = m_grid->at(0, 0, 0);

    for (auto &particle : particles) {
        QVector3D f_gravity = gravity * particle->density();
        QVector3D f_pressure, f_viscosity, f_surface;

        auto &neighborGridCellParticles = m_grid->at(0, 0, 0);

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

        f_pressure *= -CParticle::mass * particle->density();
        f_viscosity *= CParticle::viscosity * CParticle::mass;

        // ADD IN SPH FORCES
        particle->acceleration() = (f_pressure + f_viscosity + f_gravity) / particle->density();
        // collision force
        particle->acceleration() += m_grid->getCollisionGeometry()->inverseBoundingBoxBounce(particle->position(), particle->velocity());

    }
}

void CCPUBruteParticleSimulator::test(double dt, QVector3D position, QVector3D velocity, QVector3D acceleration, QVector3D &newPosition, QVector3D &newVelocity)
{
    newPosition = position + (velocity * dt) + acceleration * dt * dt;
    newVelocity = (newPosition - position) / dt;
}

void CCPUBruteParticleSimulator::integrate()
{
    std::vector<CParticle *> &particles = m_grid->getData()[0];

    for (auto &particle : particles) {
        QVector3D newPosition, newVelocity;

        test(dt, particle->position(), particle->velocity(), particle->acceleration(), newPosition, newVelocity);

        particle->translate(newPosition);
        particle->velocity() = newVelocity;
    }
}

