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
    Qt3DRender::QAttribute *indexAttribute = nullptr;
    Qt3DRender::QAttribute *normalAttribute = nullptr;

    QVector<Qt3DRender::QAttribute *> attributes =  m_geometry->attributes();

    for (Qt3DRender::QAttribute * attr : attributes)
    {
        if (attr->name() == attr->defaultPositionAttributeName())
        {
            posAttribute = attr;
        }

        if (attr->attributeType() == attr->IndexAttribute)
        {
            indexAttribute = attr;
        }

        if (attr->name() == attr->defaultNormalAttributeName())
        {
            normalAttribute = attr;
        }
    }

    //position atttribute was not found
    if (posAttribute == nullptr || indexAttribute == nullptr || normalAttribute == nullptr)
        return;

    //TODO dat do templatovej funckie !!!!!!!!!!!!!!!!!!!!!!!!!!

    //qDebug() << posAttribute->vertexBaseType();

    int vertexSize = posAttribute->vertexSize();
    int vertexCount = posAttribute->count();
    int vertexByteOffset = posAttribute->byteOffset() / sizeof(float);
    int vertexByteStride = posAttribute->byteStride() / sizeof(float);

    int normalByteStride = posAttribute->byteStride() / sizeof(float);
    int normalByteOffset = normalAttribute->byteOffset() / sizeof(float);
    int normalCount = normalAttribute->count();

    //qDebug() << vertexSize;
    //qDebug() << vertexCount;

    //qDebug() << vertexByteOffset;
    //qDebug() << vertexByteStride;


    Qt3DRender::QBuffer *posBuffer = posAttribute->buffer();
    Qt3DRender::QBufferDataGeneratorPtr generator = posBuffer->dataGenerator();
    Qt3DRender::QBufferDataGenerator * gen = generator.operator->();
    QByteArray posArr = gen->operator()();
    float* vertices = reinterpret_cast<float*>(posArr.data());

    ////int size = sizeof(vertices) / sizeof(*vertices);

    int size = posArr.size() / sizeof(float);

    for (int i = 0; i < size; i += vertexByteStride)
    {

        QVector3D pos(vertices[i + vertexByteOffset], vertices[i + vertexByteOffset + 1], vertices[i + vertexByteOffset + 2]);
        QVector3D normal(vertices[i + normalByteOffset], vertices[i + normalByteOffset + 1], vertices[i + normalByteOffset + 2]);

        m_vertices.push_back(sVertex(pos,normal));
        

        //position
        //qDebug() << vertices[i];
        //qDebug() << vertices[i + 1];
        //qDebug() << vertices[i + 2];

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

        //qDebug() << "-----------------------------------";

    }

    //QByteArray indexArr =  indexAttribute->buffer()->dataGenerator().operator->()->operator()();
    
    qDebug() << indexAttribute->vertexBaseType();
    Qt3DRender::QBuffer *indexBuffer = indexAttribute->buffer();
    Qt3DRender::QBufferDataGeneratorPtr indexGenerator = indexBuffer->dataGenerator();
    Qt3DRender::QBufferDataGenerator * indexGen = indexGenerator.operator->();
    QByteArray indexArr = indexGen->operator()();
    unsigned short* indices = reinterpret_cast<unsigned short *>(indexArr.data());

    size = indexArr.size() / sizeof(unsigned short);

    for (int i = 0; i < size; i +=3)
    {
        m_faces.push_back(sFace(m_vertices.at(indices[i]), m_vertices.at(indices[i+1]), m_vertices.at(indices[i+2])));
        qDebug() << indices[i] << indices[i+1]<< indices[i+2];
    }
}
