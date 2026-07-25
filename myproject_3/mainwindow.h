#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <qstandarditemmodel>
#include "databasemanager.h"

class WeatherThread;
struct WeatherData;
class QListWidget;
class QListView;
class QStackedWidget;
class QTextEdit;
class QLineEdit;
class QDateTimeEdit;
class QPushButton;
class QSplitter;
class QStatusBar;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    //左侧面板切换槽函数
    void onLeftPanelCurrentRowChanged(int currentRow);

    //代办相关槽函数
    void onSaveTaskClicked();
    void onTodoListClicked(const QModelIndex &index);

    //笔记本相关槽函数
    void onNoteListClicked(const QModelIndex &index);//点击列表
    void onNoteTextChanged();//笔记本变化自动保存

    //菜单动作槽函数
    void onNewTodo();//新建代办
    void onNewNote();//新建笔记
    void onExit();//退出程序

    void onWeatherUpdated(const WeatherData &weather);  // 天气更新
    void onWeatherError(const QString &error);         // 天气错误

private:

    void initMenuBar();//菜单栏
    void initCentralWidget();//中央部件
    void initLeftPanel();//左侧面板
    void initSplitter();//分割器
    void initTodoPage();//代办页面
    void initNotePage();//笔记页面
    void initStatusBar();//状态栏
    void initModels();//数据模型
    void populateTodoModel(const QList<TodoItem> &todos);
    void loadTodosFromDatabase();    // 从数据库加载待办
    void loadNotesFromDatabase();    // 从数据库加载笔记
    void saveTodoToDatabase();       // 保存待办到数据库
    void saveNoteToDatabase();       // 保存笔记到数据库

    QListWidget *m_leftPanel;
    QSplitter *m_splitter;
    QListView *m_listView;
    QStackedWidget *m_stackedWidget;

    //代办页面空间
    QWidget *m_todoPage;
    QLineEdit *m_taskNameEdit;
    QDateTimeEdit *m_dateTimeEdit;
    QPushButton *m_saveTaskBtn;


    //笔记本页面控件
    QWidget *m_notePage;
    QTextEdit *m_noteEditor;

    //数据模型：存储列表数据的标准模型
    QStandardItemModel *m_todoModel;
    QStandardItemModel *m_noteModel;

    //状态变量
    int m_currentMode;//当前模式：0-待办模式，1-笔记模式
    int m_currentTodoId;//待办ID
    int m_currentNoteId;//笔记ID

    WeatherThread *m_weatherThread;  // 天气工作线程
    QLabel *m_weatherLabel;          // 状态栏天气标签
    QLabel *m_timeLabel;             // 状态栏时间标签

    // 新增的私有函数
    void initWeather();               // 初始化天气
    void updateTime();                // 更新时间显示

};
#endif // MAINWINDOW_H
