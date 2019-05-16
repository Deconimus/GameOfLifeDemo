#ifndef GOLSCENE_H
#define GOLSCENE_H

#pragma GCC diagnostic ignored "-Wreorder"


#define GRID_WIDTH  36
#define GRID_HEIGHT 20
#define CELL_SIZE   32
#define START_FPS   10
#define NUM_THREADS  4


#include <QObject>
#include <QGraphicsScene>

#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>


class GOLThread;
class QHoverEvent;
class QGraphicsMouseEvent;


class GOLScene : public QGraphicsScene
{
    Q_OBJECT
    
    friend class GOLThread;
    
public:
    
    GOLScene(QObject* parent = NULL);
    virtual ~GOLScene();
    
    
    inline bool paused() { return m_paused.load(); }
    inline int fps() { return m_fps.load(); }
    
    inline int cellSize() { return m_cellSize; }
    inline void setCellSize(int size)
    { 
        std::lock_guard<std::mutex> g(m_cellsMutex);
        m_cellSize = size;
    }
    
    void tick();
    
    void reset();
    void save(const QString& path);
    void load(const QString& path);
    
    int rows() { return m_rows; }
    void setRows(int rows) { setSize(m_cols, rows); }
    int columns() { return m_cols; }
    void setColumns(int cols) { setSize(cols, m_rows); }
    void setSize(int cols, int rows);
    
    void chaos();
    
    
    bool* copyCells();
    void setCells(bool* cells, int cols, int rows); // takes ownership of the pointer
    
    std::mutex& _cellsMutex() { return m_cellsMutex; }
    const bool* cells() { return m_cells; }
    
    
    
protected:
    
    virtual void drawBackground(QPainter* painter, const QRectF& rect) override;
    
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    
    
public slots:
    
    void fpsChanged(int fps);
    void pauseChanged(bool pause);
    
    
signals:
    
    void aliveCellsSignal(int count);
    void tickCountSignal(int count);
    void pauseSignal(bool paused);
    void rowsSignal(int rows);
    void colsSignal(int cols);
    void cursorSignal(int col, int row);
    
    
private:
    
    // Methods:
    
    QPoint sceneToCellCoords(const QPointF& scenepos);
    bool inGrid(const QPoint& cell);
    
    
    // Attributes:
    
    int m_rows, m_cols, m_cellSize;
    bool *m_cells, *m_buffer;
    
    std::atomic_bool m_paused;
    std::atomic_int m_fps;
    
    unsigned long m_tickCount, m_cellCounter;
    
    std::mutex m_cellsMutex;
    
    GOLThread* m_thread;
    
    
    bool m_drawing, m_drawKill;
    QPoint m_lastDrawCell, m_lastHoverCursor;
    
    
};

#endif // GOLSCENE_H
