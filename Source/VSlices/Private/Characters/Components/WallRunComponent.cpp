// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Components/WallRunComponent.h"
#include "LoggingMacros.h"
#include "VectorTypes.h"
#include "Characters/VSlicesCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UWallRunComponent::UWallRunComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;
}


void UWallRunComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AVSlicesCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		LOG_ERROR("VaultComponent: Owner is not a VSlicesCharacter!");
		return;
	}
    
    MovementComp = OwnerCharacter->GetCharacterMovement();
	DefaultGravityScale = MovementComp->GravityScale;
}

void UWallRunComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!bIsWallRunning || !IsValid(OwnerCharacter)) 
		return;
	const FVector CurrentVelocity = OwnerCharacter->GetVelocity();
	const float HorizontalSpeed = FVector2D(CurrentVelocity.X, CurrentVelocity.Y).Size();
	if (HorizontalSpeed < MinVelocity)
		StopWallRun();
}

void UWallRunComponent::TryWallRun(const FHitResult& Hit)
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastWallRunAttempt < WallRunAttemptCooldown)
		return;
        
	LastWallRunAttempt = CurrentTime;
    
	if(CheckForWall(Hit))
		StartWallRun(Hit.Normal);
	else if (!bIsWallRunning) 
		StopWallRun();
}

bool UWallRunComponent::CheckForWall(const FHitResult& Hit) 
{
	if(!MovementComp->IsFalling()) return false;
	
	const FVector CurrentVelocity = OwnerCharacter->GetVelocity();
	const float HorizontalSpeed = FVector2D(CurrentVelocity.X, CurrentVelocity.Y).Size();
	if (HorizontalSpeed < MinVelocity)
		return false;

	float WallDot = FVector::DotProduct(Hit.Normal, OwnerCharacter->GetActorRightVector());
	Direction = (WallDot > 0) ? EWallRunDir::Right : EWallRunDir::Left;
	WallDot = FMath::Abs(WallDot);
	if (WallDot < MinWallAngleDot)
		return false;

	FFindFloorResult FloorResult;
	MovementComp->FindFloor(OwnerCharacter->GetActorLocation(), FloorResult, true);
    
	return FloorResult.FloorDist > MinWallHeight;
}

void UWallRunComponent::StartWallRun(const FVector& WallNormal)
{
	if(!MovementComp->IsFalling() || bIsWallRunning) return;

	bIsWallRunning = true;
	FVector NewVelocity = MovementComp->Velocity;
	NewVelocity.Z = FMath::Max(0.0f, NewVelocity.Z * 0.5f); // Reduce but don't eliminate upward velocity
	MovementComp->Velocity = NewVelocity;
	MovementComp->SetPlaneConstraintEnabled(true);
	MovementComp->SetPlaneConstraintNormal(WallNormal);
	MovementComp->GravityScale = WallRunGravityScale;
	
	GetWorld()->GetTimerManager().SetTimer(WallRunTimerHandle, this, &UWallRunComponent::StopWallRun, WallRunTimer, false);
}

void UWallRunComponent::StopWallRun()
{
	GetWorld()->GetTimerManager().ClearTimer(WallRunTimerHandle);
	MovementComp->SetPlaneConstraintEnabled(false);
	MovementComp->GravityScale = DefaultGravityScale;
}

void UWallRunComponent::Jump()
{
	StopWallRun();
	
	const float JumpForce = OwnerCharacter->GetVelocity().Length() * JumpForceMultiplier;
	const FVector DirLaunchVelocity = MovementComp->GetPlaneConstraintNormal()* JumpForce + JumpHeightBoost;
	
	OwnerCharacter->LaunchCharacter(DirLaunchVelocity, false, true);
	bIsWallRunning = false;
}

void UWallRunComponent::ResetWallRun()
{
	GetWorld()->GetTimerManager().ClearTimer(WallRunTimerHandle);
	bIsWallRunning = false;
	Direction = EWallRunDir::None;
}

