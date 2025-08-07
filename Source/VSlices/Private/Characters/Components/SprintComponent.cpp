// Copyright Epic Games, Inc. All Rights Reserved.

#include "Characters/Components/SprintComponent.h"
#include "LoggingMacros.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Characters/VSlicesCharacter.h" 

USprintComponent::USprintComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void USprintComponent::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerCharacter = Cast<AVSlicesCharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        if (MovementComponent)
        {
            MovementComponent->MaxWalkSpeed = MaxJogSpeed;
            MovementComponent->MaxWalkSpeedCrouched = MaxCrouchJogSpeed;
        }
        else
        {
            LOG_ERROR("Invalid Movement Component");
        }
    }
    else
    {
        LOG_ERROR("SprintComponent: Owner is not a Character!");
    }
}

void USprintComponent::StartSprinting()
{
    if(!bCanSprint || bSprintOnCooldown) return;
    bIsSprinting = true;
    
    if(!OwnerCharacter)
    {
        LOG_ERROR("SprintComponent: Owner is not a Character!");
        return;
    }
    if(!OwnerCharacter->bIsCrouched)
    {
        MovementComponent->MaxWalkSpeed = MaxSprintSpeed;
    }
    else
        MovementComponent->MaxWalkSpeedCrouched = MaxCrouchSprintSpeed;
}

void USprintComponent::StopSprinting()
{
    bIsSprinting = false;
        
    if(!OwnerCharacter->bIsCrouched)
    {
        MovementComponent->MaxWalkSpeed = MaxJogSpeed;
    }
    else 
        MovementComponent->MaxWalkSpeedCrouched = MaxCrouchJogSpeed;
}

void USprintComponent::SprintCheck(const float ForwardValue, const float RightValue)
{
    const bool bMovingForward = ForwardValue > 0.001f;
    const bool bMovingBackward = ForwardValue < -0.001f;
    const bool bMovingSideways = FMath::Abs(RightValue) > 0.001f;
    
    bCanSprint = !bMovingSideways && !bMovingBackward && bMovingForward && !bSprintOnCooldown;
    if (bIsSprinting && (!bCanSprint || bSprintOnCooldown))
    {
        StopSprinting();
        StartSprintCooldown();
    }
}

void USprintComponent::StartSprintCooldown()
{
    if (SprintCooldownDuration > 0.0f)
    {
        bSprintOnCooldown = true;
        GetWorld()->GetTimerManager().SetTimer(
            SprintCooldownTimerHandle,
            this,
            &USprintComponent::EndSprintCooldown,
            SprintCooldownDuration,
            false
        );
    }
}

void USprintComponent::EndSprintCooldown()
{
    bSprintOnCooldown = false;
    GetWorld()->GetTimerManager().ClearTimer(SprintCooldownTimerHandle);
}