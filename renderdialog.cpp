#include "renderdialog.h"
#include "golscene.h"

#define DEFAULT_CELL_COLOR QColor(255, 165,   0)
#define DEFAULT_BG_COLOR   QColor(255, 255, 255)


RenderDialog::RenderDialog(GOLScene* scene, QWidget* parent)
  : QDialog(parent)
  , m_scene(scene)
{
    ui.setupUi(this);
    
    connect(ui.RenderButton, SIGNAL(pressed()), this, SLOT(renderPressed()));
    connect(ui.DirButton, SIGNAL(pressed()), this, SLOT(dirPressed()));
    connect(ui.CellColorButton, SIGNAL(pressed()), this, SLOT(cellColorPickPressed()));
    connect(ui.BGColorButton, SIGNAL(pressed()), this, SLOT(bgColorPickPressed()));
}

RenderDialog::~RenderDialog()
{
}


void RenderDialog::renderPressed()
{
    QString directory = ui.DirectoryLine->text().trimmed();
    QString prefix = ui.PrefixEdit->text().trimmed();
    int x = ui.XSpin->value();
    int y = ui.YSpin->value();
    int width = ui.WidthSpin->value();
    int height = ui.HeightSpin->value();
    int cellSize = ui.CellSizeSpin->value();
    int frames = ui.FramesSpin->value();
    QColor cellColor(ui.CellColorEdit->text().trimmed());
    QColor bgColor(ui.BGColorEdit->text().trimmed());
    bool showGrid = ui.ShowGridBox->isChecked();
    
    QString warnings;
    
    if (!validFileName(prefix))
        warnings += "Prefix contains filesystem illegal characters.\n";
    if (!cellColor.isValid())
        warnings += "Cell Color is invalid.\n";
    if (!bgColor.isValid())
        warnings += "BG Color is invalid.\n";
    
    if (!warnings.isEmpty())
    {
        showWarningDialog(warnings);
        return;
    }
}


void RenderDialog::dirPressed()
{
    
}

void RenderDialog::cellColorPickPressed()
{
    
}

void RenderDialog::bgColorPickPressed()
{
    
}


void RenderDialog::showWarningDialog(const QString& warning)
{
    
}

bool RenderDialog::validFileName(const QString& str)
{
    return !(str.contains('<') || str.contains('>') || str.contains(':') ||
             str.contains('/') || str.contains('"') || str.contains('\\') ||
             str.contains('|') || str.contains('?') || str.contains('*'));
}
