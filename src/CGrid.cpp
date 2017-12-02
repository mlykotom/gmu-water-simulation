#include "CGrid.h"
#include "CWireframeMaterial.h"
#include "CRobustWireframeMaterial.h"

#include <Qt3DExtras/qcuboidmesh.h>

#include <QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QBufferDataGeneratorPtr>
#include <Qt3DRender/QBufferDataGenerator>


CGrid::CGrid(int x, int y, int z, Qt3DCore::QNode * parent)
    : RenderableEntity(parent),
    m_material(new Qt3DExtras::QPhongMaterial()),
    m_meshRenderer(new Qt3DRender::QGeometryRenderer()),
    m_x(x), 
    m_y(y),
    m_z(z), 
    m_cell_count(x * y * z)
{
    m_data = new std::vector<CParticle *>[m_cell_count];

    //Cuboid geometry
    m_geometry = new Qt3DExtras::QCuboidGeometry(this);
    m_geometry->setZExtent(z);
    m_geometry->setYExtent(y);
    m_geometry->setXExtent(x);



    m_meshRenderer->setGeometry(m_geometry);
    m_meshRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);
    addComponent(m_meshRenderer);

    //Custom material
    CWireframeMaterial *wireframeMaterial = new CWireframeMaterial();
    addComponent(wireframeMaterial);

    this->setEnabled(true);

}


CGrid::CGrid(Qt3DCore::QNode *parent)
    : RenderableEntity(parent),
    m_material(new Qt3DExtras::QPhongMaterial()),
    m_meshRenderer(new Qt3DRender::QGeometryRenderer()),
    m_x(0),
    m_y(0),
    m_z(0),
    m_cell_count(0)
{
    //Translation
    m_transform->setScale(4.0f);
    m_transform->setTranslation(QVector3D(0, 0, -10.0f)); 

    //Cuboid geometry
    m_geometry = new Qt3DExtras::QCuboidGeometry(this);
    m_geometry->updateVertices();
    m_geometry->updateIndices();


    m_meshRenderer->setGeometry(m_geometry);
    m_meshRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);
    addComponent(m_meshRenderer);


    /*============================ Working area ====================================================*/
    //m_geometry->boundingVolumePositionAttribute()->vertexSize();

    //Qt3DRender::QAttribute *posAttribute = m_geometry->positionAttribute();
    //qDebug() << posAttribute->vertexSize();
    //qDebug() << posAttribute->vertexBaseType();
    //qDebug() << posAttribute->byteOffset();
    //qDebug() << posAttribute->byteStride() / sizeof(float);
    //qDebug() << posAttribute->count();

    //Qt3DRender::QBuffer *posBuffer = m_geometry->positionAttribute()->buffer();
    //



    //Qt3DRender::QBufferDataGeneratorPtr generator = posBuffer->dataGenerator();

    //Qt3DRender::QBufferDataGenerator * gen = generator.operator->();
    //QByteArray arr = gen->operator()();


    //float* vertices = reinterpret_cast<float*>(arr.data());
    ////int size = sizeof(vertices) / sizeof(*vertices);

    //int size = arr.size() / sizeof(float);


    //for (int i = 0; i < size; ++i)
    //{
    //    qDebug() << vertices[i];
    //}
    /*============================ Working area ====================================================*/


    //Custom material
    CWireframeMaterial *wireframeMaterial = new CWireframeMaterial();
    addComponent(wireframeMaterial);

    //CRobustWireframeMaterial *robustWireframeMaterial = new CRobustWireframeMaterial();
    //addComponent(robustWireframeMaterial);

    this->setEnabled(true);
}
CGrid::~CGrid()
{
    //delete[] m_data;
}

std::vector<CParticle*> CGrid::getNeighborsCells(int x, int y, int z)
{
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
}
