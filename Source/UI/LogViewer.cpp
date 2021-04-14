#include <algorithm>

#include "Common/Logger.h"
#include "LogViewer.h"

LogViewer::LogViewer()
    : QDockWidget()
{
    logText = std::make_unique<QPlainTextEdit>(this);
    logText->setReadOnly(true);
    logText->document()->setMaximumBlockCount(50);

    updateTimer = std::make_unique<QTimer>(nullptr);
    updateTimer->start(10 * 1000);
    connect(updateTimer.get(), &QTimer::timeout, this, &LogViewer::Update);

    setWidget(logText.get());
}

LogViewer::~LogViewer()
{
    updateTimer->stop();
}

void LogViewer::Update()
{
    std::wstring message = Logger::GetJoinedMessage();
    logText->appendPlainText(QString::fromStdWString(message));
}
