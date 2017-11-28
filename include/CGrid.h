//
// Created by Tomáš Mlynarič on 27.11.17.
//

#ifndef WATERSURFACESIMULATION_CGRID_H
#define WATERSURFACESIMULATION_CGRID_H

#include <QVector3D>
#include "CParticle.h"
class CGrid
{
private:
    std::vector<CParticle *> *m_data;
    const int m_cell_count, m_x, m_y, m_z;

public:
    explicit CGrid(int x, int y, int z) : m_x(x), m_y(y), m_z(z), m_cell_count(x * y * z)
    {
        m_data = new std::vector<CParticle *>[m_cell_count];
    }

    virtual ~CGrid()
    {
        delete[] m_data;
    }

    std::vector<CParticle *> *getData() const { return m_data; }
    std::vector<CParticle *> &at(int x, int y, int z)
    {
        return m_data[x + y * m_y + z * m_z * m_y];
    }

    const int xRes() const { return m_x; }
    const int yRes() const { return m_y; }
    const int zRes() const { return m_z; }
    const int &getCellCount() const { return m_cell_count; }

    std::vector<CParticle *> getNeighborsCells(int x, int y, int z)
    {
        auto neighborParticles = std::vector<CParticle *>();

        for (int offsetX = -1; offsetX <= 1; offsetX++) {
            if (x + offsetX < 0) continue;
            if (x + offsetX >= xRes()) break;

            for (int offsetY = -1; offsetY <= 1; offsetY++) {
                if (y + offsetY < 0) continue;
                if (y + offsetY >= yRes()) break;

                for (int offsetZ = -1; offsetZ <= 1; offsetZ++) {
                    if (z + offsetZ < 0) continue;
                    if (z + offsetZ >= zRes()) break;

                    auto &particlesAtCell = at(x + offsetX, y + offsetY, z + offsetZ);
                    neighborParticles.insert(neighborParticles.end(), particlesAtCell.begin(), particlesAtCell.end());
                }
            }
        }

        return neighborParticles;
    }
};

#endif //WATERSURFACESIMULATION_CGRID_H
