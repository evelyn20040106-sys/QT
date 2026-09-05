#include "mainwindow.h"

#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QListView>
#include <QListWidget>
#include <QTextEdit>
#include <QDateTimeEdit>
#include <QStackedWidget>
#include <QPushButton>
#include <QSplitter>
#include <QStatusBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QLineEdit>
#include <QTimer>
#include <QBrush>
#include <QDateTime>
#include "weatherthread.h"
#include "weathermanager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_currentMode(0),
      m_currentNoteId(-1),
      m_currentTodoId(-1),
      m_weatherThread(nullptr),
      m_weatherLabel(nullptr),
      m_timeLabel(nullptr)
{
    setWindowTitle("待办事项和笔记管理器");
    resize(900, 600);
    initMenuBar();
    initCentralWidget();
    initLeftPanel();
    initSplitter();
    initTodoPage();
    initNotePage();
    initStatusBar();
    initModels();
    initWeather();

    m_stackedWidget->setCurrentIndex(0);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateTime);
    timer->start(1000);
}

MainWindow::~MainWindow()
{
    delete m_todoModel;
    delete m_noteModel;
    if (m_weatherThread) {
        m_weatherThread->quit();      // 退出事件循环
        m_weatherThread->wait();      // 等待线程结束
    }
}

void MainWindow::initMenuBar()//菜单栏
{
    QMenuBar *menubar = new QMenuBar(this);
    setMenuBar(menubar);

    // 创建菜单动作（菜单项）
    QMenu *fileMenu = menubar->addMenu("文件");
    QAction *newTodoAction = new QAction("新建待办", this);
    QAction *newNoteAction = new QAction("新建笔记", this);
    QAction *exitAction = new QAction("退出", this);

    // 为动作设置快捷键
    newTodoAction->setShortcut(QKeySequence("Ctrl+T"));
    newNoteAction->setShortcut(QKeySequence("Ctrl+N"));
    exitAction->setShortcut(QKeySequence("Ctrl+Q"));

    fileMenu->addAction(newTodoAction);
    fileMenu->addAction(newNoteAction);
    fileMenu->addSeparator();//分割线
    fileMenu->addAction(exitAction);

    QMenu *toolMenu = menubar->addMenu("工具");
    QAction *refreshWeatherAction = new QAction("刷新天气", this);
    refreshWeatherAction->setShortcut(QKeySequence("F5"));  // 设置F5快捷键
    toolMenu->addAction(refreshWeatherAction);

    connect(newTodoAction, &QAction::triggered, this, &MainWindow::onNewTodo);
    connect(newNoteAction, &QAction::triggered, this, &MainWindow::onNewNote);
    connect(exitAction, &QAction::triggered, this, &MainWindow::onExit);

    connect(refreshWeatherAction, &QAction::triggered, [this]() {
        if (m_weatherThread) {
            m_weatherThread->requestRefresh();
            statusBar()->showMessage("正在刷新天气...", 2000);
        }
    });
}

void MainWindow:: initCentralWidget()//中央部件
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QHBoxLayout *mainLayout = new QHBoxLayout(central);
    central->setLayout(mainLayout);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
}

void MainWindow:: initLeftPanel()//左侧面板
{
    m_leftPanel = new QListWidget(centralWidget());

    m_leftPanel->addItem("待办事项");
    m_leftPanel->addItem("笔记");
    m_leftPanel->setSelectionMode(QAbstractItemView::SingleSelection);//设置可被点击选中
    m_leftPanel->setFixedWidth(120);
    m_leftPanel->setCurrentRow(0);//设置当前选中第一项
    connect(m_leftPanel, &QListWidget::currentRowChanged,this, &MainWindow::onLeftPanelCurrentRowChanged);
    centralWidget()->layout()->addWidget(m_leftPanel);
}

void MainWindow:: initSplitter()//分割器
{
    m_splitter = new QSplitter(Qt::Horizontal, centralWidget());//水平方向分割器
    m_listView = new QListView(m_splitter);
    m_listView->setMinimumWidth(200);
    // 设置列表视图的选择行为
    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);//单选
    m_listView->setEditTriggers(QAbstractItemView::NoEditTriggers);//不可直接编辑

    m_stackedWidget = new QStackedWidget(m_splitter);
    m_stackedWidget->setMinimumWidth(300);
    centralWidget()->layout()->addWidget(m_splitter);
}

