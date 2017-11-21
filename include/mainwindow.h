#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

//forward declarations
namespace Qt3DExtras
{
    class Qt3DWindow;
}
class CScene;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    Qt3DExtras::Qt3DWindow *getView() { return m_view; }

private:
    //UI
    Ui::MainWindow *ui;

    Qt3DExtras::Qt3DWindow *m_view;
    CScene *m_scene;
};


#endif // MAINWINDOW_H