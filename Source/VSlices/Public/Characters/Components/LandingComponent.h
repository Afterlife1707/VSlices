// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ParkourComponentBase.h"
#include "Components/ActorComponent.h"
#include "LandingComponent.generated.h"

class UCharacterMovementComponent;
class AVSlicesCharacter;
class USprintComponent;
class UAnimMontage;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VSLICES_API ULandingComponent : public UParkourComponentBase
{
    GENERATED_BODY()

public:	
    ULandingComponent();

    // Called from character's Tick
    void HandleFallDetection();
    void HandleLanding(float FallDistance) const;

private:
    float FallStartZ = 0.f;
    bool bWasFalling = false;
    float LastVelocity = 0.f;
	
    UPROPERTY(EditDefaultsOnly, Category = "Landing")
    float HardLandingMinFallDistance = 600.f;
    UPROPERTY(EditDefaultsOnly, Category = "Landing")
    float RollMinFallDistance = 300.f;
    UPROPERTY(EditDefaultsOnly, Category = "Landing")
    UAnimMontage* RollAnim;
    UPROPERTY(EditDefaultsOnly, Category = "Landing")
    UAnimMontage* HardLandAnim;
};