void MainWindow:: initTodoPage()//代办页面
{
    m_todoPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_todoPage);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    // 创建并添加任务名称标签
    QLabel *taskNameLabel = new QLabel("任务名称：");
    QFont font = taskNameLabel->font();
    font.setBold(true);
    taskNameLabel->setFont(font);
    layout->addWidget(taskNameLabel);

    // 创建任务名称输入框
    m_taskNameEdit = new QLineEdit();
    m_taskNameEdit->setPlaceholderText("请输入任务名称...");
    layout->addWidget(m_taskNameEdit);

    // 创建并添加截止日期标签
    QLabel *dateLabel = new QLabel("截止日期：");
    dateLabel->setFont(font);
    layout->addWidget(dateLabel);

    // 创建日期时间选择器
    m_dateTimeEdit = new QDateTimeEdit();
    // 设置当前日期时间为默认值
    m_dateTimeEdit->setDateTime(QDateTime::currentDateTime());
    // 允许弹出日历选择
    m_dateTimeEdit->setCalendarPopup(true);
    // 设置日期时间显示格式
    m_dateTimeEdit->setDisplayFormat("yyyy-MM-dd hh:mm");
    layout->addWidget(m_dateTimeEdit);

    // 创建保存按钮
    m_saveTaskBtn = new QPushButton("保存任务");
    m_saveTaskBtn->setProperty("class", "primary-button");
    layout->addWidget(m_saveTaskBtn);
    layout->addStretch(1);
    connect(m_saveTaskBtn, &QPushButton::clicked,this, &MainWindow::onSaveTaskClicked);
    m_stackedWidget->addWidget(m_todoPage);
}

void MainWindow:: initNotePage()//笔记页面
{
    m_notePage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_notePage);
    layout->setContentsMargins(10, 10, 10, 10);
    m_noteEditor = new QTextEdit();
    m_noteEditor->setAcceptRichText(true);
    m_noteEditor->setMinimumHeight(400);
    m_noteEditor->setPlaceholderText("在这里输入笔记内容...");
    layout->addWidget(m_noteEditor);
    connect(m_noteEditor, &QTextEdit::textChanged,this, &MainWindow::onNoteTextChanged);
    m_stackedWidget->addWidget(m_notePage);
}

