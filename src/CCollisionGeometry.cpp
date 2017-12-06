#include "CCollisionGeometry.h"


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

    QVector<Qt3DRender::QAttribute *> attributes = m_geometry->attributes();

    for (Qt3DRender::QAttribute *attr : attributes) {
        if (attr->name() == attr->defaultPositionAttributeName()) {
            posAttribute = attr;
        }

        if (attr->attributeType() == attr->IndexAttribute) {
            indexAttribute = attr;
        }

        if (attr->name() == attr->defaultNormalAttributeName()) {
            normalAttribute = attr;
        }
    }

    //needed atttributes were not found
    if (posAttribute == nullptr || indexAttribute == nullptr || normalAttribute == nullptr)
        return;

    switch (posAttribute->vertexBaseType()) {
        case Qt3DRender::QAttribute::VertexBaseType::Byte:
            extractVertices<char>(posAttribute, normalAttribute);
            break;
        case Qt3DRender::QAttribute::VertexBaseType::Double:
            extractVertices<double>(posAttribute, normalAttribute);
            break;
        case Qt3DRender::QAttribute::VertexBaseType::Float:
            extractVertices<float>(posAttribute, normalAttribute);
            break;
        case Qt3DRender::QAttribute::VertexBaseType::Int:
            extractVertices<int>(posAttribute, normalAttribute);
            break;
        case Qt3DRender::QAttribute::VertexBaseType::Short:
            extractVertices<short>(posAttribute, normalAttribute);
            break;
        case Qt3DRender::QAttribute::VertexBaseType::UnsignedInt:
            extractVertices<unsigned int>(posAttribute, normalAttribute);
            break;
        case Qt3DRender::QAttribute::VertexBaseType::UnsignedShort:
            extractVertices<unsigned short>(posAttribute, normalAttribute);
            break;
        default:
            break;
    }

    switch (indexAttribute->vertexBaseType()) {
        case Qt3DRender::QAttribute::VertexBaseType::Byte:
            extractFaces<char>(indexAttribute);
            break;
        case Qt3DRender::QAttribute::VertexBaseType::Double:
            extractFaces<double>(indexAttribute);
            break;
        case Qt3DRender::QAttribute::VertexBaseType::Float:
            extractFaces<float>(indexAttribute);
            break;
        case Qt3DRender::QAttribute::VertexBaseType::Int:
            extractFaces<int>(indexAttribute);
            break;
        case Qt3DRender::QAttribute::VertexBaseType::Short:
            extractFaces<short>(indexAttribute);
            break;
        case Qt3DRender::QAttribute::VertexBaseType::UnsignedInt:
            extractFaces<unsigned int>(indexAttribute);
            break;
        case Qt3DRender::QAttribute::VertexBaseType::UnsignedShort:
            extractFaces<unsigned short>(indexAttribute);
            break;
        default:
            break;
    }

    //BOUNDING BOX
    for (sVertex vertex : m_vertices) {
        m_boundingBox.expandBy(vertex.m_pos);
    }
}

QVector3D CCollisionGeometry::inverseBounce(QVector3D pos, QVector3D velocity)
{
    QVector3D acc(0, 0, 0);
    for (auto face : m_faces) {
        for (auto vertex : face.m_vertices) {
            QVector3D inverseNormal(face.m_normal * (-1));
            inverseNormal.normalize();

            double d = QVector3D::dotProduct(vertex.m_pos - pos, inverseNormal) + 0.01; // particle radius

            if (d > 0.0) {
                acc += QVector3D(5000.0 * inverseNormal * d);
                acc += -0.9 * QVector3D::dotProduct(velocity, inverseNormal) * inverseNormal;
            }
        }
    }

    return acc;
}

#define WALL_K 10000.0 // wall spring constant
#define WALL_DAMPING (-0.9) // wall damping constant

QVector3D CCollisionGeometry::inverseBoundingBoxBounce(QVector3D &pos, QVector3D &velocity)
{
    QVector3D acc(0, 0, 0);

    for (sBoundingBox::tWall wall : m_boundingBox.m_walls) {
        QVector3D inverseNormal(wall.first * (-1));

        double d = QVector3D::dotProduct(wall.second - pos, inverseNormal) + 0.01; // particle radius

        if (d > 0.0) {
            // This is an alernate way of calculating collisions of particles against walls, but produces some jitter at boundaries
//            pos += d * wall.first;
//            velocity -= QVector3D::dotProduct(velocity, wall.first) * 1.9 * wall.first;

            acc += WALL_K * inverseNormal * d;
            acc += WALL_DAMPING * QVector3D::dotProduct(velocity, inverseNormal) * inverseNormal;
        }
    }

    return acc;
}
