#ifndef RENDERDIALOG_H
#define RENDERDIALOG_H


#include "ui_renderdialog.h";

#include <QObject>
#include <QDialog>


class GOLScene;


class RenderDialog : public QDialog
{
    Q_OBJECT
    
public:
    
    RenderDialog(GOLScene* scene, QWidget* parent = NULL);
    virtual ~RenderDialog();
    
    
private slots:
    
    void renderPressed();
    
    void dirPressed();
    void cellColorPickPressed();
    void bgColorPickPressed();
    
    
private:
    
    // Attributes:
    
    Ui::RenderDialog ui;
    
    GOLScene* m_scene;
    
    // Methods:
    
    void showWarningDialog(const QString& warning);
    
    bool validFileName(const QString& str);
    
};

#endif // RENDERDIALOG_H
