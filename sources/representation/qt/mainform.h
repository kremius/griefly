#ifndef MAINFORM_H
#define MAINFORM_H

#include <QWidget>
#include <QTimer>
#include <QLineEdit>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QMap>
#include <QVector>
#include <QElapsedTimer>

namespace Ui {
class MainForm;
}

class MainForm : public QWidget
{
    Q_OBJECT
public:
    explicit MainForm(QWidget *parent = 0);
    ~MainForm();
public slots:
    void addSystemText(QString key, QString text);

    void startGameLoop(int id, QString map);
    void connectionFailed(QString reason);

    void insertHtmlIntoChat(QString html);
    void playMusic(QString name, float volume);

    void oocPrefixToLineEdit();

    void uploadStarted();
    void uploadFinished();
protected:
    void closeEvent(QCloseEvent* event);
    void resizeEvent(QResizeEvent* event);
private slots:
    void setFocusOnLineEdit();

    void on_lineEdit_returnPressed();

    void on_splitter_splitterMoved(int pos, int index);

    void connectToHost();
signals:
    void closing();
    void generateUnsync();
private:
    QElapsedTimer close_request_timer_;
    bool map_sending_;

    void RemoveFirstBlockFromTextEditor();

    int left_column;
    int right_column;

    int argc_;
    char** argv_;
    Ui::MainForm *ui;

    QMap<QString, QString> texts_;

    QVector<QString> chat_messages_;
};

#endif // MAINFORM_H
