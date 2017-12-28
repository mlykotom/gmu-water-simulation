
#ifndef WATERSURFACESIMULATION_SSIMULATOREVENT_H
#define WATERSURFACESIMULATION_SSIMULATOREVENT_H

#include <QDebug>
#include <QDebugStateSaver>

struct sProfilingEvent
{
    unsigned long iteration;
    double fps;
    double updateGrid;
    double updateDensityPressure;
    double updateForces;
    double updateCollisions;
    double integrate;

    explicit sProfilingEvent(unsigned long iteration)
        : iteration(iteration),
          fps(0),
          updateGrid(0),
          updateDensityPressure(0),
          updateForces(0),
          updateCollisions(0),
          integrate(0) {}

    double together() const
    {
        return updateGrid + updateDensityPressure + updateForces + updateCollisions + integrate;
    }
};

static QDebug operator<<(QDebug debug, const sProfilingEvent &e)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << '('
                    << "i=" << e.iteration << " | "
                    << "fps=" << e.fps << ", "
                    << "grid=" << e.updateGrid << ", "
                    << "density=" << e.updateDensityPressure << ", "
                    << "forces=" << e.updateForces << ", "
                    << "collisions=" << e.updateCollisions << ", "
                    << "integrate=" << e.integrate << ", "
                    << "together = " << e.together()
                    << ")";

    return debug;
}

#endif //WATERSURFACESIMULATION_SSIMULATOREVENT_H
