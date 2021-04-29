#include "MessageDialog.h"

MessageDialog::MessageDialog()
{
    messageBox.setMinimumSize(DIALOG_MIN_WIDTH, DIALOG_MIN_HEIGHT);
}

MessageDialogResult MessageDialog::ShowMessage(MessageDialogType type, std::string message)
{
    // TODO: support more types of messages
    messageBox.setIcon(QMessageBox::Icon::Warning);
    messageBox.setText(QString::fromStdString(message));
    messageBox.setStandardButtons(QMessageBox::Ok);
    int result = messageBox.exec();

    return result == QMessageBox::Ok ? MessageDialogResult::Ok : MessageDialogResult::Cancel;
}
