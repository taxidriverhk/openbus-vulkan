#include "Control.h"

ControlManager::ControlManager()
    : currentPosition{},
      currentMouseModifier(MouseMovementModifier::None)
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
        // Do not handle the command if the command is already handled,
        // and the user has not released the control (ex. toggle FPS display)
        if (command.type == ControlCommandType::Discrete && handledCommands.count(command) != 0)
        {
            continue;
        }
        handledCommands.insert(command);
        unhandledCommands.push_back(command);
    }

    // Remove any commands from non-holdable control (ex. mouse wheel or mouse move)
    // to avoid the command being repeated
    for (auto commandIt = commandSequence.begin(); commandIt != commandSequence.end();)
    {
        if (commandIt->type == ControlCommandType::NonHoldable)
        {
            commandIt = commandSequence.erase(commandIt);
        }
        else
        {
            commandIt++;
        }
    }

    commandMutex.unlock();

    return unhandledCommands;
}

void ControlManager::QueueEvents(std::list<sf::Event> &events)
{
    for (sf::Event &inputEvent : events)
    {
        sf::Event::EventType eventType = inputEvent.type;
        // Convert the SFML event into a generic command, and then
        // look to see if the event is registered to any command
        Control control{};
        MouseMovement mouseMovement{};

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

            control.key = keyCode;
            control.modifier = modifier;
            control.source = ControlSource::Keyboard;
        }
        else if (eventType == sf::Event::MouseEntered || eventType == sf::Event::MouseLeft)
        {
            currentPosition = { 0, 0 };
            continue;
        }
        else if (eventType == sf::Event::MouseMoved)
        {
            sf::Event::MouseMoveEvent mouseMoveEvent = inputEvent.mouseMove;
            mouseMovement.deltaX = currentPosition.x - mouseMoveEvent.x;
            mouseMovement.deltaY = currentPosition.y - mouseMoveEvent.y;

            currentPosition = { mouseMoveEvent.x, mouseMoveEvent.y };

            control.key = KeyCode::Invalid;
            control.modifier = KeyModifier::None;
            control.source = ControlSource::Mouse;
            control.mouse = MouseAction::MouseMove;
            control.mouseModifier = currentMouseModifier;
            control.movement = mouseMovement;
        }
        else if (eventType == sf::Event::MouseWheelScrolled)
        {
            sf::Event::MouseWheelScrollEvent scrollEvent = inputEvent.mouseWheelScroll;

            mouseMovement.wheelDelta = scrollEvent.delta;

            control.key = KeyCode::Invalid;
            control.source = ControlSource::Mouse;
            control.mouse = mouseMovement.wheelDelta > 0 ? MouseAction::MouseWheelScrollUp : MouseAction::MouseWheelScrollDown;
            control.movement = mouseMovement;
        }
        else if (eventType == sf::Event::MouseButtonPressed || eventType == sf::Event::MouseButtonReleased)
        {
            sf::Event::MouseButtonEvent mouseButtonEvent = inputEvent.mouseButton;
            MouseAction mouseButton = static_cast<MouseAction>(mouseButtonEvent.button);

            // Update the current modifier state for mouse wheel and move
            if (eventType == sf::Event::MouseButtonPressed)
            {
                if (mouseButtonEvent.button == sf::Mouse::Button::Left)
                {
                    currentMouseModifier |= MouseMovementModifier::MouseLeftButton;
                }
                else if (mouseButtonEvent.button == sf::Mouse::Button::Right)
                {
                    currentMouseModifier |= MouseMovementModifier::MouseRightButton;
                }
                else if (mouseButtonEvent.button == sf::Mouse::Button::Middle)
                {
                    currentMouseModifier |= MouseMovementModifier::MouseMiddleButton;
                }
            }
            else
            {
                currentMouseModifier = MouseMovementModifier::None;
            }

            control.key = KeyCode::Invalid;
            control.source = ControlSource::Mouse;
            control.mouse = mouseButton;
        }

        if (registeredControls.count(control) == 0)
        {
            continue;
        }

        // Put the command into the queue for the game thread to process
        ControlCommand command = registeredControls[control];
        command.movement = mouseMovement;
        commandMutex.lock();
        if (eventType != sf::Event::KeyReleased)
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

void ControlManager::RegisterControls()
{
    // TODO: should read from configuration, hard-coding the registered keys for now
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
                Control{ ControlSource::Keyboard, KeyModifier::None, KeyCode::KeyC },
                ControlCommand{ ControlCommandType::Discrete, ControlCommandOperation::SwitchView }
            },
            {
                Control{ ControlSource::Keyboard, KeyModifier::None, KeyCode::KeyF },
                ControlCommand{ ControlCommandType::Discrete, ControlCommandOperation::ToggleFrameRateDisplay }
            },
            {
                Control{ ControlSource::Mouse, KeyModifier::None, KeyCode::Invalid, MouseAction::MouseWheelScrollUp, MouseMovementModifier::None },
                ControlCommand{ ControlCommandType::NonHoldable, ControlCommandOperation::CameraZoomIn }
            },
            {
                Control{ ControlSource::Mouse, KeyModifier::None, KeyCode::Invalid, MouseAction::MouseWheelScrollDown, MouseMovementModifier::None },
                ControlCommand{ ControlCommandType::NonHoldable, ControlCommandOperation::CameraZoomOut }
            },
            {
                Control{ ControlSource::Mouse, KeyModifier::None, KeyCode::Invalid, MouseAction::MouseMove, MouseMovementModifier::MouseRightButton },
                ControlCommand{ ControlCommandType::NonHoldable, ControlCommandOperation::CameraPitchChange }
            }
        }
    );
}
