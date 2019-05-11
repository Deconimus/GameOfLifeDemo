#include "golscene.h"
#include "golthread.h"

#include <QPainter>
#include <QFileInfo>
#include <QFile>
#include <QDataStream>
#include <QGraphicsView>

#include <omp.h>
#include <assert.h>

#include <cstring>
#include <random>
#include <time.h>


GOLScene::GOLScene(QObject* parent)
 : QGraphicsScene(parent)
 , m_drawing(false)
 , m_paused(true)
 , m_rows(GRID_HEIGHT)
 , m_cols(GRID_WIDTH)
 , m_tickCount(0)
 , m_cellSize(CELL_SIZE)
{
    m_cells = new bool[m_cols * m_rows];
    m_buffer = new bool[m_cols * m_rows];
    
    m_fps.store(START_FPS);
    
    for (int i = 0; i < GRID_WIDTH * GRID_HEIGHT; ++i) { m_cells[i] = false; }
    
    m_thread = new GOLThread(this, this);
    m_thread->start();
}

GOLScene::~GOLScene()
{
    m_thread->setRun(false);
    m_thread->wait();
    
    delete m_thread;
    
    delete[] m_cells;
    delete[] m_buffer;
}


void GOLScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QPoint cell = sceneToCellCoords(event->scenePos());
    
    if (!inGrid(cell))
    {
        m_drawing = false;
    }
    else
    {
        m_drawing = true;
        m_lastDrawCell = cell;
        
        {
            std::lock_guard<std::mutex> guard(m_cellsMutex);
            
            m_drawKill = m_cells[cell.y() * m_cols + cell.x()];
            m_cells[cell.y() * m_cols + cell.x()] = !m_drawKill;
            
            update();
        }
    }
    
    QGraphicsScene::mousePressEvent(event);
}

void GOLScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QPoint cell = sceneToCellCoords(event->scenePos());
    
    if (m_drawing && m_lastDrawCell != cell && inGrid(cell))
    {
        std::lock_guard<std::mutex> guard(m_cellsMutex);
        
        m_cells[cell.y() * m_cols + cell.x()] = !m_drawKill;
        m_lastDrawCell = cell;
        
        update();
    }
    
    QGraphicsScene::mouseMoveEvent(event);
}

void GOLScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    //QPoint cell = sceneToCellCoords(event->scenePos());
    
    m_drawing = false;
    
    QGraphicsScene::mouseReleaseEvent(event);
}


void GOLScene::tick()
{
    std::lock_guard<std::mutex> guard(m_cellsMutex);
    
    int aliveNeighbours;
    bool alive;
    
    #pragma omp parallel for num_threads(NUM_THREADS) private(alive, aliveNeighbours)
    for (int y = 0; y < m_rows; ++y)
    {
        for (int x = 0; x < m_cols; ++x)
        {
            aliveNeighbours = 0;
            
            for (int ny = std::max(y-1, 0); ny < std::min(y+2, m_rows); ++ny)
            {
                for (int nx = std::max(x-1, 0); nx < std::min(x+2, m_cols); ++nx)
                {
                    if ((nx != x || ny != y) && m_cells[ny * m_cols + nx])
                        ++aliveNeighbours;
                }
            }
            
            alive = m_cells[y * m_cols + x];
            
            if ((alive && (aliveNeighbours == 2 || aliveNeighbours == 3)) || 
                (!alive && aliveNeighbours == 3))
                m_buffer[y * m_cols + x] = true;
            else
                m_buffer[y * m_cols + x] = false;
        }
    }
    
    bool* tmp = m_cells;
    m_cells = m_buffer;
    m_buffer = tmp;
    
    ++m_tickCount;
    emit tickCountSignal(m_tickCount);
    
    update();
}


void GOLScene::drawBackground(QPainter* painter, const QRectF& rect)
{
    assert(views().size() > 0);
    
    qreal width = m_cols * m_cellSize;
    qreal height = m_rows * m_cellSize;
    
    qreal startX = -width / 2.0;
    qreal startY = -height / 2.0;
    
    QGraphicsView* view = views()[0];
    QRectF visible = view->mapToScene(view->viewport()->geometry()).boundingRect();
    
    int startRow = 0, endRow = m_rows-1, startCol = 0, endCol = m_cols-1;
    
    if (width > visible.width())
    {
        int w = (int)std::ceil(visible.width() / m_cellSize);
        startCol = (int)std::floor((m_cols - w) / 2.0);
        endCol = startCol + w + 1;
    }
    if (height > visible.height())
    {
        int h = (int)std::ceil(visible.height() / m_cellSize);
        startRow = (int)std::floor((m_rows - h) / 2.0);
        endRow = startRow + h + 1;
    }
    
    //printf("%d - %d; %d - %d\n", startCol, endCol, startRow, endRow);
    //fflush(stdout);
    
    painter->setPen(QPen(QColor(255, 165, 0)));
    
    int cellCounter = 0;
    
    {
        std::lock_guard<std::mutex> guard(m_cellsMutex);
        
        for (int i = startRow; i <= endRow; ++i)
        {
            for (int j = startCol; j <= endCol; ++j)
            {
                if (m_cells[i * m_cols + j])
                {
                    ++cellCounter;
                    
                    painter->fillRect(QRectF(startX + j * m_cellSize, 
                                      startY + i * m_cellSize, m_cellSize, m_cellSize), 
                                      QBrush(QColor(255, 165, 0)));
                }
            }
        }
    }
    
    emit aliveCellsSignal(cellCounter);
    
    painter->setPen(QPen(Qt::darkGray));
    
    for (int i = startCol; i <= endCol+1; ++i)
    {
        qreal x = i * m_cellSize + startX;
        painter->drawLine(x, startY, x, startY + m_rows * m_cellSize);
    }
    
    for (int i = startRow; i <= endRow+1; ++i)
    {
        qreal y = i * m_cellSize + startY;
        painter->drawLine(startX, y, startX + m_cols * m_cellSize, y);
    }
}


