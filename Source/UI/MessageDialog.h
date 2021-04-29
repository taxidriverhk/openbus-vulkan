#pragma once

#include <memory>
#include <string>
#include <QMessageBox>

enum class MessageDialogType
{
    Alert,
    Confirmation,
    Fatal,
    Information,
};

enum class MessageDialogResult
{
    Ok,
    Cancel,
};

class MessageDialog
{
public:
    MessageDialog();
    MessageDialogResult ShowMessage(MessageDialogType type, std::string message);

private:
    static constexpr int DIALOG_MIN_WIDTH = 500;
    static constexpr int DIALOG_MIN_HEIGHT = 300;

    QMessageBox messageBox;
};
