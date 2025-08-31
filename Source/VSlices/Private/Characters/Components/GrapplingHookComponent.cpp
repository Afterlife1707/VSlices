#include "Characters/Components/GrapplingHookComponent.h"
#include "CableComponent.h"
#include "Characters/VSlicesCharacter.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

UGrapplingHookComponent::UGrapplingHookComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UGrapplingHookComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
	if (bIsGrappling)
	{
		UpdateGrappleMovement(DeltaTime);
		UpdateCableVisuals(DeltaTime);
	}
}

void UGrapplingHookComponent::TryShoot()
{
	if (bIsGrappling)
	{
		ReleaseGrapple();
		return;
	}
    
	FVector Start, End;
	if (const APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
	{
		FVector CameraLocation;
		FRotator CameraRotation;
		PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
        
		Start = CameraLocation;
		End = Start + CameraRotation.Vector() * Range;
	}
    
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(OwnerCharacter);

	//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.0f, 0, 2.0f);

	if (FHitResult Hit; GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, TraceParams))
	{
		//DrawDebugSphere(GetWorld(), Hit.Location, 50.0f, 12, FColor::Green, false, 2.0f);
		UE_LOG(LogTemp, Warning, TEXT("Grapple HIT at: %s"), *Hit.Location.ToString());
		StartGrapple(Hit.Location);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Grapple MISS - no target found"));
	}
}

void UGrapplingHookComponent::StartGrapple(const FVector& TargetLocation)
{
    bIsGrappling = true;
	bShouldBoost = false;
    GrappleLocation = TargetLocation;
    
    MovementComponent->SetMovementMode(MOVE_Flying);
    
	const FVector ToTarget = GrappleLocation - OwnerCharacter->GetActorLocation();
	const FVector LaunchDirection = ToTarget.GetSafeNormal();
	FVector LaunchVelocity = LaunchDirection * GrappleSpeed;
    
	if (ToTarget.Z > 100.0f)
	{
		bShouldBoost = true;
	}
    
	if (ToTarget.Z < 0) 
	{
		LaunchVelocity.Z += 600.0f;
	}
	else
	{
		LaunchVelocity.Z += 400.0f;
	}
    OwnerCharacter->LaunchCharacter(LaunchVelocity, true, true);
	
    if (UCableComponent* Cable = OwnerCharacter->GetCable())
    {
        Cable->SetVisibility(true);
    }
}

void UGrapplingHookComponent::UpdateGrappleMovement(const float DeltaTime)
{
    const FVector CurrentLocation = OwnerCharacter->GetActorLocation();
    const FVector ToGrapple = GrappleLocation - CurrentLocation;
    
	const float Distance = ToGrapple.Size();
    if (Distance < 10.0f)
    {
        ReleaseGrapple();
        return;
    }
	
	if (bShouldBoost && MovementComponent->Velocity.Z < 0)
	{
		bShouldBoost = false;
	}
	
	const FVector SwingDirection = ToGrapple.GetSafeNormal();
    const FVector CurrentVelocity = MovementComponent->Velocity;
    
	// Stronger force for downward grapples
	float AdjustedSwingForce = SwingForce;
	if (ToGrapple.Z < 0) 
	{
		AdjustedSwingForce *= 1.5f; 
	}
    
	const float ForceMultiplier = FMath::Clamp(Distance / 300.0f, 0.8f, 2.0f);
	const FVector SwingForceVector = SwingDirection * AdjustedSwingForce * ForceMultiplier * DeltaTime;
    
	const FVector TargetVelocity = CurrentVelocity + SwingForceVector;
	MovementComponent->Velocity = FMath::VInterpTo(CurrentVelocity, TargetVelocity, DeltaTime, 4.0f);
}

void UGrapplingHookComponent::UpdateCableVisuals(const float DeltaTime) const
{
    if (UCableComponent* Cable = OwnerCharacter->GetCable())
    {
        const FVector CurrentStart = OwnerCharacter->GetActorLocation();
        const FVector DesiredEnd = GrappleLocation - CurrentStart;
        
        Cable->EndLocation = FMath::VInterpTo(Cable->EndLocation, DesiredEnd, DeltaTime, CableInterpSpeed);
    }
}

void UGrapplingHookComponent::ReleaseGrapple()
{
    if (!bIsGrappling) return;
    
    bIsGrappling = false;
    MovementComponent->SetMovementMode(MOVE_Walking);
    if (UCableComponent* Cable = OwnerCharacter->GetCable())
        Cable->SetVisibility(false);
}

bool UGrapplingHookComponent::ShouldBoost() const
{
	return bShouldBoost && MovementComponent && MovementComponent->Velocity.Z > 0;
}