#ifndef CSCENE_H
#define CSCENE_H

#include<QObject>

//forward declaration
namespace Qt3DCore
{
    class QEntity;
};

class CGrid;

class CScene : QObject
{
    Q_OBJECT

public:

    CScene();
    ~CScene();

    Qt3DCore::QEntity *getRootEntity() { return m_rootEntity; }
    void setRootEntity(Qt3DCore::QEntity *e) { m_rootEntity = e; }
   
    //TODO: Test method - delete later
    void createSphere();

    void addGrid();

private:
    Qt3DCore::QEntity *m_rootEntity;
    CGrid *m_grid;
};

#endif