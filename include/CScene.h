#ifndef CSCENE_H
#define CSCENE_H

#include<QObject>
#include <QEntity>


class CScene: QObject
{
Q_OBJECT
public:

    CScene();
    ~CScene();

    Qt3DCore::QEntity *getRootEntity() { return m_rootEntity; }
    void setRootEntity(Qt3DCore::QEntity *e) { m_rootEntity = e; }

    //TODO: Test method - delete later
    void createScene();

//private:
    Qt3DCore::QEntity *m_rootEntity;
};

#endif