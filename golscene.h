#ifndef GOLSCENE_H
#define GOLSCENE_H


#define GRID_WIDTH  36
#define GRID_HEIGHT 20
#define CELL_SIZE   32
#define START_FPS   10
#define NUM_THREADS  4


#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>


class GOLThread;


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
    
    void setRows(int rows) { setSize(m_cols, rows); }
    void setColumns(int cols) { setSize(cols, m_rows); }
    void setSize(int cols, int rows);
    
    void chaos();
    
    
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
    
    
private:
    
    // Methods:
    
    QPoint sceneToCellCoords(const QPointF& scenepos);
    bool inGrid(const QPoint& cell);
    
    
    // Attributes:
    
    int m_rows, m_cols, m_cellSize;
    bool *m_cells, *m_buffer;
    
    std::atomic_bool m_paused;
    std::atomic_int m_fps;
    
    unsigned long m_tickCount;
    
    std::mutex m_cellsMutex;
    
    GOLThread* m_thread;
    
    
    bool m_drawing, m_drawKill;
    QPoint m_lastDrawCell;
    
    
};

#endif // GOLSCENE_H
