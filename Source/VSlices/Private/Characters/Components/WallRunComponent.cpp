// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Components/WallRunComponent.h"
#include "LoggingMacros.h"
#include "VectorTypes.h"
#include "Characters/VSlicesCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UWallRunComponent::UWallRunComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

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

	if(bIsWallRunning)
	{
		if(OwnerCharacter->GetVelocity().Length()<MinVelocity)
			StopWallRun();
	}
}

void UWallRunComponent::TryWallRun(const FHitResult& Hit)
{
	if(CheckForWall(Hit))
		StartWallRun(Hit.Normal);
	else
		StopWallRun();
}

bool UWallRunComponent::CheckForWall(const FHitResult& Hit) const
{
	const float Dot =  FMathf::Abs(FVector::DotProduct(Hit.Normal, OwnerCharacter->GetActorRightVector()));
	FFindFloorResult FloorHit;
	MovementComp->FindFloor(OwnerCharacter->GetActorLocation(), FloorHit, true);
	
	return (MovementComp->IsFalling() && (FloorHit.FloorDist>MinWallHeight) && (Dot>=0.8f) && OwnerCharacter->GetVelocity().Length()>MinVelocity);
}

void UWallRunComponent::StartWallRun(const FVector& WallNormal)
{
	if(!MovementComp->IsFalling() || bIsWallRunning) return;

	bIsWallRunning = true;
	MovementComp->Velocity = FVector(MovementComp->Velocity.X, MovementComp->Velocity.Y, 0);
	MovementComp->SetPlaneConstraintEnabled(true);
	MovementComp->SetPlaneConstraintNormal(WallNormal);
	MovementComp->GravityScale = WallRunGravityScale;
	GetWorld()->GetTimerManager().SetTimer(WallRunTimerHandle, this, &UWallRunComponent::StopWallRun, WallRunTimer, false);
}

void UWallRunComponent::StopWallRun() const
{
	MovementComp->SetPlaneConstraintEnabled(false);
	MovementComp->GravityScale = DefaultGravityScale;
}

void UWallRunComponent::Jump()
{
	StopWallRun();
	
	const float JumpForce = OwnerCharacter->GetVelocity().Length() * JumpForceMultiplier;
	const FVector JumpHeightBoost = FVector(0,0,OwnerCharacter->GetVelocity().Length());
	const FVector DirLaunchVelocity = MovementComp->GetPlaneConstraintNormal()* JumpForce + JumpHeightBoost;
	
	OwnerCharacter->LaunchCharacter(DirLaunchVelocity, false, true);
	bIsWallRunning = false;
}

