#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "golscene.h"

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QStandardPaths>
#include <QThread>
#include <QWheelEvent>

#include <omp.h>


#define WINDOW_TITLE "Game Of Life Demo"


MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , m_lastDir(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation))
{
    ui->setupUi(this);
    setWindowTitle(WINDOW_TITLE);
    
    ui->ColumnsSpin->setValue(GRID_WIDTH);
    ui->RowsSpin->setValue(GRID_HEIGHT);
    ui->fpsSpinbox->setValue(START_FPS);
    ui->CellSizeSpin->setValue(CELL_SIZE);
    
    m_scene = new GOLScene();
    
    connect(m_scene, SIGNAL(aliveCellsSignal(int)), this, SLOT(aliveCells(int)));
    connect(m_scene, SIGNAL(tickCountSignal(int)), this, SLOT(tickCount(int)));
    connect(m_scene, SIGNAL(pauseSignal(bool)), this, SLOT(setPaused(bool)));
    connect(m_scene, SIGNAL(colsSignal(int)), this, SLOT(sceneSetCols(int)));
    connect(m_scene, SIGNAL(rowsSignal(int)), this, SLOT(sceneSetRows(int)));
    
    connect(ui->PauseButton, SIGNAL(pressed()), this, SLOT(pausePressed()));
    connect(ui->NextTickButton, SIGNAL(pressed()), this, SLOT(nextTickPressed()));
    connect(ui->ResetButton, SIGNAL(pressed()), this, SLOT(resetPressed()));
    connect(ui->LoadButton, SIGNAL(pressed()), this, SLOT(loadPressed()));
    connect(ui->SaveButton, SIGNAL(pressed()), this, SLOT(savePressed()));
    connect(ui->ChaosButton, SIGNAL(pressed()), this, SLOT(chaosPressed()));
    
    connect(ui->fpsSpinbox, SIGNAL(valueChanged(int)), m_scene, SLOT(fpsChanged(int)));
    connect(ui->CellSizeSpin, SIGNAL(valueChanged(int)), this, SLOT(cellSizeChanged(int)));
    connect(ui->ColumnsSpin, SIGNAL(valueChanged(int)), this, SLOT(colsChanged(int)));
    connect(ui->RowsSpin, SIGNAL(valueChanged(int)), this, SLOT(rowsChanged(int)));
    
    
    QAction* pauseSpace = new QAction(this);
    pauseSpace->setShortcut(QKeySequence(Qt::Key_Space));
    connect(pauseSpace, SIGNAL(triggered(bool)), this, SLOT(pausePressed()));
    addAction(pauseSpace);
    
    QAction* dotNextTick = new QAction(this);
    dotNextTick->setShortcut(QKeySequence(Qt::Key_Period));
    connect(dotNextTick, SIGNAL(triggered(bool)), this, SLOT(nextTickPressed()));
    addAction(dotNextTick);
    
    QAction* rReset = new QAction(this);
    rReset->setShortcut(QKeySequence(Qt::Key_R));
    connect(rReset, SIGNAL(triggered(bool)), this, SLOT(resetPressed()));
    addAction(rReset);
    
    QAction* sSave = new QAction(this);
    sSave->setShortcut(QKeySequence(Qt::Key_Control | Qt::Key_S));
    connect(sSave, SIGNAL(triggered(bool)), this, SLOT(savePressed()));
    addAction(sSave);
    
    QAction* eLoad = new QAction(this);
    eLoad->setShortcut(QKeySequence(Qt::Key_Control | Qt::Key_E));
    connect(eLoad, SIGNAL(triggered(bool)), this, SLOT(loadPressed()));
    addAction(eLoad);
    
    QAction* oLoad = new QAction(this);
    oLoad->setShortcut(QKeySequence(Qt::Key_O));
    connect(oLoad, SIGNAL(triggered(bool)), this, SLOT(loadPressed()));
    addAction(oLoad);
    
    QAction* plus = new QAction(this);
    plus->setShortcut(QKeySequence(Qt::Key_Plus));
    connect(plus, SIGNAL(triggered(bool)), this, SLOT(cellSizeInc()));
    addAction(plus);
    
    QAction* dash = new QAction(this);
    dash->setShortcut(QKeySequence(Qt::Key_Minus));
    connect(dash, SIGNAL(triggered(bool)), this, SLOT(cellSizeDec()));
    addAction(dash);
    
    if (m_scene->paused())
        setWindowTitle(QString(WINDOW_TITLE) + " (Paused)");
    
    ui->graphicsView->setScene(m_scene);
    //ui->graphicsView->setSceneRect(QRectF());
}

