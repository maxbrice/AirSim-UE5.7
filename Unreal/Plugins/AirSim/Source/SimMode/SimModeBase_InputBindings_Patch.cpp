// Updated setupInputBindings() function for SimModeBase.cpp
// Replace the existing setupInputBindings() function with this version

void ASimModeBase::setupInputBindings()
{
    UAirBlueprintLib::EnableInput(this);

    // Bind Reset action
    UAirBlueprintLib::BindActionToKey("InputEventResetAll", EKeys::BackSpace, this, &ASimModeBase::reset);

    // Spawn and setup DroneInputController with all drone control bindings
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    ADroneInputController* DroneInputController = this->GetWorld()->SpawnActor<ADroneInputController>(
        ADroneInputController::StaticClass(),
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        SpawnParams);

    if (DroneInputController)
    {
        DroneInputController->SetSimMode(this);
        spawned_actors_.Add(DroneInputController);
        UAirBlueprintLib::LogMessage(
            TEXT("SimModeBase"),
            TEXT("Drone input controller initialized - All controls active"),
            LogDebugLevel::Informational);
    }
    else
    {
        UAirBlueprintLib::LogMessage(
            TEXT("SimModeBase"),
            TEXT("WARNING: Failed to spawn DroneInputController"),
            LogDebugLevel::Warning);
    }
}

// Also add this include at the top of SimModeBase.cpp:
// #include "SimMode/DroneInputController.h"
