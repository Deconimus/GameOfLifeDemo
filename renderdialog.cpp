#include "renderdialog.h"
#include "golscene.h"

#include <QColorDialog>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QSvgGenerator>
#include <QPainter>
#include <QFileDialog>


#include <string>


#define DEFAULT_CELL_COLOR QColor(255, 165,   0)
#define DEFAULT_BG_COLOR   QColor(255, 255, 255)


RenderDialog::RenderDialog(GOLScene* scene, const QString& lastDir, 
                           const QString& lastFile, QWidget* parent)
  : QDialog(parent)
  , m_scene(scene)
  , m_lastDir(lastDir)
{
    ui.setupUi(this);
    
    setWindowTitle("Render Scene");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    
    {
        std::lock_guard<std::mutex> guard(m_scene->_cellsMutex());
        
        ui.CellSizeSpin->setValue(m_scene->cellSize());
        ui.XSpin->setMinimum(0);
        ui.XSpin->setMaximum(m_scene->columns()-1);
        ui.YSpin->setMinimum(0);
        ui.YSpin->setMaximum(m_scene->rows()-1);
        ui.WidthSpin->setMinimum(1);
        ui.WidthSpin->setValue(m_scene->columns());
        ui.WidthSpin->setMaximum(m_scene->columns());
        ui.HeightSpin->setMinimum(1);
        ui.HeightSpin->setValue(m_scene->rows());
        ui.HeightSpin->setMaximum(m_scene->rows());
    }
    
    ui.FormatCombo->addItem("SVG");
    ui.FormatCombo->addItem("HTML");
    ui.FormatCombo->setCurrentIndex(0);
    
    loadHTMLTemplate("template.html");
    
    connect(ui.RenderButton, SIGNAL(pressed()), this, SLOT(renderPressed()));
    connect(ui.DirButton, SIGNAL(pressed()), this, SLOT(dirPressed()));
    connect(ui.CellColorButton, SIGNAL(pressed()), this, SLOT(cellColorPickPressed()));
    connect(ui.BGColorButton, SIGNAL(pressed()), this, SLOT(bgColorPickPressed()));
    connect(ui.CellColorEdit, SIGNAL(textChanged(QString)), this, SLOT(cellColorChanged(QString)));
    connect(ui.BGColorEdit, SIGNAL(textChanged(QString)), this, SLOT(bgColorChanged(QString)));
    connect(ui.XSpin, SIGNAL(valueChanged(int)), this, SLOT(xChanged(int)));
    connect(ui.YSpin, SIGNAL(valueChanged(int)), this, SLOT(yChanged(int)));
    
    ui.CellColorEdit->setText(DEFAULT_CELL_COLOR.name());
    ui.BGColorEdit->setText(DEFAULT_BG_COLOR.name());
    
    QString prefix = lastFile.left(lastFile.lastIndexOf("."));
    QString dir = lastDir + "/" + prefix + "_animation";
    dir.replace("\\", "/");
    prefix += "_frame_";
    
    ui.DirectoryLine->setText(dir);
    ui.PrefixEdit->setText(prefix);
}

RenderDialog::~RenderDialog()
{
}


