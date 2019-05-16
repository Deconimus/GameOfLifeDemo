#ifndef INSERTDIALOG_H
#define INSERTDIALOG_H


#include "ui_insertdialog.h"

#include <QWidget>
#include <QDialog>


class GOLScene;

        
class InsertDialog : public QDialog
{
    Q_OBJECT
    
public:
    
    explicit InsertDialog(const QString& filepath, GOLScene* scene, 
                          QWidget *parent = nullptr);
    virtual ~InsertDialog();
    
    
private slots:
    
    void insertPressed();
    
    
private:
    
    Ui::InsertDialog ui;
    
    const QString& m_filepath;
    GOLScene* m_scene;
    
};


#endif // INSERTDIALOG_H
