// Copyright Epic Games, Inc. All Rights Reserved.

#include "Characters/Components/LandingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Characters/VSlicesCharacter.h"

ULandingComponent::ULandingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULandingComponent::HandleFallDetection()
{
	if (!OwnerCharacter || !MovementComponent)
		return;

	if (!bWasFalling && MovementComponent->IsFalling())
	{
		// Just started falling
		bWasFalling = true;
		LastVelocity = OwnerCharacter->GetVelocity().Length();
		FallStartZ = OwnerCharacter->GetActorLocation().Z;
	}

	if (bWasFalling && !MovementComponent->IsFalling())
	{
		// Just landed
		bWasFalling = false;
		const float FallDistance = FallStartZ - OwnerCharacter->GetActorLocation().Z;
		HandleLanding(FallDistance);
	}
}

void ULandingComponent::HandleLanding(const float FallDistance) const
{
	//LOG_INFO("Fall distance: %f and last velocity: %f", FallDistance, LastVelocity);

	if (!OwnerCharacter) return;
    
	const float MaxJogSpeed = OwnerCharacter->GetMaxJogSpeed();
    
	if (FallDistance >= HardLandingMinFallDistance)
	{
		MovementComponent->DisableMovement();
		if (HardLandAnim)
			OwnerCharacter->PlayAnimMontage(HardLandAnim);
		else
			WarnMissingAsset("HardLandAnim");
	}
	else if (FallDistance >= RollMinFallDistance && LastVelocity >= MaxJogSpeed - 50.f)
	{
		if (RollAnim)
			OwnerCharacter->PlayAnimMontage(RollAnim);
		else
			WarnMissingAsset("RollAnim");
	}
}
