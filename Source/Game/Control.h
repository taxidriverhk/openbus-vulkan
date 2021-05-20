#pragma once

#include <list>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

#include <SFML/Window.hpp>

enum class ControlSource
{
    Keyboard,
    Mouse
};

enum class KeyCode
{
    Invalid = -1,
    KeyA = 0,
    KeyC = 2,
    KeyD = 3,
    KeyE = 4,
    KeyF = 5,
    KeyQ = 16,
    KeyS = 18,
    KeyW = 22,
    KeyLeft = 71,
    KeyRight = 72
};

enum class KeyModifier : int
{
    None = 0,
    Ctrl = 1,
    Alt = 2,
    Shift = 4
};

inline KeyModifier operator |(KeyModifier left, KeyModifier right)
{
    return static_cast<KeyModifier>(static_cast<int>(left) | static_cast<int>(right));
}

inline KeyModifier &operator |=(KeyModifier &left, KeyModifier right)
{
    return left = left | right;
}

enum class ControlCommandType
{
    Discrete,
    Continuous
};

enum class ControlCommandOperation
{
    CameraMoveLeft,
    CameraMoveRight,
    CameraMoveForward,
    CameraMoveBackward,
    CameraMoveUp,
    CameraMoveDown,
    CameraRotateCounterClockwise,
    CameraRotateClockwise,
    SwitchView,
    ToggleFrameRateDisplay
};

struct Control
{
    ControlSource source;
    KeyModifier modifier;
    KeyCode key;

    bool operator==(const Control &other) const
    {
        return this->source == other.source
            && this->modifier == other.modifier
            && this->key == other.key;
    }
};

struct ControlCommand
{
    ControlCommandType type;
    ControlCommandOperation operation;

    bool operator==(const ControlCommand &other) const
    {
        return this->type == other.type
            && this->operation == other.operation;
    }
};

namespace std
{
    template<> struct hash<Control>
    {
        size_t operator()(Control const &control) const
        {
            return (static_cast<size_t>(control.key) << 4) ^ static_cast<size_t>(control.modifier);
        }
    };

    template<> struct hash<ControlCommand>
    {
        size_t operator()(ControlCommand const &controlCommand) const
        {
            return static_cast<size_t>(controlCommand.operation);
        }
    };
}

class ControlManager
{
public:
    ControlManager();
    ~ControlManager();

    std::list<ControlCommand> PollCommands();
    void QueueEvents(std::list<sf::Event> &events);
    void RegisterControls();

private:
    std::mutex commandMutex;

    std::unordered_map<Control, ControlCommand> registeredControls;
    std::unordered_set<ControlCommand> handledCommands;
    std::unordered_set<ControlCommand> commandSequence;
};
