#ifndef OBJECTEDITOR_H
#define OBJECTEDITOR_H

#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>

class ObjectView;

namespace Ui {
    class ObjectEditor;
}

class ObjectEditor : public QMainWindow {
    Q_OBJECT
public:
    ObjectEditor(QWidget *parent = 0);
    ~ObjectEditor();
    static ObjectEditor * instance();

    void open(QString file);

    QListWidget * artList();
    QListWidget * layersList();

    QPushButton * newLayerButton();
    QPushButton * deleteLayerButton();
    QPushButton * moveLayerUpButton();
    QPushButton * moveLayerDownButton();

    void createEmpty();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ObjectEditor *m_ui;
    static ObjectEditor * s_inst;

    ObjectView * m_view;

    void refreshArt();

private slots:
    void on_btnDeleteLayer_clicked();
    void on_btnNewLayer_clicked();
    void on_btnMoveLayerDown_clicked();
    void on_btnMoveLayerUp_clicked();
    void on_actionZoomOut_triggered();
    void on_actionZoomIn_triggered();
    void on_actionZoom100_triggered();
    void on_actionShape_triggered();
    void on_actionSurfaceType_triggered();
    void on_actionNormal_triggered();
    void on_lstLayers_currentRowChanged(int currentRow);

};

#endif // OBJECTEDITOR_H
