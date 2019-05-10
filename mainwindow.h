#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class GOLScene;
class QWheelEvent;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
public slots:
    
    void aliveCells(int count);
    void tickCount(int count);
    
    void rowsChanged(int rows);
    void colsChanged(int cols);
    
    void cellSizeChanged(int size);
    void cellSizeInc();
    void cellSizeDec();
    
private slots:
    
    void sceneSetRows(int rows);
    void sceneSetCols(int cols);
    
    void pausePressed();
    void setPaused(bool paused);
    void nextTickPressed();
    void savePressed();
    void loadPressed();
    void resetPressed();
    void chaosPressed();
    
    
protected:
    
    virtual void wheelEvent(QWheelEvent* event);
    
    
private slots:
    
//    void fpsChanged(int fps);
    
private:
    
    Ui::MainWindow *ui;
    
    GOLScene* m_scene;
    
    QString m_lastDir;
    
};

#endif // MAINWINDOW_H
