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
    void Update();

    std::unique_ptr<QTimer> updateTimer;
    std::unique_ptr<QPlainTextEdit> logText;
};
