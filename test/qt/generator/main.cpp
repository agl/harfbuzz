/*******************************************************************
 *
 *  Copyright 2006  Trolltech ASA
 *
 *  This is part of HarfBuzz, an OpenType Layout engine library.
 *
 *  See the file name COPYING for licensing information.
 *
 ******************************************************************/

#include <QApplication>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QPushButton>
#include <QFontDatabase>
#include <QCompleter>
#include <QDirModel>
#include "ui_generator.h"

#define private public
#include <qfont.h>
#include <private/qtextengine_p.h>
#include <private/qfontengine_p.h>
#include <qtextlayout.h>
#undef private

class Generator : public QWidget, public Ui_Generator
{
    Q_OBJECT
public:
    Generator();
public slots:
    void on_chooseFontFile_clicked();
    void on_fontPath_editingFinished();
    void on_sample_editingFinished();

private:
    int fontId;
};

Generator::Generator()
{
    setupUi(this);

    QCompleter *completer = new QCompleter(fontPath);
    QDirModel *dirModel = new QDirModel(completer);
    dirModel->setFilter(QDir::AllEntries | QDir::Hidden);
    completer->setModel(dirModel);
    fontPath->setCompleter(completer);

    fontId = -1;
}

void Generator::on_chooseFontFile_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, /*caption*/ "Select TrueType Font",
                                                /*dir*/ QString(),
                                                /*filter*/ "*.ttf");
    if (path.isEmpty())
        return;
    fontPath->setText(path);
    on_fontPath_editingFinished();
}

void Generator::on_fontPath_editingFinished()
{
    const QString path = fontPath->text();
    if (!QFile::exists(path)) {
        fontPath->setStyleSheet("background-color: red");
        return;
    }

    if (fontId != -1)
        QFontDatabase::removeApplicationFont(fontId);

    fontId = QFontDatabase::addApplicationFont(path);
    if (fontId == -1 || QFontDatabase::applicationFontFamilies(fontId).isEmpty()) {
        fontPath->setStyleSheet("background-color: red");
        return;
    } else {
        fontPath->setStyleSheet(QString());
    }
    on_sample_editingFinished();
}

void Generator::on_sample_editingFinished()
{
    if (sample->text().isEmpty()) {
        glyphOutput->setPlainText("Error: No sample text");
        return;
    }
    if (fontId == -1) {
        glyphOutput->setPlainText("Error: No valid font selected");
        return;
    }
    glyphOutput->clear();

    QFont font;
    font.setFamily(QFontDatabase::applicationFontFamilies(fontId).first());
    font.setPixelSize(12);
    font.setStyleStrategy(QFont::NoFontMerging);
    font.setKerning(false);

    const QString str = sample->text();
    QTextLayout layout(str, font);
    QTextEngine *e = layout.d;
    e->itemize();
    e->shape(0);

    QString result;
    result = "# Using font '" + e->fontEngine(e->layoutData->items[0])->fontDef.family + "'\n";

    result += "# Text: ";
    for (int i = 0; i < str.length(); ++i) 
        result += "0x" + QString::number(str.at(i).unicode(), 16) + " ";
    result += "\n# Glyphs [glyph] [x advance] [y advance] [x offset] [y offset]\n";
    for (int i = 0; i < e->layoutData->items[0].num_glyphs; ++i) {
        QGlyphLayout *g = e->layoutData->glyphPtr + i;
        result += "0x" + QString::number(g->glyph, 16);
        result += ' ';
        result += QString::number(g->advance.x.toReal());
        result += ' ';
        result += QString::number(g->advance.y.toReal());
        result += ' ';
        result += QString::number(g->offset.x.toReal());
        result += ' ';
        result += QString::number(g->offset.y.toReal());
        result += '\n';
    }

    glyphOutput->setPlainText(result);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    Generator w;
    w.show();

    return app.exec();
}

#include "main.moc"
