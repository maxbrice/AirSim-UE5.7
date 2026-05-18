#include "DroneInputController.h"
#include "SimModeBase.h"
#include "PawnSimApi.h"
#include "AirBlueprintLib.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Controller.h"
#include "InputBindingsConfig.h"

ADroneInputController::ADroneInputController()
    : CurrentThrottle(0.0f)
    , CurrentPitch(0.0f)
    , CurrentRoll(0.0f)
    , CurrentYaw(0.0f)
    , bInputEnabled(true)
    , bRecordingActive(false)
    , bDebugInputVisible(false)
    , JoystickIndex(0)
    , InputUpdateTimer(0.0f)
    , FrameCounter(0)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.016f; // 60Hz

    // Disable rotation and physics
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void ADroneInputController::BeginPlay()
{
    Super::BeginPlay();

    // Get reference to SimMode
    if (!SimMode)
    {
        SimMode = ASimModeBase::getSimMode();
    }

    if (!SimMode)
    {
        UAirBlueprintLib::LogMessage(TEXT("DroneInputController"), TEXT("ERROR: SimMode not found!"), LogDebugLevel::Failure);
        return;
    }

    // Initialize joystick
    Joystick = std::make_unique<SimJoyStick>();

    // Setup input bindings
    SetupInputBindings();

    UAirBlueprintLib::LogMessage(TEXT("DroneInputController"), TEXT("Initialized successfully"), LogDebugLevel::Informational);
}

void ADroneInputController::SetSimMode(ASimModeBase* InSimMode)
{
    SimMode = InSimMode;
}

void ADroneInputController::EnableInput(bool bEnable)
{
    bInputEnabled = bEnable;
    if (!bEnable)
    {
        CurrentThrottle = 0.0f;
        CurrentPitch = 0.0f;
        CurrentRoll = 0.0f;
        CurrentYaw = 0.0f;
    }
}

void ADroneInputController::SetupInputBindings()
{
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController)
    {
        return;
    }

    // Enable input for this controller
    EnableInput(PlayerController);

    UAirBlueprintLib::LogMessage(TEXT("DroneInputController"), TEXT("Input bindings configured"), LogDebugLevel::Informational);
}

void ADroneInputController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bInputEnabled || !SimMode)
    {
        return;
    }

    InputUpdateTimer += DeltaTime;

    // Process input at fixed rate
    if (InputUpdateTimer >= INPUT_UPDATE_RATE)
    {
        InputUpdateTimer = 0.0f;

        ProcessKeyboardInput(DeltaTime);
        ProcessGamepadInput(DeltaTime);
        ApplyInputToVehicle(DeltaTime);

        if (bDebugInputVisible && FrameCounter++ % 30 == 0)
        {
            LogInputState();
        }
    }
}

