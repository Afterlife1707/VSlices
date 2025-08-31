// Copyright Epic Games, Inc. All Rights Reserved.

#include "Characters/Components/SprintComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Characters/VSlicesCharacter.h" 

USprintComponent::USprintComponent()
{
}

void USprintComponent::BeginPlay()
{
    Super::BeginPlay();
  
    if (MovementComponent)
    {
        MovementComponent->MaxWalkSpeed = OwnerCharacter->GetMaxJogSpeed();
        MovementComponent->MaxWalkSpeedCrouched = OwnerCharacter->GetMaxCrouchJogSpeed();
    }
    else
    {
        LOG_ERROR("Invalid Movement Component");
    }
}

void USprintComponent::StartSprinting()
{
    if (!bCanSprint || bSprintOnCooldown || !OwnerCharacter || !MovementComponent) 
        return;
    
    bIsSprinting = true;
    if (!OwnerCharacter->bIsCrouched)
        MovementComponent->MaxWalkSpeed = OwnerCharacter->GetMaxSprintSpeed();
    else
        MovementComponent->MaxWalkSpeedCrouched = OwnerCharacter->GetMaxCrouchSprintSpeed();
}

void USprintComponent::StopSprinting()
{
    if (!OwnerCharacter || !MovementComponent) return;
    
    bIsSprinting = false;
    if (!OwnerCharacter->bIsCrouched)
        MovementComponent->MaxWalkSpeed = OwnerCharacter->GetMaxJogSpeed();
    else 
        MovementComponent->MaxWalkSpeedCrouched = OwnerCharacter->GetMaxCrouchJogSpeed();
}

void USprintComponent::SprintCheck(const float ForwardValue, const float RightValue)
{
    if (!OwnerCharacter || !MovementComponent)
    {
        LOG_ERROR("SprintComponent: Missing OwnerCharacter or MovementComponent in SprintCheck");
        return;
    }
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
        GetWorld()->GetTimerManager().SetTimer(SprintCooldownTimerHandle,this,&USprintComponent::EndSprintCooldown,SprintCooldownDuration,false);
    }
}

void USprintComponent::EndSprintCooldown()
{
    bSprintOnCooldown = false;
    GetWorld()->GetTimerManager().ClearTimer(SprintCooldownTimerHandle);
}