// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Components/WallRunComponent.h"
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
	DefaultGravityScale = MovementComponent->GravityScale;
}

void UWallRunComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bCameraTilt)
		UpdateCameraTilt(DeltaTime);
	
	if (!bIsWallRunning || !IsValid(OwnerCharacter)) return;
	
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
	{
		if(LastWallActor==Hit.GetActor()) return;
		LastWallActor = Hit.GetActor();
		StartWallRun(Hit.Normal);
	}
	else if (!bIsWallRunning) 
		StopWallRun();
}

bool UWallRunComponent::CheckForWall(const FHitResult& Hit) 
{
	if(!MovementComponent->IsFalling()) return false;
	
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
	MovementComponent->FindFloor(OwnerCharacter->GetActorLocation(), FloorResult, true);
    
	return FloorResult.FloorDist > MinWallHeight;
}

void UWallRunComponent::StartWallRun(const FVector& WallNormal)
{
	if(!MovementComponent->IsFalling() || bIsWallRunning) return;
	//LOG_INFO("start Wall run");

	bIsWallRunning = true;
	bCameraTilt = true;
	FVector NewVelocity = MovementComponent->Velocity;
	NewVelocity.Z = FMath::Max(0.0f, NewVelocity.Z * 0.5f);
	MovementComponent->Velocity = NewVelocity;
	MovementComponent->SetPlaneConstraintEnabled(true);
	MovementComponent->SetPlaneConstraintNormal(WallNormal);
	MovementComponent->GravityScale = WallRunGravityScale;
	
	GetWorld()->GetTimerManager().SetTimer(WallRunTimerHandle, this, &UWallRunComponent::StopWallRun, WallRunTimer, false);
}

void UWallRunComponent::StopWallRun()
{
	//LOG_INFO("Stop Wall run");
	GetWorld()->GetTimerManager().ClearTimer(WallRunTimerHandle);
	MovementComponent->SetPlaneConstraintEnabled(false);
	MovementComponent->GravityScale = DefaultGravityScale;
	bIsWallRunning = false;
	bCameraTilt = true;
}

void UWallRunComponent::Jump()
{
	StopWallRun();
	const APlayerController* PC = OwnerCharacter->GetController<APlayerController>();
	if(!PC) return;
	const FVector LookDirection = PC->GetControlRotation().Vector();
    
	const float JumpForce = OwnerCharacter->GetVelocity().Length() * JumpForceMultiplier;
	const FVector WallNormal = MovementComponent->GetPlaneConstraintNormal();
	FVector LaunchDirection = (LookDirection + WallNormal * 0.5f).GetSafeNormal();
    
	LaunchDirection.Z = 0.4f; 
	LaunchDirection = LaunchDirection.GetSafeNormal();
    
	FVector DirLaunchVelocity = LaunchDirection * JumpForce;
	DirLaunchVelocity.Z += JumpHeightBoost;
	OwnerCharacter->LaunchCharacter(DirLaunchVelocity, false, true);
	bIsWallRunning = false;
}

void UWallRunComponent::ResetWallRun()
{
	GetWorld()->GetTimerManager().ClearTimer(WallRunTimerHandle);
	bIsWallRunning = false;
	Direction = EWallRunDir::None;
	bCameraTilt = false;
	LastWallActor=nullptr;
}

void UWallRunComponent::UpdateCameraTilt(const float DeltaTime)
{
	const float TargetTilt = bIsWallRunning ? ((Direction == EWallRunDir::Left) ? -CameraTiltAngle : CameraTiltAngle) : 0.0f;
	CurrentCameraTilt = FMath::FInterpTo(CurrentCameraTilt, TargetTilt, DeltaTime, CameraTiltSpeed);
    
	if (FMath::Abs(CurrentCameraTilt - TargetTilt) < 0.1f)
	{
		CurrentCameraTilt = TargetTilt; 
		bCameraTilt = false;
	}
}
