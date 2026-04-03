#pragma once

// Qt
#include <QMainWindow> // Required for inheritance
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QString>
#include <QVector>
#include <QPair>

class QVBoxLayout;
class QSyntaxStyle;
class QComboBox;
class QCheckBox;
class QSpinBox;
class QCompleter;
class QStyleSyntaxHighlighter;
class QCodeEditor;

/**
 * @brief Class, that describes demo main window.
 */
class QMareTextEditorWidget : public QWidget
{
    Q_OBJECT

public:

    /**
     * @brief Constructor.
     * @param parent Pointer to parent widget.
     */
    explicit QMareTextEditorWidget(QWidget* parent=nullptr);

    void loadStyle(QString path);

    QString loadCode(QString path);

    void initData();

    void createWidgets();

    void setupWidgets();

    void performConnections();

protected:
    QVBoxLayout* m_setupLayout;

    QComboBox* m_codeSampleCombobox;
    QComboBox* m_highlighterCombobox;
    QComboBox* m_completerCombobox;
    QComboBox* m_styleCombobox;

    QCheckBox* m_readOnlyCheckBox;
    QCheckBox* m_wordWrapCheckBox;
    QCheckBox* m_tabReplaceEnabledCheckbox;
    QSpinBox*  m_tabReplaceNumberSpinbox;
    QCheckBox* m_autoIndentationCheckbox;

    //QMenu * m_mainMenu;
    //QAction * m_actionToggleComment;
    //QAction * m_actionToggleBlockComment;

    QCodeEditor* m_codeEditor;

    QVector<QPair<QString, QString>> m_codeSamples;
    QVector<QPair<QString, QCompleter*>> m_completers;
    QVector<QPair<QString, QStyleSyntaxHighlighter*>> m_highlighters;
    QVector<QPair<QString, QSyntaxStyle*>> m_styles;
};