MainWindow::~MainWindow()
{
    delete m_scene;
    delete ui;
}


void MainWindow::aliveCells(int count)
{
    ui->AliveCellsLabel->setText(QString("Alive Cells: %1").arg(count));
}

void MainWindow::tickCount(int count)
{
    ui->EvolutionsLabel->setText(QString("Evolutions: %1").arg(count));
}


void MainWindow::pausePressed()
{
    setPaused(!m_scene->paused());
}

void MainWindow::setPaused(bool paused)
{
    m_scene->pauseChanged(paused);
    
    if (paused)
    {
        ui->PauseButton->setText("Start");
        setWindowTitle(QString(WINDOW_TITLE) + " (Paused)");
    }
    else
    {
        ui->PauseButton->setText("Pause");
        setWindowTitle(WINDOW_TITLE);
    }
}

void MainWindow::nextTickPressed()
{
    m_scene->tick();
}

void MainWindow::resetPressed()
{
    m_scene->reset();
}

void MainWindow::loadPressed()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Save File", 
                           m_lastDir,
                           "Save-File (*.gol)");
    
    if (!fileName.isEmpty())
    {
        QFileInfo file(fileName);
        
        m_lastDir = file.absoluteDir().absolutePath();
        
        m_scene->load(file.absoluteFilePath());
    }
}

void MainWindow::savePressed()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save File", 
                           m_lastDir,
                           "Save-File (*.gol)");
    
    if (!fileName.isEmpty())
    {
        QFileInfo file(fileName);
        
        m_lastDir = file.absoluteDir().absolutePath();
        
        if (file.exists())
        {
            for (int i = 0; i < 10; ++i)
            {
                if (QFile(fileName).remove())
                    break;
                QThread::msleep(50);
            }
        }
        
        m_scene->save(file.absoluteFilePath());
    }
}


void MainWindow::cellSizeChanged(int size)
{
    size = std::max(size, 1);
    m_scene->setCellSize(size);
    m_scene->update();
}

void MainWindow::cellSizeInc()
{
    cellSizeChanged(m_scene->cellSize()+1);
}

void MainWindow::cellSizeDec()
{
    cellSizeChanged(m_scene->cellSize()-1);
}

void MainWindow::wheelEvent(QWheelEvent* event)
{
    if (QApplication::keyboardModifiers() & Qt::ControlModifier)
    {
        if (event->angleDelta().y() < 0)
            cellSizeDec();
        else if (event->angleDelta().y() > 0)
            cellSizeInc();
        
        event->accept();
    }
    
    QMainWindow::wheelEvent(event);
}

void MainWindow::rowsChanged(int rows)
{
    m_scene->setRows(rows);
}

void MainWindow::colsChanged(int cols)
{
    m_scene->setColumns(cols);
}

void MainWindow::sceneSetRows(int rows)
{
    bool prev = ui->RowsSpin->blockSignals(true);
    ui->RowsSpin->setValue(rows);
    ui->RowsSpin->blockSignals(prev);
}

void MainWindow::sceneSetCols(int cols)
{
    bool prev = ui->ColumnsSpin->blockSignals(true);
    ui->ColumnsSpin->setValue(cols);
    ui->ColumnsSpin->blockSignals(prev);
}

void MainWindow::chaosPressed()
{
    m_scene->chaos();
}
