#ifndef RENDERDIALOG_H
#define RENDERDIALOG_H


#include "ui_renderdialog.h"

#include <QObject>
#include <QDialog>


class GOLScene;


class RenderDialog : public QDialog
{
    Q_OBJECT
    
public:
    
    RenderDialog(GOLScene* scene, const QString& lastDir, 
                 const QString& lastFile, QWidget* parent = NULL);
    virtual ~RenderDialog();
    
    
private slots:
    
    void renderPressed();
    
    void dirPressed();
    void cellColorPickPressed();
    void bgColorPickPressed();
    
    void cellColorChanged(const QString& hex);
    void bgColorChanged(const QString& hex);
    
    void xChanged(int x);
    void yChanged(int y);
    
    
private:
    
    // Attributes:
    
    Ui::RenderDialog ui;
    
    GOLScene* m_scene;
    
    QString m_lastDir, m_htmlTemplate;
    
    
    // Methods:
    
    bool renderToHTML(const QString& filepath,
                      const bool* cells, const int cols, const int rows, 
                      const int x, const int y, const int width, const int height, 
                      const int cellSize, const QColor& cellColor,
                      const QColor& bgColor, const bool showGrid);
    
    bool renderToSVG(const QString& filepath,
                     const bool* cells, const int cols, const int rows,
                     const int x, const int y, const int width, const int height,
                     const int cellSize, const QColor& cellColor,
                     const QColor& bgColor, const bool showGrid);
    
    void showWarningDialog(const QString& warning);
    bool validFileName(const QString& str);
    
    void loadHTMLTemplate(const QString& filepath);
    
};

#endif // RENDERDIALOG_H
