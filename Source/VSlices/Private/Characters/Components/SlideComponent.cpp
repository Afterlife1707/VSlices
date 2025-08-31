// Copyright Epic Games, Inc. All Rights Reserved.

#include "Characters/Components/SlideComponent.h"
#include "Characters/VSlicesCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Characters/Components/SlopeComponent.h"

USlideComponent::USlideComponent()
{
}

void USlideComponent::StartSlide()
{
    if (bIsSliding || !MovementComponent->IsMovingOnGround())
        return;

    bIsSliding = true;
    SlideElapsed = 0.f;
    ActualSlideDuration = SlideDuration;

    MovementComponent->MaxWalkSpeedCrouched = SlideSpeed;
    OwnerCharacter->LaunchForward();
    OwnerCharacter->Crouch();
}

void USlideComponent::HandleSlideTick(float DeltaSeconds)
{
    SlideElapsed += DeltaSeconds;

    const float CurrentSpeed = OwnerCharacter->GetVelocity().Size();
    if (CurrentSpeed < MinSlideSpeed || SlideElapsed >= ActualSlideDuration)
    {
        StopSlide();
        return;
    }
    
    const FSlopeInfo& SlopeInfo = OwnerCharacter->GetSlopeInfo();
    
    if (!SlopeInfo.bIsOnSlope) return;
    if (SlopeInfo.bIsUphill && SlideElapsed > SlideDuration * UphillDurationMultiplier)
        StopSlide();
    else if (SlopeInfo.bIsDownhill && CurrentSpeed > DownhillSpeedThreshold)
        ActualSlideDuration = FMath::Min(ActualSlideDuration + SlideExtensionPerTick, MaxSlideDuration);
}

void USlideComponent::StopSlide()
{
    if (!bIsSliding)
        return;

    bIsSliding = false;
    SlideElapsed = 0.f;
    ActualSlideDuration = 0.f;

    MovementComponent->MaxWalkSpeedCrouched = OwnerCharacter->GetMaxCrouchJogSpeed();
}