void RenderDialog::renderPressed()
{
    bool success = true;
    
    const QString directory = ui.DirectoryLine->text().trimmed();
    const QString prefix = ui.PrefixEdit->text().trimmed();
    const int x = ui.XSpin->value();
    const int y = ui.YSpin->value();
    const int width = ui.WidthSpin->value();
    const int height = ui.HeightSpin->value();
    const int cellSize = ui.CellSizeSpin->value();
    const int frames = ui.FramesSpin->value();
    const QColor cellColor(ui.CellColorEdit->text().trimmed());
    const QColor bgColor(ui.BGColorEdit->text().trimmed());
    const bool showGrid = ui.ShowGridBox->isChecked();
    const QString format = ui.FormatCombo->currentText().trimmed().toLower();
    
    if (m_htmlTemplate.isEmpty())
    {
        loadHTMLTemplate("template.html");
        if (m_htmlTemplate.isEmpty())
        {
            QMessageBox::critical(this, "No HTML Template", 
                                  "HTML Template could not be loaded.");
            return;
        }
    }
    
    QString warnings;
    
    QDir dir(directory);
    
    if (!directory.isEmpty() && !dir.exists() && !dir.mkpath("."))
        warnings += "Directory is invalid.\n";
    
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
    
    this->setEnabled(false);
    
    GOLScene* renderScene = new GOLScene(this);
    renderScene->pauseChanged(true);
    renderScene->setCells(m_scene->copyCells(), m_scene->columns(), m_scene->rows());
    
    //int numFrameDigits = std::to_string(frames).length();
    
    for (int i = 0; i < frames; ++i)
    {
        QString filepath(directory + "/" + prefix + QString("%1").arg(i) + "." + format);
                   //QString("%1").arg(i, numFrameDigits, 10, QChar('0')) + ".html");
        
        QFile file(filepath);
        for (int j = 0; j < 10 && file.exists(); ++j)
        {
            if (file.remove()) { break; }
            QThread::msleep(50);
        }
        
        bool* cells = renderScene->copyCells();
        
        if (format == "html")
        {
            success &= renderToHTML(filepath, cells, 
                                    renderScene->columns(), renderScene->rows(),
                                    x, y, width, height, cellSize,
                                    cellColor, bgColor, showGrid);
        }
        else if (format == "svg")
        {
            success &= renderToSVG(filepath, cells, 
                                   renderScene->columns(), renderScene->rows(),
                                   x, y, width, height, cellSize,
                                   cellColor, bgColor, showGrid);
        }
        
        delete[] cells;
        
        if (!success)
        {
            QMessageBox::critical(this, "Rendering Error", 
                                  QString("Failed to save frame %1.").arg(i));
            break;
        }
        
        if (i < frames-1)
            renderScene->tick();
    }
    
    delete renderScene;
    
    this->setEnabled(true);
    
    if (success)
    {
        QMessageBox::information(this, "Rendering Successful", 
                                 QString("Successfully rendered %1 frame(s).").arg(frames));
    }
}


void RenderDialog::dirPressed()
{
    QString dirpath = QFileDialog::getExistingDirectory(this, "Choose folder for frames",
                                                        m_lastDir);
    
    if (dirpath.isEmpty())
        return;
    
    QDir dir(dirpath);
    m_lastDir = dir.absolutePath();
    
    ui.DirectoryLine->setText(dir.absolutePath());
    
    if (ui.PrefixEdit->text() == "NewState_frame_")
        ui.PrefixEdit->setText(m_lastDir.right(m_lastDir.size() - m_lastDir.lastIndexOf("/")-1)
                               + "_frame_");
}

void RenderDialog::cellColorPickPressed()
{
    QColor col = QColorDialog::getColor(QColor(ui.CellColorEdit->text().trimmed()), this,
                                        "Pick a Cell Color");
    if (col.isValid())
        ui.CellColorEdit->setText(col.name());
}

void RenderDialog::bgColorPickPressed()
{
    QColor col = QColorDialog::getColor(QColor(ui.BGColorEdit->text().trimmed()), this,
                                        "Pick a Background Color");
    if (col.isValid())
        ui.BGColorEdit->setText(col.name());
}

void RenderDialog::cellColorChanged(const QString& hex)
{
    QColor col(hex.trimmed());
    if (col.isValid())
    {
        QPixmap pixmap(22, 22);
        pixmap.fill(col);
        ui.CellColorLabel->setPixmap(pixmap);
    }
}

void RenderDialog::bgColorChanged(const QString& hex)
{
    QColor col(hex.trimmed());
    if (col.isValid())
    {
        QPixmap pixmap(22, 22);
        pixmap.fill(col);
        ui.BGColorLabel->setPixmap(pixmap);
    }
}

void RenderDialog::xChanged(int x)
{
    ui.WidthSpin->setMaximum(m_scene->columns() - x);
    ui.WidthSpin->setValue(std::min(m_scene->columns() - x, ui.WidthSpin->value()));
}