void ADroneInputController::ProcessKeyboardInput(float DeltaTime)
{
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController)
    {
        return;
    }

    // Throttle input
    if (PlayerController->IsInputKeyDown(EKeys::SpaceBar))
    {
        CurrentThrottle = FMath::Clamp(CurrentThrottle + DeltaTime * 2.0f, 0.0f, 1.0f);
    }
    else if (PlayerController->IsInputKeyDown(EKeys::LeftControl))
    {
        CurrentThrottle = FMath::Clamp(CurrentThrottle - DeltaTime * 2.0f, 0.0f, 1.0f);
    }

    // Pitch input (Forward/Backward)
    if (PlayerController->IsInputKeyDown(EKeys::W))
    {
        CurrentPitch = FMath::Clamp(CurrentPitch + DeltaTime * 2.0f, -1.0f, 1.0f);
    }
    else if (PlayerController->IsInputKeyDown(EKeys::S))
    {
        CurrentPitch = FMath::Clamp(CurrentPitch - DeltaTime * 2.0f, -1.0f, 1.0f);
    }
    else
    {
        CurrentPitch *= 0.9f; // Decay
    }

    // Roll input (Left/Right)
    if (PlayerController->IsInputKeyDown(EKeys::A))
    {
        CurrentRoll = FMath::Clamp(CurrentRoll - DeltaTime * 2.0f, -1.0f, 1.0f);
    }
    else if (PlayerController->IsInputKeyDown(EKeys::D))
    {
        CurrentRoll = FMath::Clamp(CurrentRoll + DeltaTime * 2.0f, -1.0f, 1.0f);
    }
    else
    {
        CurrentRoll *= 0.9f; // Decay
    }

    // Yaw input (Rotation)
    if (PlayerController->IsInputKeyDown(EKeys::Q))
    {
        CurrentYaw = FMath::Clamp(CurrentYaw - DeltaTime * 2.0f, -1.0f, 1.0f);
    }
    else if (PlayerController->IsInputKeyDown(EKeys::E))
    {
        CurrentYaw = FMath::Clamp(CurrentYaw + DeltaTime * 2.0f, -1.0f, 1.0f);
    }
    else
    {
        CurrentYaw *= 0.9f; // Decay
    }

    // Action keys
    if (PlayerController->WasInputKeyJustPressed(EKeys::R))
    {
        OnArmDisarm();
    }

    if (PlayerController->WasInputKeyJustPressed(EKeys::X))
    {
        OnEmergency();
    }

    if (PlayerController->WasInputKeyJustPressed(EKeys::P))
    {
        OnToggleRecording();
    }

    if (PlayerController->WasInputKeyJustPressed(EKeys::BackSpace))
    {
        SimMode->reset();
    }
}

void ADroneInputController::ProcessGamepadInput(float DeltaTime)
{
    if (!Joystick)
    {
        return;
    }

    SimJoyStick::State State;
    Joystick->getJoyStickState(JoystickIndex, State);

    if (!State.is_valid || !State.is_initialized)
    {
        return;
    }

    // Apply gamepad input
    ApplyDeadZone(State.left_y, AirSimInputBindings::FGamepadAxisBindings::DeadZone);
    ApplyDeadZone(State.right_x, AirSimInputBindings::FGamepadAxisBindings::DeadZone);
    ApplyDeadZone(State.left_x, AirSimInputBindings::FGamepadAxisBindings::DeadZone);
    ApplyDeadZone(State.right_y, AirSimInputBindings::FGamepadAxisBindings::DeadZone);

    // Map gamepad axes to drone controls
    CurrentThrottle = State.left_y; // Left stick vertical = throttle
    CurrentRoll = State.left_x;     // Left stick horizontal = roll
    CurrentPitch = State.right_x;   // Right stick horizontal = pitch
    CurrentYaw = State.right_y;     // Right stick vertical = yaw

    // Clamp all values
    CurrentThrottle = FMath::Clamp(CurrentThrottle, 0.0f, 1.0f);
    CurrentRoll = FMath::Clamp(CurrentRoll, -1.0f, 1.0f);
    CurrentPitch = FMath::Clamp(CurrentPitch, -1.0f, 1.0f);
    CurrentYaw = FMath::Clamp(CurrentYaw, -1.0f, 1.0f);
}

void ADroneInputController::ApplyInputToVehicle(float DeltaTime)
{
    if (!SimMode)
    {
        return;
    }

    PawnSimApi* VehicleSimApi = SimMode->getVehicleSimApi();
    if (!VehicleSimApi)
    {
        return;
    }

    // Convert input values to vehicle commands
    // This depends on the vehicle type and control mode
    // For now, we'll just store the values for external use
}

void ADroneInputController::ApplyDeadZone(float& AxisValue, float DeadZone)
{
    if (FMath::Abs(AxisValue) < DeadZone)
    {
        AxisValue = 0.0f;
    }
    else if (AxisValue > 0.0f)
    {
        AxisValue = (AxisValue - DeadZone) / (1.0f - DeadZone);
    }
    else
    {
        AxisValue = (AxisValue + DeadZone) / (1.0f - DeadZone);
    }
}

