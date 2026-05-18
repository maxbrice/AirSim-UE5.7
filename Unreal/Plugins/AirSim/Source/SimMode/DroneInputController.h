#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InputActionValue.h"
#include "SimJoyStick/SimJoyStick.h"
#include "InputBindingsConfig.h"
#include "DroneInputController.generated.h"

class ASimModeBase;
class PawnSimApi;

/**
 * Drone Input Controller for AirSim UE5.7
 * Handles all keyboard and joystick input for drone piloting
 */
UCLASS()
class AIRSIM_API ADroneInputController : public AActor
{
    GENERATED_BODY()

public:
    ADroneInputController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Set the SimMode reference
    void SetSimMode(ASimModeBase* InSimMode);

    // Enable/Disable input
    UFUNCTION(BlueprintCallable, Category = "AirSim|Input")
    void EnableInput(bool bEnable);

    // Get current input state
    UFUNCTION(BlueprintCallable, Category = "AirSim|Input")
    FVector GetThrottleAndRoll() const;

    UFUNCTION(BlueprintCallable, Category = "AirSim|Input")
    FVector GetPitchAndYaw() const;

protected:
    // Input callback functions
    virtual void SetupInputBindings();
    virtual void ProcessKeyboardInput(float DeltaTime);
    virtual void ProcessGamepadInput(float DeltaTime);
    virtual void ApplyInputToVehicle(float DeltaTime);

    // Keyboard input callbacks
    void OnThrottleUp();
    void OnThrottleDown();
    void OnThrottleReleased();

    void OnPitchForward();
    void OnPitchBackward();
    void OnPitchReleased();

    void OnRollLeft();
    void OnRollRight();
    void OnRollReleased();

    void OnYawLeft();
    void OnYawRight();
    void OnYawReleased();

    void OnArmDisarm();
    void OnEmergency();
    void OnSwitchFlightMode();
    void OnToggleRecording();

    // Helper functions
    void NormalizeInput(float& Value, float MaxValue);
    void ApplyDeadZone(float& AxisValue, float DeadZone);
    void LogInputState();

protected:
    UPROPERTY()
    ASimModeBase* SimMode;

    // Input state
    float CurrentThrottle;
    float CurrentPitch;
    float CurrentRoll;
    float CurrentYaw;

    // Flags
    bool bInputEnabled;
    bool bRecordingActive;
    bool bDebugInputVisible;

    // Joystick
    std::unique_ptr<SimJoyStick> Joystick;
    int32 JoystickIndex;

    // Timing
    float InputUpdateTimer;
    static constexpr float INPUT_UPDATE_RATE = 0.016f; // 60Hz

    // Debug
    int32 FrameCounter;
};
