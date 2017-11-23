#ifndef WATERSURFACESIMULATION_TEST_H
#define WATERSURFACESIMULATION_TEST_H

#include <vector>

class Particle
{

};

class Grid
{
    typedef std::vector<Particle> particleVector;

private:
    particleVector* data;
    int sizeX;
    int sizeY;
    int sizeZ;

public:
    Grid(int xSize, int ySize, int zSize = 1)
    {
        this->sizeX = xSize;
        this->sizeY = ySize;
        this->sizeZ = zSize;

        data = new particleVector(xSize * ySize * zSize);
    }

    virtual ~Grid()
    {
        delete[] data;
    }

    inline particleVector &get(int x, int y, int z = 1)
    {
        return data[x + y * sizeX + z * sizeX * sizeY];
    }
};

class test
{

public:
    void start();
};

void test::start()
{
//    auto grid = Grid(5, 5);
//    auto banan = FluidCubeCreate(5, 1, 2, 0);
//    FluidCubeStep(banan);
}

#endif //WATERSURFACESIMULATION_TEST_H
