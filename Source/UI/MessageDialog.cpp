#include "MessageDialog.h"

MessageDialog::MessageDialog()
{
    messageBox.setMinimumSize(DIALOG_MIN_WIDTH, DIALOG_MIN_HEIGHT);
}

MessageDialogResult MessageDialog::ShowMessage(
    MessageDialogType type,
    const std::string &message,
    const std::string &details)
{
    // TODO: support more types of messages
    if (type == MessageDialogType::Alert)
    {
        messageBox.setIcon(QMessageBox::Icon::Warning);
        messageBox.setText(QString::fromStdString(message));
        messageBox.setStandardButtons(QMessageBox::Ok);
    }
    else if (type == MessageDialogType::Fatal)
    {
        messageBox.setIcon(QMessageBox::Icon::Critical);
        messageBox.setText(QString::fromStdString(message));
        messageBox.setDetailedText(QString::fromStdString(details));
        messageBox.setStandardButtons(QMessageBox::Close);
    }
    
    int result = messageBox.exec();
    return result == QMessageBox::Ok ? MessageDialogResult::Ok : MessageDialogResult::Cancel;
}
