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

}

void UWallRunComponent::TryWallRun(const FHitResult& Hit, AActor* Other)
{
	if(CheckForWall(Hit))
		StartWallRun(Hit.Normal);
	else
		StopWallRun();
}

bool UWallRunComponent::CheckForWall(const FHitResult& Hit) const
{
	const float Dot =  FMathf::Abs(FVector::DotProduct(Hit.Normal, OwnerCharacter->GetActorRightVector()));
	return (Dot>=0.8f);
}

void UWallRunComponent::StartWallRun(const FVector& WallNormal)
{
	if(!MovementComp->IsFalling() || !bCanWallRun) return;
	MovementComp->SetPlaneConstraintEnabled(true);
	MovementComp->SetPlaneConstraintNormal(WallNormal);
	MovementComp->GravityScale = WallRunGravityScale;
	GetWorld()->GetTimerManager().SetTimer(WallRunTimerHandle, this, &UWallRunComponent::StopWallRun, WallRunTimer, false);
	bCanWallRun = false;
}

void UWallRunComponent::StopWallRun() const
{
	MovementComp->SetPlaneConstraintEnabled(false);
	MovementComp->GravityScale = DefaultGravityScale;
}

