#include "CToplessBox.h"


#include <QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DExtras/QPhongMaterial>

CToplessBox::CToplessBox(int x, int y, int z, Qt3DCore::QNode *parent)
    : RenderableEntity(parent)
{
    createGeomtry();
    
    //m_geometry->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);
    Qt3DExtras::QPhongMaterial* m_material = new Qt3DExtras::QPhongMaterial();
    Qt3DRender::QGeometryRenderer *m_meshRenderer = new Qt3DRender::QGeometryRenderer();

    m_meshRenderer->setGeometry(m_geometry);
    m_meshRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Lines);

    addComponent(m_meshRenderer);
    addComponent(m_material);

    this->setEnabled(true);
}

CToplessBox::~CToplessBox()
{
}

void CToplessBox::createGeomtry()
{
    //create empty geometry
    m_geometry = new Qt3DRender::QGeometry(this);

    //create all needed attributes
    Qt3DRender::QAttribute *m_positionAttribute = new Qt3DRender::QAttribute(m_geometry);
    Qt3DRender::QAttribute *m_indexAttribute = new Qt3DRender::QAttribute(m_geometry);

    //buffers
    Qt3DRender::QBuffer *m_vertexBuffer = new Qt3DRender::QBuffer();
    Qt3DRender::QBuffer *m_indexBuffer = new Qt3DRender::QBuffer();

    const quint32 stride = 3;
    const int nVerts = 8;

    QByteArray vertexBytes;
    vertexBytes.resize(nVerts *stride);
    float* vertices = reinterpret_cast<float*>(vertexBytes.data());
    
    //A
    *vertices++ = 1;
    *vertices++ = 1;
    *vertices++ = 1;

    //B
    *vertices++ = 1;
    *vertices++ = 1;
    *vertices++ = -1;

    //C
    *vertices++ = 1;
    *vertices++ = 0;
    *vertices++ = -1;

    //D
    *vertices++ = 1;
    *vertices++ = 0;
    *vertices++ = 1;

    //E
    *vertices++ = -1;
    *vertices++ = 0;
    *vertices++ = 1;

    //F
    *vertices++ = -1;
    *vertices++ = 0;
    *vertices++ = -1;

    //G
    *vertices++ = -1;
    *vertices++ = 1;
    *vertices++ = 1;

    //H
    *vertices++ = -1;
    *vertices++ = 1;
    *vertices++ = -1;

    m_vertexBuffer->setData(vertexBytes);

    const int indexCount = 20;

    QByteArray indexData;
    indexData.resize(indexCount * sizeof(quint16));
    quint16 *indices = reinterpret_cast<quint16 *>(indexData.data());
    

    *indices++ = 0;
    *indices++ = 1;
    *indices++ = 2;
    *indices++ = 3;

    *indices++ = 2;
    *indices++ = 3;
    *indices++ = 4;
    *indices++ = 5;


    *indices++ = 4;
    *indices++ = 5;
    *indices++ = 6;
    *indices++ = 7;


    *indices++ = 2;
    *indices++ = 1;
    *indices++ = 7;
    *indices++ = 5;


    *indices++ = 0;
    *indices++ = 3;
    *indices++ = 4;
    *indices++ = 6;

    m_indexBuffer->setData(indexData);

    m_positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
    m_positionAttribute->setVertexBaseType(Qt3DRender::QAttribute::Float);
    m_positionAttribute->setVertexSize(3);
    m_positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    m_positionAttribute->setBuffer(m_vertexBuffer);
    m_positionAttribute->setByteStride(stride);
    m_positionAttribute->setCount(nVerts);
    


    m_indexAttribute->setAttributeType(Qt3DRender::QAttribute::IndexAttribute);
    m_indexAttribute->setVertexBaseType(Qt3DRender::QAttribute::UnsignedShort);
    m_indexAttribute->setBuffer(m_indexBuffer);

    //add all attributes
    m_geometry->addAttribute(m_positionAttribute);
    m_geometry->addAttribute(m_indexAttribute);



}