void MainWindow:: initStatusBar()//状态栏
{
    QStatusBar *statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    // 显示临时信息（显示3秒后自动消失）
    statusBar->showMessage("欢迎使用 SmartNote", 3000);
    QLabel *versionLabel = new QLabel("版本 1.0.0", this);
    statusBar->addPermanentWidget(versionLabel);
    QLabel *timeLabel = new QLabel(
    QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm"), this);
    statusBar->addPermanentWidget(timeLabel);

    m_timeLabel = new QLabel(this);
    updateTime();  // 设置初始时间
    statusBar->addPermanentWidget(m_timeLabel);
}

void MainWindow:: initModels()//数据模型
{
    if (!DatabaseManager::instance()->initialize()) {
        QMessageBox::critical(this, "错误", "数据库初始化失败！");
        return;
    }

    // 创建待办模型
    m_todoModel = new QStandardItemModel(this);
    m_todoModel->setColumnCount(1);
    m_todoModel->setHeaderData(0, Qt::Horizontal, "待办事项");

    // 创建笔记模型
    m_noteModel = new QStandardItemModel(this);
    m_noteModel->setColumnCount(1);
    m_noteModel->setHeaderData(0, Qt::Horizontal, "笔记");

    // 从数据库加载数据
    loadTodosFromDatabase();
    loadNotesFromDatabase();

    // 设置列表视图的模型
    m_listView->setModel(m_todoModel);

    // 连接列表点击信号
    connect(m_listView, &QListView::clicked, this, &MainWindow::onTodoListClicked);
}

void MainWindow::onLeftPanelCurrentRowChanged(int currentRow)
{
    m_currentMode = currentRow;
    if (currentRow == 0)
    {
        m_listView->setModel(m_todoModel);
        m_stackedWidget->setCurrentIndex(0);
        disconnect(m_listView, &QListView::clicked, nullptr, nullptr);
        connect(m_listView, &QListView::clicked,this, &MainWindow::onTodoListClicked);
        setWindowTitle("SmartNote - 待办事项");
    }
    else
    {
        m_listView->setModel(m_noteModel);
        m_stackedWidget->setCurrentIndex(1);
        disconnect(m_listView, &QListView::clicked, nullptr, nullptr);
        connect(m_listView, &QListView::clicked,this, &MainWindow::onNoteListClicked);
        setWindowTitle("SmartNote - 笔记");
    }
}

void MainWindow::onSaveTaskClicked()
{
    QString taskName = m_taskNameEdit->text().trimmed();
    if (taskName.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入任务名称！");
        return;
    }

    if (m_currentTodoId == -1) {
        // 新建待办 - 保存到数据库
        if (DatabaseManager::instance()->addTodo(taskName, m_dateTimeEdit->dateTime())) {
            loadTodosFromDatabase();  // 重新加载列表
            statusBar()->showMessage("新建待办已保存", 2000);
        } else {
            QMessageBox::critical(this, "错误", "保存失败！");
        }
    } else {
        // 获取数据库ID（存储在UserRole+2中）
        QStandardItem *item = m_todoModel->item(m_currentTodoId, 0);
        if (item) {
            int dbId = item->data(Qt::UserRole + 2).toInt();
            // 更新数据库
            if (DatabaseManager::instance()->updateTodo(dbId, taskName, m_dateTimeEdit->dateTime())) {
                loadTodosFromDatabase();  // 重新加载列表
                statusBar()->showMessage("待办已更新", 2000);
            } else {
                QMessageBox::critical(this, "错误", "更新失败！");
            }
        }
    }

    // 清空输入
    m_taskNameEdit->clear();
    m_dateTimeEdit->setDateTime(QDateTime::currentDateTime());
    m_currentTodoId = -1;
}


void MainWindow::onTodoListClicked(const QModelIndex &index)
{
    m_currentTodoId = index.row();

    QStandardItem *item = m_todoModel->item(m_currentTodoId, 0);
    if (item)
    {
        m_taskNameEdit->setText(item->text());
        QDateTime dateTime = item->data(Qt::UserRole + 1).toDateTime();
        if (dateTime.isValid())
        {
            m_dateTimeEdit->setDateTime(dateTime);
        }
    }
}

void MainWindow::onNoteListClicked(const QModelIndex &index)
{
    m_currentNoteId = index.row();

    QStandardItem *item = m_noteModel->item(m_currentNoteId, 0);
    if (item)
    {
        int dbId = item->data(Qt::UserRole + 1).toInt();
        QString content = DatabaseManager::instance()->getNoteContent(dbId);
        m_noteEditor->setPlainText(content);
    }
}

void MainWindow::onNoteTextChanged()
{
    if (m_currentNoteId >= 0) {
        // 使用定时器延迟保存，避免频繁保存
        static QTimer *saveTimer = nullptr;
        if (!saveTimer) {
            saveTimer = new QTimer(this);
            saveTimer->setSingleShot(true);
            saveTimer->setInterval(1000);  // 1秒后保存

            connect(saveTimer, &QTimer::timeout, [this]() {
                QStandardItem *item = m_noteModel->item(m_currentNoteId, 0);
                if (item) {
                    int dbId = item->data(Qt::UserRole + 1).toInt();
                    QString title = item->text();

                    if (DatabaseManager::instance()->updateNote(dbId, title, m_noteEditor->toPlainText())) {
                        statusBar()->showMessage("笔记已自动保存", 1000);
                    }
                }
            });
        }

        saveTimer->start();  // 重启定时器
    }
}

void MainWindow::onNewTodo()
{
    m_currentTodoId = -1;
    m_taskNameEdit->clear();
    m_dateTimeEdit->setDateTime(QDateTime::currentDateTime());
    m_leftPanel->setCurrentRow(0);
    m_taskNameEdit->setFocus();
}

void MainWindow::onNewNote()
{
    QString title = "新笔记 " + QDateTime::currentDateTime().toString("MM-dd hh:mm");

    if (DatabaseManager::instance()->addNote(title, "")) {
        loadNotesFromDatabase();  // 重新加载列表

        // 选中新创建的笔记
        m_currentNoteId = m_noteModel->rowCount() - 1;
        m_noteEditor->clear();
        m_leftPanel->setCurrentRow(1);
        m_noteEditor->setFocus();

        statusBar()->showMessage("新建笔记...", 2000);
    }
}

void MainWindow::onExit()
{
    close();
}

void MainWindow::populateTodoModel(const QList<TodoItem> &todos)
{
    for (const TodoItem &todo : todos) {
        QStandardItem *item = new QStandardItem(todo.title);
        item->setCheckable(true);
        item->setCheckState(todo.completed ? Qt::Checked : Qt::Unchecked);
        item->setData(todo.deadline, Qt::UserRole + 1);
        item->setData(todo.id, Qt::UserRole + 2);

        // 如果已过期且未完成，用红色显示
        if (!todo.completed && todo.deadline < QDateTime::currentDateTime()) {
            item->setForeground(QBrush(Qt::red));
        }

        m_todoModel->appendRow(item);
    }
}
void MainWindow::loadTodosFromDatabase() {
    m_todoModel->clear();
    m_todoModel->setColumnCount(1);
    m_todoModel->setHeaderData(0, Qt::Horizontal, "待办事项");

    QList<TodoItem> todos = DatabaseManager::instance()->getAllTodos();

    populateTodoModel(todos);
}

// 新增：从数据库加载笔记
void MainWindow::loadNotesFromDatabase()
{
    m_noteModel->clear();
    m_noteModel->setColumnCount(1);
    m_noteModel->setHeaderData(0, Qt::Horizontal, "笔记");

    QList<NoteItem> notes = DatabaseManager::instance()->getAllNotes();

    for (auto it = notes.constBegin(); it != notes.constEnd(); ++it) {
        const NoteItem &note = *it;
        QStandardItem *item = new QStandardItem(note.title);
        item->setData(note.id, Qt::UserRole + 1);
        item->setData(note.updatedAt, Qt::UserRole + 2);
        m_noteModel->appendRow(item);
    }
}

void MainWindow::initWeather()
{
    // 创建天气标签（放在状态栏最右边）
    m_weatherLabel = new QLabel("加载天气...", this);
    statusBar()->addPermanentWidget(m_weatherLabel);

    // 创建天气线程
    m_weatherThread = new WeatherThread(this);

    // 连接信号
    connect(m_weatherThread, &WeatherThread::weatherUpdated,
            this, &MainWindow::onWeatherUpdated);
    connect(m_weatherThread, &WeatherThread::errorOccurred,
            this, &MainWindow::onWeatherError);

    // 启动线程
    m_weatherThread->start();

    qDebug() << "天气功能初始化完成";
}

void MainWindow::updateTime()
{
    if (m_timeLabel) {
        m_timeLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    }
}

void MainWindow::onWeatherUpdated(const WeatherData &weather)
{
    if (m_weatherLabel) {
        // 格式化天气显示
        QString weatherText = QString("%1 %2 %3")
                                  .arg(weather.city)
                                  .arg(weather.weather)
                                  .arg(weather.temperature);

        // 如果有风力信息，也显示
        if (!weather.wind.isEmpty()) {
            weatherText += " " + weather.wind;
        }

        m_weatherLabel->setText(weatherText);

        // 设置提示信息（鼠标悬停时显示）
        QString tooltip = QString("湿度: %1\n更新时间: %2")
                              .arg(weather.humidity)
                              .arg(weather.updateTime);
        m_weatherLabel->setToolTip(tooltip);
    }
}

void MainWindow::onWeatherError(const QString &error)
{
    if (m_weatherLabel) {
        m_weatherLabel->setText("天气获取失败");
        m_weatherLabel->setToolTip(error);
    }
}


