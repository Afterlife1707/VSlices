// Copyright Epic Games, Inc. All Rights Reserved.

#include "Characters/Components/LandingComponent.h"

#include "LoggingMacros.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Characters/VSlicesCharacter.h"

ULandingComponent::ULandingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULandingComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Cache owner character reference
	OwnerCharacter = Cast<AVSlicesCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		LOG_ERROR("LandingComponent: Owner is not a Character!");
	}
}

void ULandingComponent::HandleFallDetection()
{
	if (!OwnerCharacter)
		return;

	const UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
	if (!MovementComp)
		return;

	if (!bWasFalling && MovementComp->IsFalling())
	{
		// Just started falling
		bWasFalling = true;
		LastVelocity = OwnerCharacter->GetVelocity().Length();
		FallStartZ = OwnerCharacter->GetActorLocation().Z;
	}

	if (bWasFalling && !MovementComp->IsFalling())
	{
		// Just landed
		bWasFalling = false;
		const float FallDistance = FallStartZ - OwnerCharacter->GetActorLocation().Z;
		HandleLanding(FallDistance);
	}
}

void ULandingComponent::HandleLanding(const float FallDistance) const
{
	LOG_INFO("Fall distance: %f and last velocity: %f", FallDistance, LastVelocity);

	if (!OwnerCharacter) return;
    
	const float MaxJogSpeed = OwnerCharacter->GetMaxJogSpeed();
    
	if (FallDistance >= HardLandingMinFallDistance)
	{
		if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
			MovementComp->DisableMovement();
		OwnerCharacter->PlayAnimMontage(HardLandAnim);
	}
	else if (FallDistance >= RollMinFallDistance && LastVelocity >= MaxJogSpeed - 50.f)
	{
		OwnerCharacter->PlayAnimMontage(RollAnim);
	}
}
