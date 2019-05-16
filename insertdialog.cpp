#include "insertdialog.h"
#include "golscene.h"


InsertDialog::InsertDialog(const QString& filepath, GOLScene* scene, QWidget* parent)
    : QDialog(parent)
    , m_filepath(filepath)
    , m_scene(scene)
{
    ui.setupUi(this);
    
    setWindowTitle("Insert");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    
    connect(ui.InsertButton, SIGNAL(pressed()), this, SLOT(insertPressed()));
}

InsertDialog::~InsertDialog()
{
}


void InsertDialog::insertPressed()
{
    int x = ui.XSpin->value();
    int y = ui.YSpin->value();
    int rotation = ui.RotationSpin->value();
    
    int cols, rows;
    bool* cells = GOLScene::loadFile(m_filepath, cols, rows);
    cells = GOLScene::rotateCells(cells, cols, rows, rotation);
    
    m_scene->insert(cells, x, y, cols, rows);
    
    accept();
}
