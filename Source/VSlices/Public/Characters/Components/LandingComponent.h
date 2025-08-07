// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LandingComponent.generated.h"

class AVSlicesCharacter;
class USprintComponent;
class UAnimMontage;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VSLICES_API ULandingComponent : public UActorComponent
{
    GENERATED_BODY()

public:	
    ULandingComponent();

protected:
    virtual void BeginPlay() override;

public:
    // Called from character's Tick to handle fall detection
    void HandleFallDetection();
    void HandleLanding(float FallDistance) const;

private:
    UPROPERTY()
    AVSlicesCharacter* OwnerCharacter;
	
    // Fall detection state
    float FallStartZ = 0.f;
    bool bWasFalling = false;
    float LastVelocity = 0.f;
	
    // Landing thresholds and animations
    UPROPERTY(EditDefaultsOnly, Category = "Landing")
    float HardLandingMinFallDistance = 600.f;
    UPROPERTY(EditDefaultsOnly, Category = "Landing")
    float RollMinFallDistance = 300.f;
    UPROPERTY(EditDefaultsOnly, Category = "Landing")
    UAnimMontage* RollAnim;
    UPROPERTY(EditDefaultsOnly, Category = "Landing")
    UAnimMontage* HardLandAnim;
};