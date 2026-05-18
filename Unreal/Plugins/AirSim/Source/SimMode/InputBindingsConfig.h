#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"

/**
 * Input Bindings Configuration for AirSim Drone Control
 * Defines all keyboard and joystick bindings for drone piloting
 */

namespace AirSimInputBindings
{
    // Drone Control Input Actions
    enum class EDroneInputAction : uint8
    {
        Throttle,           // Vertical movement (Z axis)
        Pitch,              // Forward/Backward (Y axis)
        Roll,               // Left/Right (X axis)
        Yaw,                // Rotation (Rotation around Z)
        EnableManualControl, // Toggle manual control mode
        ArmDisarm,          // Arm/Disarm drone
        SwitchFlightMode,   // Switch between flight modes
        ResetAll,           // Reset simulation
        ToggleRecording,    // Start/Stop recording
        ToggleCamera,       // Switch camera view
        Emergency           // Emergency landing
    };

    // Default Keyboard Bindings
    struct FKeyboardBindings
    {
        // Throttle Controls
        static const FKey ThrottleUp;      // Space
        static const FKey ThrottleDown;    // LeftCtrl

        // Movement Controls
        static const FKey PitchForward;    // W
        static const FKey PitchBackward;   // S
        static const FKey RollLeft;        // A
        static const FKey RollRight;       // D

        // Rotation Controls
        static const FKey YawLeft;         // Q
        static const FKey YawRight;        // E

        // Mode Controls
        static const FKey EnableControl;   // LeftShift
        static const FKey ArmDisarm;       // R
        static const FKey SwitchMode;      // M
        static const FKey Emergency;       // X

        // UI Controls
        static const FKey ResetAll;        // BackSpace
        static const FKey ToggleRecording; // P
        static const FKey ToggleCamera;    // C
    };

    // Joystick/Gamepad Axis Mappings
    struct FGamepadAxisBindings
    {
        static constexpr float DeadZone = 0.1f;
        static constexpr float MaxTilt = 45.0f;      // Max pitch/roll in degrees
        static constexpr float MaxYawRate = 60.0f;   // Max yaw rate in degrees/sec
        static constexpr float MaxThrottle = 1.0f;
        static constexpr float MinThrottle = 0.0f;
    };

    // Input sensitivity settings
    struct FInputSensitivity
    {
        static constexpr float PitchSensitivity = 1.0f;
        static constexpr float RollSensitivity = 1.0f;
        static constexpr float YawSensitivity = 0.8f;
        static constexpr float ThrottleSensitivity = 0.5f;
    };
}
