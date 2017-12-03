#include "CCollisionGeometry.h"


#include <QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QBufferDataGeneratorPtr>
#include <Qt3DRender/QBufferDataGenerator>

CCollisionGeometry::CCollisionGeometry(Qt3DRender::QGeometry *geometry, Qt3DCore::QNode *parent)
    : Qt3DCore::QEntity(parent),
    m_geometry(geometry)
{
    init();
}

CCollisionGeometry::~CCollisionGeometry()
{
}

void CCollisionGeometry::init()
{

    Qt3DRender::QAttribute *posAttribute = nullptr;

    QVector<Qt3DRender::QAttribute *> attributes =  m_geometry->attributes();

    for (Qt3DRender::QAttribute * attr : attributes)
    {
        if (attr->name() == attr->defaultPositionAttributeName())
        {
            posAttribute = attr;
            break;
        }
    }

    //position atttribute was not found
    if (posAttribute == nullptr) 
        return;

    //TODO dat do templatovej funckie

    int vertexSize = posAttribute->vertexSize();
    int vertexCount = posAttribute->count();
    int vertexByteOffset = posAttribute->byteOffset();
    int vertexByteStride = posAttribute->byteStride() / sizeof(float);
    //qDebug() << posAttribute->vertexBaseType();

    qDebug() << vertexSize;
    qDebug() << vertexCount;

    qDebug() << vertexByteOffset;
    qDebug() << vertexByteStride;


    Qt3DRender::QBuffer *posBuffer = posAttribute->buffer();

    Qt3DRender::QBufferDataGeneratorPtr generator = posBuffer->dataGenerator();

    Qt3DRender::QBufferDataGenerator * gen = generator.operator->();
    QByteArray arr = gen->operator()();


    float* vertices = reinterpret_cast<float*>(arr.data());
    ////int size = sizeof(vertices) / sizeof(*vertices);

    int size = arr.size() / sizeof(float);

    qDebug() << "-----------------------------------";
    for (int i = vertexByteOffset; i < size; i += vertexByteStride)
    {
        m_vertices.push_back();
        sVertex
        QVector3D(vertices[i], vertices[i + 1], vertices[i + 2]);

        //position
        qDebug() << vertices[i];
        qDebug() << vertices[i + 1];
        qDebug() << vertices[i + 2];
        // texture coordinates
        //qDebug() << vertices[i + 3];
        //qDebug() << vertices[i + 4];
        //// normal
        //qDebug() << vertices[i + 5];
        //qDebug() << vertices[i + 6];
        //qDebug() << vertices[i + 7];
        //// tangent
        //qDebug() << vertices[i + 8];
        //qDebug() << vertices[i + 9];
        //qDebug() << vertices[i + 10];
        //qDebug() << vertices[i + 11];

        qDebug() << "-----------------------------------";

    }
    qDebug() << "-----------------------------------";
}
