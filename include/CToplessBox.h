#ifndef CTOPLESSBOX_H
#define CTOPLESSBOX_H

#include "renderableentity.h"

//Qt
#include <QNode>
#include <QGeometry>

class CToplessBox : RenderableEntity
{
public:
    explicit CToplessBox(int x = 1, int y = 1, int z = 1, Qt3DCore::QNode *parent = 0);
    ~CToplessBox();

private: 
    Qt3DRender::QGeometry *m_geometry;

private:
    void createGeomtry();
};

//class CToplessBox : RenderableEntity
//{
//    explicit CToplessBox(int x, int y, int z, Qt3DCore::QNode *parent = 0);
//    ~CToplessBox();
//};

#endif