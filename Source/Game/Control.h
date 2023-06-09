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
    KeyRight = 72,
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

enum class MouseAction
{
    MouseLeftButton = 1,
    MouseRightButton = 2,
    MouseMiddleButton = 4,
    MouseWheelScrollUp = 5,
    MouseWheelScrollDown = 6,
    MouseMove = 7
};

enum class MouseMovementModifier : int
{
    None = 0,
    MouseLeftButton = 1,
    MouseRightButton = 2,
    MouseMiddleButton = 4
};

inline MouseMovementModifier operator |(MouseMovementModifier left, MouseMovementModifier right)
{
    return static_cast<MouseMovementModifier>(static_cast<int>(left) | static_cast<int>(right));
}

inline MouseMovementModifier &operator |=(MouseMovementModifier &left, MouseMovementModifier right)
{
    return left = left | right;
}

struct MousePosition
{
    int x;
    int y;
};

struct MouseMovement
{
    // Negative delta menas the mouse scrolled down
    // Positive means the mouse scrolled up
    float wheelDelta;
    // Negative delta means the mouse moved to the left/down
    // Positive means the mouse moved to the right/up
    int deltaX;
    int deltaY;
};

enum class ControlCommandType
{
    Discrete,
    Continuous,
    NonHoldable
};

enum class ControlCommandOperation
{
    CameraControlStart, // Used for comparison only
    CameraMoveLeft,
    CameraMoveRight,
    CameraMoveForward,
    CameraMoveBackward,
    CameraAngleChange,
    CameraRotateCounterClockwise,
    CameraRotateClockwise,
    CameraZoomIn,
    CameraZoomOut,
    CameraControlEnd,

    SwitchView,
    ToggleFrameRateDisplay,

    UserObjectControlStart,
    VehicleAccelerate,
    VehicleBrake,
    VehicleSteerLeft,
    VehicleSteerRight,
    UserObjectControlEnd
};

struct Control
{
    ControlSource source;

    KeyModifier modifier;
    KeyCode key;

    MouseAction mouse;
    MouseMovementModifier mouseModifier;
    MouseMovement movement;

    bool operator==(const Control &other) const
    {
        return this->source == other.source
            && ((this->source == ControlSource::Keyboard
                && this->modifier == other.modifier
                && this->key == other.key)
            || (this->source == ControlSource::Mouse
                && this->mouse == other.mouse
                && this->mouseModifier == other.mouseModifier));
    }
};

struct ControlCommand
{
    ControlCommandType type;
    ControlCommandOperation operation;
    MouseMovement movement;

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

class ControlCommandName
{
public:
    static constexpr char *CAMERA_MOVE_LEFT = "CAMERA_MOVE_LEFT";
    static constexpr char *CAMERA_MOVE_RIGHT = "CAMERA_MOVE_RIGHT";
    static constexpr char *CAMERA_MOVE_FORWARD = "CAMERA_MOVE_FORWARD";
    static constexpr char *CAMERA_MOVE_BACKWARD = "CAMERA_MOVE_BACKWARD";
    static constexpr char *CAMERA_ROTATE_COUNTER_CLOCKWISE = "CAMERA_ROTATE_COUNTER_CLOCKWISE";
    static constexpr char *CAMERA_ROTATE_CLOCKWISE = "CAMERA_ROTATE_CLOCKWISE";
    static constexpr char *CAMERA_ZOOM_IN = "CAMERA_ZOOM_IN";
    static constexpr char *CAMERA_ZOOM_OUT = "CAMERA_ZOOM_OUT";
    static constexpr char *CAMERA_ANGLE_CHANGE = "CAMERA_ANGLE_CHANGE";
    
    static constexpr char *SWITCH_VIEW = "SWITCH_VIEW";
    static constexpr char *TOGGLE_FRAMERATE_DISPLAY = "TOGGLE_FRAMERATE_DISPLAY";
    
    static constexpr char *VEHICLE_ACCELERATE = "VEHICLE_ACCELERATE";
    static constexpr char *VEHICLE_BRAKE = "VEHICLE_BRAKE";
    static constexpr char *VEHICLE_STEER_LEFT = "VEHICLE_STEER_LEFT";
    static constexpr char *VEHICLE_STEER_RIGHT = "VEHICLE_STEER_RIGHT";
};

class ControlManager
{
public:
    ControlManager();
    ~ControlManager();

    std::list<ControlCommand> PollCommands();
    void QueueEvents(std::list<sf::Event> &events);
    void RegisterControls();

private:
    MouseMovementModifier currentMouseModifier;
    MousePosition currentPosition;

    std::mutex commandMutex;

    std::unordered_map<char *, ControlCommand> registeredCommands;
    std::unordered_map<Control, std::vector<ControlCommand>> registeredControls;
    std::unordered_set<ControlCommand> handledCommands;
    std::unordered_set<ControlCommand> commandSequence;
};
