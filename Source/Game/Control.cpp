#include "Control.h"

ControlManager::ControlManager()
{
    RegisterControls();
}

ControlManager::~ControlManager()
{
}

std::list<ControlCommand> ControlManager::PollCommands()
{
    commandMutex.lock();
    std::list<ControlCommand> unhandledCommands;
    for (const ControlCommand &command : commandSequence)
    {
        if (command.type == ControlCommandType::Discrete && handledCommands.count(command) != 0)
        {
            continue;
        }
        handledCommands.insert(command);
        unhandledCommands.push_back(command);
    }
    commandMutex.unlock();

    return unhandledCommands;
}

void ControlManager::QueueEvents(std::list<sf::Event> &events)
{
    for (sf::Event &inputEvent : events)
    {
        sf::Event::EventType eventType = inputEvent.type;
        if (eventType == sf::Event::KeyPressed || eventType == sf::Event::KeyReleased)
        {
            sf::Event::KeyEvent keyEvent = inputEvent.key;
            KeyCode keyCode = static_cast<KeyCode>(keyEvent.code);
            KeyModifier modifier = KeyModifier::None;
            if (keyEvent.alt)
            {
                modifier |= KeyModifier::Alt;
            }
            if (keyEvent.control)
            {
                modifier |= KeyModifier::Ctrl;
            }
            if (keyEvent.shift)
            {
                modifier |= KeyModifier::Shift;
            }

            Control control{};
            control.key = keyCode;
            control.modifier = modifier;
            control.source = ControlSource::Keyboard;

            if (registeredControls.count(control) == 0)
            {
                continue;
            }

            ControlCommand command = registeredControls[control];
            commandMutex.lock();
            if (eventType == sf::Event::KeyPressed)
            {
                commandSequence.insert(command);
            }
            else
            {
                handledCommands.erase(command);
                commandSequence.erase(command);
            }
            commandMutex.unlock();
        }
    }
}

void ControlManager::RegisterControls()
{
    registeredControls.insert(
        {
            {
                Control{ ControlSource::Keyboard, KeyModifier::None, KeyCode::KeyA },
                ControlCommand{ ControlCommandType::Continuous, ControlCommandOperation::CameraMoveLeft }
            },
            {
                Control{ ControlSource::Keyboard, KeyModifier::None, KeyCode::KeyD },
                ControlCommand{ ControlCommandType::Continuous, ControlCommandOperation::CameraMoveRight }
            },
            {
                Control{ ControlSource::Keyboard, KeyModifier::None, KeyCode::KeyW },
                ControlCommand{ ControlCommandType::Continuous, ControlCommandOperation::CameraMoveForward }
            },
            {
                Control{ ControlSource::Keyboard, KeyModifier::None, KeyCode::KeyS },
                ControlCommand{ ControlCommandType::Continuous, ControlCommandOperation::CameraMoveBackward }
            },
            {
                Control{ ControlSource::Keyboard, KeyModifier::None, KeyCode::KeyLeft },
                ControlCommand{ ControlCommandType::Continuous, ControlCommandOperation::CameraRotateCounterClockwise }
            },
            {
                Control{ ControlSource::Keyboard, KeyModifier::None, KeyCode::KeyRight },
                ControlCommand{ ControlCommandType::Continuous, ControlCommandOperation::CameraRotateClockwise }
            },
            {
                Control{ ControlSource::Keyboard, KeyModifier::None, KeyCode::KeyQ },
                ControlCommand{ ControlCommandType::Continuous, ControlCommandOperation::CameraMoveUp }
            },
            {
                Control{ ControlSource::Keyboard, KeyModifier::None, KeyCode::KeyE },
                ControlCommand{ ControlCommandType::Continuous, ControlCommandOperation::CameraMoveDown }
            },
            {
                Control{ ControlSource::Keyboard, KeyModifier::None, KeyCode::KeyF },
                ControlCommand{ ControlCommandType::Discrete, ControlCommandOperation::ToggleFrameRateDisplay }
            }
        }
    );
}
