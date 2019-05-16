#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

#include <QMainWindow>

class GOLScene;
class QWheelEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
public slots:
    
    void aliveCells(int count);
    void tickCount(int count);
    
    void cursorCoordsChanged(int col, int row);
    
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
    void renderPressed();
    
    void reloadFilePressed();
    
    
protected:
    
    virtual void wheelEvent(QWheelEvent* event);
    
    
private slots:
    
//    void fpsChanged(int fps);
    
private:
    
    // Attributes:
    
    Ui::MainWindow ui;
    
    GOLScene* m_scene;
    
    QString m_lastDir, m_lastFile;
    
    
    // Methods:
    
    void loadConfig();
    void saveConfig();
    
    void addShortcuts();
    
};

#endif // MAINWINDOW_H