void GOLScene::reset()
{
    std::lock_guard<std::mutex> guard(m_cellsMutex);
    
    std::memset(m_cells, false, sizeof(bool) * m_rows * m_cols);
    m_tickCount = 0;
    
    emit aliveCellsSignal(0);
    emit tickCountSignal(0);
    
    update();
}

void GOLScene::save(const QString& path)
{
    std::lock_guard<std::mutex> guard(m_cellsMutex);
    
    QFile file(path);
    if (file.open(QFile::WriteOnly))
    {
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        out << m_rows << m_cols;
        
        for (int i = 0; i < m_rows * m_cols; ++i)
            out << m_cells[i];
        
        file.write(data);
        file.close();
    }
}

void GOLScene::load(const QString& path)
{
    std::lock_guard<std::mutex> guard(m_cellsMutex);
    
    QFile file(path);
    if (file.open(QFile::ReadOnly))
    {
        QByteArray data = file.readAll();
        QDataStream in(&data, QIODevice::ReadOnly);
        
        int cols, rows;
        in >> rows >> cols;
        
        if (cols != m_cols || rows != m_rows)
        {
            delete[] m_cells;
            delete[] m_buffer;
            
            m_cells = new bool[cols * rows];
            m_buffer = new bool[cols * rows];
            
            m_rows = rows;
            m_cols = cols;
        }
        
        for (int i = 0; i < cols * rows; ++i)
            in >> m_cells[i];
        
        m_tickCount = 0;
        emit tickCountSignal(0);
        
        emit pauseSignal(true);
        
        emit rowsSignal(m_rows);
        emit colsSignal(m_cols);
        
        update();
        
        file.close();
    }
}


void GOLScene::setSize(int cols, int rows)
{
    std::lock_guard<std::mutex> guard(m_cellsMutex);
    
    if (cols == m_cols && rows == m_rows) { return; }
    
    bool* ncells = new bool[cols * rows];
    bool* nbuffer = new bool[cols * rows];
    
    std::memset(ncells, false, sizeof(bool) * rows * cols);
    
    if (cols == m_cols && rows != m_rows)
    {
        std::memcpy(ncells, m_cells, sizeof(bool) * cols * std::min(rows, m_rows));
    }
    else
    {
        #pragma omp parallel for num_threads(NUM_THREADS)
        for (int y = 0; y < std::min(m_rows, rows); ++y)
        {
            for (int x = 0; x < std::min(m_cols, cols); ++x)
            {
                ncells[y * cols + x] = m_cells[y * m_cols + x];
            }
        }
    }
    
    delete[] m_cells;
    delete[] m_buffer;
    m_cells = ncells;
    m_buffer = nbuffer;
    
    m_cols = cols;
    m_rows = rows;
    
    update();
}


void GOLScene::chaos()
{
    std::lock_guard<std::mutex> guard(m_cellsMutex);
    
    //std::random_device dev;
    std::mt19937 rng(time(0));
    std::normal_distribution<float> dist(0.0, 1.0);
    
    for (int i = 0; i < m_cols * m_rows; ++i)
        m_cells[i] = dist(rng) > 0.5;
    
    update();
}


void GOLScene::fpsChanged(int fps)
{
    m_fps.store(fps);
}

void GOLScene::pauseChanged(bool pause)
{
    m_paused.store(pause);
}


QPoint GOLScene::sceneToCellCoords(const QPointF& scenepos)
{
    QPoint cell;
    
    qreal width = m_cols * m_cellSize;
    qreal height = m_rows * m_cellSize;
    
    qreal startX = -width / 2.0;
    qreal startY = -height / 2.0;
    
    cell.setX((scenepos.x() - startX) / m_cellSize);
    cell.setY((scenepos.y() - startY) / m_cellSize);
    
    return cell;
}

bool GOLScene::inGrid(const QPoint& cell)
{
    return !(cell.x() < 0 || cell.x() >= m_cols || cell.y() < 0 || cell.y() >= m_rows);
}


bool* GOLScene::copyCells()
{
    std::lock_guard<std::mutex> guard(m_cellsMutex);
    
    bool* ncells = new bool[m_cols * m_rows];
    std::memcpy(ncells, m_cells, sizeof(bool) * m_cols * m_rows);
    return ncells;
}

void GOLScene::setCells(bool* cells, int cols, int rows)
{
    std::lock_guard<std::mutex> guard(m_cellsMutex);
    
    delete[] m_cells;
    m_cells = cells;
    
    if (cols != m_cols || rows != m_rows)
    {
        delete[] m_buffer;
        m_buffer = new bool[cols * rows];
        
        m_cols = cols;
        m_rows = rows;
    }
}
