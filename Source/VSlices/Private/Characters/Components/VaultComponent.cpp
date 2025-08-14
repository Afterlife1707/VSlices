#include "Characters/Components/VaultComponent.h"
#include "LoggingMacros.h"
#include "Characters/VSlicesCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h" // Add this include for debug drawing

UVaultComponent::UVaultComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVaultComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<AVSlicesCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		LOG_ERROR("VaultComponent: Owner is not a VSlicesCharacter!");
		return;
	}
	MovementComp = OwnerCharacter->GetCharacterMovement();
}

bool UVaultComponent::TryVault()
{
	if (!OwnerCharacter || bIsVaulting || !OwnerCharacter->GetCharacterMovement()->IsMovingOnGround())
	{
		LOG_INFO("Try Vault fail");
		return false;
	}
	// Trace forward for obstacle
	const FVector Start = OwnerCharacter->GetActorLocation() - FVector(0, 0, 10); 
	const FVector End = Start + OwnerCharacter->GetActorForwardVector() * 100.f;

	FHitResult ForwardHit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);
	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);
	
	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.0f, 0, 2.0f);
	//detect wall
	if (!GetWorld()->LineTraceSingleByObjectType(ForwardHit, Start, End, ObjParams, Params))
	{
		LOG_INFO("line trace false");
		DrawDebugSphere(GetWorld(), End, 10.0f, 12, FColor::Orange, false, 2.0f);
		return false;
	}
	// Debug draw hit point
	DrawDebugSphere(GetWorld(), ForwardHit.Location, 15.0f, 12, FColor::Green, false, 2.0f);
	DrawDebugLine(GetWorld(), ForwardHit.Location, ForwardHit.Location + ForwardHit.Normal * 50.0f, FColor::Blue, false, 2.0f, 0, 3.0f);
	
	const FVector WallNormal = ForwardHit.Normal;
	const FVector WallLocation = ForwardHit.Location;
	
	const FVector Start2 = WallLocation + FVector(0, 0, 200.0f) - WallNormal * 10.0f; // Start above the wall, slightly back
	const FVector End2 = Start2 - FVector(0, 0, 400.0f); // Trace down to find top of wall

	DrawDebugLine(GetWorld(), Start2, End2, FColor::Yellow, false, 2.0f, 0, 2.0f);

	FHitResult HeightHit; 
	//calculate wall height
	if (GetWorld()->LineTraceSingleByObjectType(HeightHit, Start2, End2, ObjParams, Params))
	{
		const FVector WallTop = HeightHit.Location;
		
		// Debug draw wall top
		DrawDebugSphere(GetWorld(), WallTop, 12.0f, 12, FColor::Purple, false, 2.0f);
		
		// Calculate height difference from character to wall top
		const float WallHeight = WallTop.Z - OwnerCharacter->GetActorLocation().Z; 
		LOG_INFO("Wall height : %.2f", WallHeight);
		//calculate wall height
		if(WallHeight >= MinHeightForShortVault && WallHeight<=MaxHeightForShortVault)
		{
			//short wall
			LOG_INFO("Short vault - Wall height: %f", WallHeight);
			VaultLerpTime = OwnerCharacter->PlayAnimMontage(VaultShortMontage);
		}
		else if(WallHeight>MaxHeightForShortVault && WallHeight<=MaxHeightForTraverse)
		{
			//tall wall
			LOG_INFO("Tall vault - Wall height: %f", WallHeight);
			VaultLerpTime = OwnerCharacter->PlayAnimMontage(VaultTallMontage);
		}
		else
		{
			LOG_INFO("Vault FAIL: Wall height %.2f not within vaultable range", WallHeight);
			return false;
		}

		if (VaultLerpTime <= 0.f)
		{
			LOG_INFO("Vault FAIL: Animation montage failed to play");
			return false;
		}
		LOG_INFO("Vault SUCCESS: Montage duration %.2f", VaultLerpTime);
		OwnerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MovementComp->SetMovementMode(MOVE_Flying);

		GetWorld()->GetTimerManager().SetTimer(
			VaultMoveTimerHandle,
			this,
			&UVaultComponent::FinishVault,
			VaultLerpTime,
			false
		);
		bIsVaulting = true;
		return true;
	}
	
	DrawDebugSphere(GetWorld(), End2, 8.0f, 12, FColor::Red, false, 2.0f);
	LOG_INFO("Could not find wall top");
	return false;
}

void UVaultComponent::StartVault(EVaultType VaultType, const FVector& TargetLocation)
{
	if (!OwnerCharacter)
		return;

	bIsVaulting = true;

	VaultStartLocation = OwnerCharacter->GetActorLocation();
	VaultTargetLocation = TargetLocation;
	VaultStartRotation = OwnerCharacter->GetActorRotation();
	VaultTargetRotation = OwnerCharacter->GetActorRotation();

	UAnimMontage* MontageToPlay = nullptr;
	switch (VaultType)
	{
		case EVaultType::VAULT_SHORT: MontageToPlay = VaultShortMontage; break;
		case EVaultType::VAULT_TALL:  MontageToPlay = VaultTallMontage; break;
		case EVaultType::CLIMB_SHORT: MontageToPlay = ClimbShortMontage; break;
		case EVaultType::CLIMB_TALL:  MontageToPlay = ClimbTallMontage; break;
	}

	if (MontageToPlay)
	{
		VaultLerpTime = OwnerCharacter->PlayAnimMontage(MontageToPlay);
		if (VaultLerpTime <= 0.f)
			VaultLerpTime = 1.f; //fallback
	}

	OwnerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MovementComp->SetMovementMode(MOVE_Flying);

	GetWorld()->GetTimerManager().SetTimer(
		VaultMoveTimerHandle,
		this,
		&UVaultComponent::MoveCharacterToTarget,
		0.01f,
		true
	);
}

void UVaultComponent::MoveCharacterToTarget()
{
	if (!OwnerCharacter)
		return;

	const float LerpStep = 0.01f / VaultLerpTime;
	VaultLerpAlpha += LerpStep;
	VaultLerpAlpha = FMath::Clamp(VaultLerpAlpha, 0.f, 0.8f);

	const FVector NewLocation = FMath::Lerp(VaultStartLocation, VaultTargetLocation, VaultLerpAlpha);
	const FRotator NewRotation = FMath::Lerp(VaultStartRotation, VaultTargetRotation, VaultLerpAlpha);

	OwnerCharacter->SetActorLocationAndRotation(NewLocation, NewRotation, false, nullptr, ETeleportType::TeleportPhysics);

	if (VaultLerpAlpha >= 0.8f)
	{
		GetWorld()->GetTimerManager().ClearTimer(VaultMoveTimerHandle);
	}
}

void UVaultComponent::FinishVault()
{
	if (!OwnerCharacter) return;
	
	bIsVaulting = false;
	OwnerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MovementComp->SetMovementMode(MOVE_Walking);
	
	GetWorld()->GetTimerManager().ClearTimer(VaultMoveTimerHandle);
}