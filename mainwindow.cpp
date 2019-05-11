#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "golscene.h"
#include "renderdialog.h"

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QStandardPaths>
#include <QThread>
#include <QWheelEvent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTextStream>

#include <omp.h>


#define WINDOW_TITLE "Game Of Life Demo"


MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , m_lastDir(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation))
  , m_lastFile("NewState.gol")
{
    ui.setupUi(this);
    setWindowTitle(WINDOW_TITLE);
    
    ui.ColumnsSpin->setValue(GRID_WIDTH);
    ui.RowsSpin->setValue(GRID_HEIGHT);
    ui.fpsSpinbox->setValue(START_FPS);
    ui.CellSizeSpin->setValue(CELL_SIZE);
    
    
    loadConfig();
    
    
    m_scene = new GOLScene();
    m_scene->setCellSize(ui.CellSizeSpin->value());
    
    connect(m_scene, SIGNAL(aliveCellsSignal(int)), this, SLOT(aliveCells(int)));
    connect(m_scene, SIGNAL(tickCountSignal(int)), this, SLOT(tickCount(int)));
    connect(m_scene, SIGNAL(pauseSignal(bool)), this, SLOT(setPaused(bool)));
    connect(m_scene, SIGNAL(colsSignal(int)), this, SLOT(sceneSetCols(int)));
    connect(m_scene, SIGNAL(rowsSignal(int)), this, SLOT(sceneSetRows(int)));
    
    connect(ui.PauseButton, SIGNAL(pressed()), this, SLOT(pausePressed()));
    connect(ui.NextTickButton, SIGNAL(pressed()), this, SLOT(nextTickPressed()));
    connect(ui.RenderButton, SIGNAL(pressed()), this, SLOT(renderPressed()));
    connect(ui.ResetButton, SIGNAL(pressed()), this, SLOT(resetPressed()));
    connect(ui.LoadButton, SIGNAL(pressed()), this, SLOT(loadPressed()));
    connect(ui.SaveButton, SIGNAL(pressed()), this, SLOT(savePressed()));
    connect(ui.ChaosButton, SIGNAL(pressed()), this, SLOT(chaosPressed()));
    
    connect(ui.fpsSpinbox, SIGNAL(valueChanged(int)), m_scene, SLOT(fpsChanged(int)));
    connect(ui.CellSizeSpin, SIGNAL(valueChanged(int)), this, SLOT(cellSizeChanged(int)));
    connect(ui.ColumnsSpin, SIGNAL(valueChanged(int)), this, SLOT(colsChanged(int)));
    connect(ui.RowsSpin, SIGNAL(valueChanged(int)), this, SLOT(rowsChanged(int)));
    
    
    addShortcuts();
    
    
    if (m_scene->paused())
        setWindowTitle(QString(WINDOW_TITLE) + " (Paused)");
    
    ui.graphicsView->setScene(m_scene);
    //ui.graphicsView->setSceneRect(QRectF());
}

MainWindow::~MainWindow()
{
    saveConfig();
}


void MainWindow::aliveCells(int count)
{
    ui.AliveCellsLabel->setText(QString("Alive Cells: %1").arg(count));
}

void MainWindow::tickCount(int count)
{
    ui.EvolutionsLabel->setText(QString("Evolutions: %1").arg(count));
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
        ui.PauseButton->setText("Start");
        setWindowTitle(QString(WINDOW_TITLE) + " (Paused)");
    }
    else
    {
        ui.PauseButton->setText("Pause");
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
    QString fileName = QFileDialog::getOpenFileName(this, "Load State", 
                           m_lastDir, "Save-File (*.gol)");
    
    if (!fileName.isEmpty())
    {
        QFileInfo file(fileName);
        
        m_lastDir = file.absoluteDir().absolutePath();
        m_lastFile = file.fileName();
        
        m_scene->load(file.absoluteFilePath());
    }
}

void MainWindow::savePressed()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save State", 
                           m_lastDir, "Save-File (*.gol)");
    
    if (!fileName.isEmpty())
    {
        QFileInfo file(fileName);
        
        m_lastDir = file.absoluteDir().absolutePath();
        m_lastFile = file.fileName();
        
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

void MainWindow::reloadFilePressed()
{
    QFile file(m_lastDir+"/"+m_lastFile);
    if (file.exists())
    {
        m_scene->load(m_lastDir+"/"+m_lastFile);
    }
}


void MainWindow::cellSizeChanged(int size)
{
    size = std::max(size, 1);
    m_scene->setCellSize(size);
    m_scene->update();
    
    bool prev = ui.CellSizeSpin->blockSignals(true);
    ui.CellSizeSpin->setValue(size);
    ui.CellSizeSpin->blockSignals(prev);
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
    bool prev = ui.RowsSpin->blockSignals(true);
    ui.RowsSpin->setValue(rows);
    ui.RowsSpin->blockSignals(prev);
}

void MainWindow::sceneSetCols(int cols)
{
    bool prev = ui.ColumnsSpin->blockSignals(true);
    ui.ColumnsSpin->setValue(cols);
    ui.ColumnsSpin->blockSignals(prev);
}

void MainWindow::chaosPressed()
{
    m_scene->chaos();
}

void MainWindow::renderPressed()
{
    bool prev = m_scene->paused();
    setPaused(true);
    
    RenderDialog diag(m_scene, m_lastDir, m_lastFile, this);
    diag.exec();
    
    setPaused(prev);
}


void MainWindow::loadConfig()
{
    QFile configFile("config.json");
    if (configFile.exists() && configFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QJsonDocument doc = QJsonDocument::fromJson(configFile.readAll());
        configFile.close();
        
        QJsonObject obj = doc.object();
        
        if (obj.find("lastdir") != obj.end() && !obj["lastdir"].toString().isEmpty())
            m_lastDir = obj["lastdir"].toString();
        //if (obj.find("lastfile") != obj.end() && !obj["lastfile"].toString().isEmpty())
            //m_lastFile = obj["lastfile"].toString();
        if (obj.find("cellsize") != obj.end() && obj["cellsize"].toInt() > 0)
            ui.CellSizeSpin->setValue(obj["cellsize"].toInt());
    }
}

void MainWindow::saveConfig()
{
    QJsonObject baseObj;
    baseObj["lastdir"] = QJsonValue(m_lastDir);
    //baseObj["lastfile"] = QJsonValue(m_lastFile);
    baseObj["cellsize"] = QJsonValue(ui.CellSizeSpin->value());
    
    QJsonDocument doc(baseObj);
    
    QFile configFile("config.json");
    if (configFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&configFile);
        out << doc.toJson();
        configFile.close();
    }
}

void MainWindow::addShortcuts()
{
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
    sSave->setShortcut(QKeySequence("Ctrl+S"));
    connect(sSave, SIGNAL(triggered(bool)), this, SLOT(savePressed()));
    addAction(sSave);
    
    QAction* eLoad = new QAction(this);
    eLoad->setShortcut(QKeySequence("Ctrl+E"));
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
    
    QAction* reload = new QAction(this);
    reload->setShortcut(QKeySequence("Ctrl+L"));
    connect(reload, SIGNAL(triggered(bool)), this, SLOT(reloadFilePressed()));
    addAction(reload);
}
