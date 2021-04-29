#pragma once

#include <memory>

#include <QtWidgets>

class LogViewer : public QDockWidget
{
    Q_OBJECT

public:
    LogViewer();
    ~LogViewer();

private:
    static constexpr char *TITLE = "Log Viewer";
    static constexpr int MAX_LINE_COUNT = 500;
    static constexpr int UPDATE_INTERVAL_MILLIS = 1 * 1000;

    void Update();

    QString logText;
    std::unique_ptr<QTimer> updateTimer;
    std::unique_ptr<QPlainTextEdit> logTextBox;
};