void RenderDialog::yChanged(int y)
{
    ui.HeightSpin->setMaximum(m_scene->rows() - y);
    ui.HeightSpin->setValue(std::min(m_scene->rows() - y, ui.HeightSpin->value()));
}


bool RenderDialog::renderToHTML(const QString& filepath,
                                const bool* cells, const int cols, const int rows, 
                                const int x, const int y, const int width, const int height, 
                                const int cellSize, const QColor& cellColor,
                                const QColor& bgColor, const bool showGrid)
{
    QString html = m_htmlTemplate;
    
    if (showGrid)
        html.replace("[borderstyle]", "border: 1px solid black;");
    else
        html.replace("[borderstyle]", "border: 0px solid transparent;");
    
    html.replace("[cellsize]", QString("%1px").arg(cellSize));
    
    html.replace("[cellcolor]", cellColor.name());
    html.replace("[bgcolor]", bgColor.name());
    
    
    QString cellTable;
    
    for (int r = y; r < std::min(y + height, rows); ++r)
    {
        cellTable += "\t<tr>\n";
        
        for (int c = x; c < std::min(x + width, cols); ++c)
        {
            if (cells[r * cols + c])
                cellTable += "\t\t<td class=filled></td>\n";
            else
                cellTable += "\t\t<td></td>\n";
        }
        
        cellTable += "\t</tr>\n";
    }
    
    html.replace("[celltable]", cellTable);
    
    QFile file(filepath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        out << html;
        file.close();
    }
    else
        return false;
    
    return true;
}

bool RenderDialog::renderToSVG(const QString& filepath,
                               const bool* cells, const int cols, const int rows, 
                               const int x, const int y, const int width, const int height, 
                               const int cellSize, const QColor& cellColor,
                               const QColor& bgColor, const bool showGrid)
{
    QFile file(filepath);
    if (file.open(QIODevice::WriteOnly))
    {
        QRect viewRect(0, 0, width * cellSize, height * cellSize);
        
        QSvgGenerator generator;
        generator.setOutputDevice(&file);
        generator.setSize(QSize(width * cellSize, height * cellSize));
        generator.setViewBox(viewRect);
        generator.setDescription("Generated by Pascal Sielski's Game Of Life Demo (2019)");
        
        QPainter painter;
        painter.begin(&generator);
        
        painter.fillRect(viewRect, bgColor);
        
        for (int r = y; r < std::min(y + height, rows); ++r)
        {
            for (int c = x; c < std::min(x + width, cols); ++c)
            {
                if (cells[r * cols + c])
                    painter.fillRect(QRect((c-x) * cellSize, (r-y) * cellSize,
                                           cellSize, cellSize), cellColor);
            }
        }
        
        if (showGrid)
        {
            QPen pen(Qt::black, 1);
            painter.setPen(pen);
            
            for (int i = 0; i < width; ++i)
                painter.drawLine(i * cellSize, 0, i * cellSize, height * cellSize);
            painter.drawLine(width * cellSize - 1, 0, width * cellSize - 1, height * cellSize);
            
            for (int i = 0; i < height; ++i)
                painter.drawLine(0, i * cellSize, width * cellSize, i * cellSize);
            painter.drawLine(0, height * cellSize, width * cellSize, height * cellSize);
        }
        
        painter.end();
    }
    else
    {
        return false;
    }
    
    return true;
}


void RenderDialog::loadHTMLTemplate(const QString& filepath)
{
    QFile file(filepath);
    if (!file.exists()) { printf("HTML Template not found!\n"); fflush(stdout); }
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        m_htmlTemplate = file.readAll();
        
        file.close();
    }
}

void RenderDialog::showWarningDialog(const QString& warning)
{
    QMessageBox::warning(this, "Invalid rendering parameters", warning);
}

bool RenderDialog::validFileName(const QString& str)
{
    return !(str.contains('<') || str.contains('>') || str.contains(':') ||
             str.contains('/') || str.contains('"') || str.contains('\\') ||
             str.contains('|') || str.contains('?') || str.contains('*')) && !str.isEmpty();
}
