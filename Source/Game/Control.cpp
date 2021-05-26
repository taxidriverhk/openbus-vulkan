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
        std::vector<ControlCommand> &commands = registeredControls[control];
        commandMutex.lock();
        for (ControlCommand command : commands)
        {
            command.movement = mouseMovement;
            if (eventType != sf::Event::KeyReleased)
            {
                commandSequence.insert(command);
            }
            else
            {
                handledCommands.erase(command);
                commandSequence.erase(command);
            }
        }
        commandMutex.unlock();
    }
}

void ControlManager::RegisterControls()
{
    // Register commands that can be mapped by devices (ex. keyboard, mouse, etc.)
    // Each control command should be registered here so that the settings can map to the command correctly
    registeredCommands.insert(
        {
            {
                ControlCommandName::CAMERA_MOVE_LEFT,
                ControlCommand{ ControlCommandType::Continuous, ControlCommandOperation::CameraMoveLeft }
            },
            {
                ControlCommandName::CAMERA_MOVE_RIGHT,
                ControlCommand{ ControlCommandType::Continuous, ControlCommandOperation::CameraMoveRight }
            },
            {
                ControlCommandName::CAMERA_MOVE_FORWARD,
                ControlCommand{ ControlCommandType::Continuous, ControlCommandOperation::CameraMoveForward }
            },
            {
                ControlCommandName::CAMERA_MOVE_BACKWARD,
                ControlCommand{ ControlCommandType::Continuous, ControlCommandOperation::CameraMoveBackward }
            },
            {
                ControlCommandName::CAMERA_ROTATE_COUNTER_CLOCKWISE,
                ControlCommand{ ControlCommandType::Continuous, ControlCommandOperation::CameraRotateCounterClockwise }
            },
            {
                ControlCommandName::CAMERA_ROTATE_CLOCKWISE,
                ControlCommand{ ControlCommandType::Continuous, ControlCommandOperation::CameraRotateClockwise }
            },
            {
                ControlCommandName::SWITCH_VIEW,
                ControlCommand{ ControlCommandType::Discrete, ControlCommandOperation::SwitchView }
            },
            {
                ControlCommandName::TOGGLE_FRAMERATE_DISPLAY,
                ControlCommand{ ControlCommandType::Discrete, ControlCommandOperation::ToggleFrameRateDisplay }
            },
            {
                ControlCommandName::CAMERA_ZOOM_IN,
                ControlCommand{ ControlCommandType::NonHoldable, ControlCommandOperation::CameraZoomIn }
            },
            {
                ControlCommandName::CAMERA_ZOOM_OUT,
                ControlCommand{ ControlCommandType::NonHoldable, ControlCommandOperation::CameraZoomOut }
            },
            {
                ControlCommandName::CAMERA_ANGLE_CHANGE,
                ControlCommand{ ControlCommandType::NonHoldable, ControlCommandOperation::CameraAngleChange }
            }
        });

    // TODO: should read from configuration, hard-coding the registered keys for now
    registeredControls.insert(
        {
            {
                Control{ ControlSource::Keyboard, KeyModifier::None, KeyCode::KeyA },
                { registeredCommands[ControlCommandName::CAMERA_MOVE_LEFT] }
            },
            {
                Control{ ControlSource::Keyboard, KeyModifier::None, KeyCode::KeyD },
                { registeredCommands[ControlCommandName::CAMERA_MOVE_RIGHT] }
            },
            {
                Control{ ControlSource::Keyboard, KeyModifier::None, KeyCode::KeyW },
                { registeredCommands[ControlCommandName::CAMERA_MOVE_FORWARD] }
            },
            {
                Control{ ControlSource::Keyboard, KeyModifier::None, KeyCode::KeyS },
                { registeredCommands[ControlCommandName::CAMERA_MOVE_BACKWARD] }
            },
            {
                Control{ ControlSource::Keyboard, KeyModifier::None, KeyCode::KeyLeft },
                { registeredCommands[ControlCommandName::CAMERA_ROTATE_COUNTER_CLOCKWISE] }
            },
            {
                Control{ ControlSource::Keyboard, KeyModifier::None, KeyCode::KeyRight },
                { registeredCommands[ControlCommandName::CAMERA_ROTATE_CLOCKWISE] }
            },
            {
                Control{ ControlSource::Keyboard, KeyModifier::None, KeyCode::KeyC },
                { registeredCommands[ControlCommandName::SWITCH_VIEW] }
            },
            {
                Control{ ControlSource::Keyboard, KeyModifier::None, KeyCode::KeyF },
                { registeredCommands[ControlCommandName::TOGGLE_FRAMERATE_DISPLAY] }
            },
            {
                Control{ ControlSource::Mouse, KeyModifier::None, KeyCode::Invalid, MouseAction::MouseWheelScrollUp, MouseMovementModifier::None },
                { registeredCommands[ControlCommandName::CAMERA_ZOOM_IN] }
            },
            {
                Control{ ControlSource::Mouse, KeyModifier::None, KeyCode::Invalid, MouseAction::MouseWheelScrollDown, MouseMovementModifier::None },
                { registeredCommands[ControlCommandName::CAMERA_ZOOM_OUT] }
            },
            {
                Control{ ControlSource::Mouse, KeyModifier::None, KeyCode::Invalid, MouseAction::MouseMove, MouseMovementModifier::MouseRightButton },
                { registeredCommands[ControlCommandName::CAMERA_ANGLE_CHANGE] }
            }
        }
    );
}
