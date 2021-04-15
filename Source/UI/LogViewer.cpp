#include "Common/Logger.h"
#include "LogViewer.h"

LogViewer::LogViewer()
    : QDockWidget(),
      logText("")
{
    logTextBox = std::make_unique<QPlainTextEdit>(this);
    logTextBox->setReadOnly(true);
    logTextBox->document()->setMaximumBlockCount(50);

    updateTimer = std::make_unique<QTimer>(nullptr);
    updateTimer->start(1 * 1000);
    connect(updateTimer.get(), &QTimer::timeout, this, &LogViewer::Update);

    setWidget(logTextBox.get());
}

LogViewer::~LogViewer()
{
    updateTimer->stop();
}

void LogViewer::Update()
{
    if (Logger::IsUpdated())
    {
        std::wstring message = Logger::GetJoinedMessage();
        logText = QString::fromStdWString(message);
        logTextBox->appendPlainText(logText);
    }
}