void ADroneInputController::OnThrottleUp()
{
    CurrentThrottle = FMath::Clamp(CurrentThrottle + 0.1f, 0.0f, 1.0f);
}

void ADroneInputController::OnThrottleDown()
{
    CurrentThrottle = FMath::Clamp(CurrentThrottle - 0.1f, 0.0f, 1.0f);
}

void ADroneInputController::OnThrottleReleased()
{
    // Can add hover logic here
}

void ADroneInputController::OnPitchForward()
{
    CurrentPitch = FMath::Clamp(CurrentPitch + 0.1f, -1.0f, 1.0f);
}

void ADroneInputController::OnPitchBackward()
{
    CurrentPitch = FMath::Clamp(CurrentPitch - 0.1f, -1.0f, 1.0f);
}

void ADroneInputController::OnPitchReleased()
{
    CurrentPitch = 0.0f;
}

void ADroneInputController::OnRollLeft()
{
    CurrentRoll = FMath::Clamp(CurrentRoll - 0.1f, -1.0f, 1.0f);
}

void ADroneInputController::OnRollRight()
{
    CurrentRoll = FMath::Clamp(CurrentRoll + 0.1f, -1.0f, 1.0f);
}

void ADroneInputController::OnRollReleased()
{
    CurrentRoll = 0.0f;
}

void ADroneInputController::OnYawLeft()
{
    CurrentYaw = FMath::Clamp(CurrentYaw - 0.1f, -1.0f, 1.0f);
}

void ADroneInputController::OnYawRight()
{
    CurrentYaw = FMath::Clamp(CurrentYaw + 0.1f, -1.0f, 1.0f);
}

void ADroneInputController::OnYawReleased()
{
    CurrentYaw = 0.0f;
}

void ADroneInputController::OnArmDisarm()
{
    if (!SimMode)
    {
        return;
    }

    PawnSimApi* VehicleSimApi = SimMode->getVehicleSimApi();
    if (VehicleSimApi)
    {
        UAirBlueprintLib::LogMessage(TEXT("DroneInputController"), TEXT("Arm/Disarm toggled"), LogDebugLevel::Informational);
    }
}

void ADroneInputController::OnEmergency()
{
    CurrentThrottle = 0.0f;
    CurrentPitch = 0.0f;
    CurrentRoll = 0.0f;
    CurrentYaw = 0.0f;

    UAirBlueprintLib::LogMessage(TEXT("DroneInputController"), TEXT("EMERGENCY STOP!"), LogDebugLevel::Failure);
}

void ADroneInputController::OnSwitchFlightMode()
{
    UAirBlueprintLib::LogMessage(TEXT("DroneInputController"), TEXT("Flight mode switched"), LogDebugLevel::Informational);
}

void ADroneInputController::OnToggleRecording()
{
    if (!SimMode)
    {
        return;
    }

    bRecordingActive = SimMode->toggleRecording();
    FString RecordingState = bRecordingActive ? TEXT("Recording STARTED") : TEXT("Recording STOPPED");
    UAirBlueprintLib::LogMessage(TEXT("DroneInputController"), *RecordingState, LogDebugLevel::Informational);
}

FVector ADroneInputController::GetThrottleAndRoll() const
{
    return FVector(CurrentThrottle, CurrentRoll, 0.0f);
}

FVector ADroneInputController::GetPitchAndYaw() const
{
    return FVector(CurrentPitch, CurrentYaw, 0.0f);
}

void ADroneInputController::LogInputState()
{
    FString DebugMsg = FString::Printf(
        TEXT("Throttle: %.2f | Pitch: %.2f | Roll: %.2f | Yaw: %.2f"),
        CurrentThrottle, CurrentPitch, CurrentRoll, CurrentYaw);
    UAirBlueprintLib::LogMessage(TEXT("DroneInput"), *DebugMsg, LogDebugLevel::Informational);
}

void ADroneInputController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Joystick.reset();
    Super::EndPlay(EndPlayReason);
}
