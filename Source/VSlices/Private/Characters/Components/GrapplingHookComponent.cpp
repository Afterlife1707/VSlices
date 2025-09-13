#include "Characters/Components/GrapplingHookComponent.h"
#include "CableComponent.h"
#include "Characters/VSlicesCharacter.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

UGrapplingHookComponent::UGrapplingHookComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    GrapplePullAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("GrapplePullAudio"));
    GrapplePullAudioComponent->bAutoActivate = false;
}

void UGrapplingHookComponent::BeginPlay()
{
    Super::BeginPlay();
    
	CurrentCooldown = GrappleCooldown;
    OriginalCapsuleHalfHeight = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

void UGrapplingHookComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsMantling)
    {
        UpdateMantle(DeltaTime);
        return;
    }
    if (!bIsGrappling || CurrentCooldown <= 0.0f)
        return;

    CurrentCooldown -= DeltaTime;
    const FVector ToTarget = GrappleLocation - OwnerCharacter->GetActorLocation();
    Distance = ToTarget.Length();
    
    // Apply continuous pulling force
    const FVector PullDirection = ToTarget.GetSafeNormal();
    const float PullStrength = CalculatePullStrength(ToTarget);
    const FVector PullForce = PullDirection * PullStrength * DeltaTime;
    MovementComponent->AddImpulse(PullForce, true);
    
    if (ShouldApplyAntiGravity(ToTarget))
        MovementComponent->AddImpulse(FVector(0, 0, HorizontalAntiGravityForce * DeltaTime), true);
    
    if (CurrentCooldown <= 0.0f || Distance < ReleaseDistance)
    {
        CurrentCooldown = GrappleCooldown;
        ClimbAtEnd();
    }
    
    UpdateCableVisuals(DeltaTime);
}

void UGrapplingHookComponent::TryShoot()
{
    if (bIsGrappling) return;
    if (GrappleStart)
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), GrappleStart, OwnerCharacter->GetActorLocation());
    const APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
    if (!PC) return;
    
    FVector CameraLocation;
    FRotator CameraRotation;
    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

    const FVector Start = CameraLocation;
    const FVector End = Start + CameraRotation.Vector() * Range;
    
    FCollisionQueryParams TraceParams;
    TraceParams.AddIgnoredActor(OwnerCharacter);

    FHitResult Hit;
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, TraceParams))
    {
        UE_LOG(LogTemp, Warning, TEXT("Grapple HIT at: %s"), *Hit.Location.ToString());
        StartGrapple(Hit.Location);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Grapple MISS - no target found"));
    }
    
    CurrentCooldown = GrappleCooldown;
}

void UGrapplingHookComponent::StartGrapple(const FVector& TargetLocation)
{
    bIsGrappling = true;
    GrappleLocation = TargetLocation;
    if (GrappleAttach)
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), GrappleAttach, TargetLocation);
    if (GrapplePullAudioComponent && GrapplePull)
        GrapplePullAudioComponent->Play();
    
    OwnerCharacter->GetCapsuleComponent()->SetCapsuleHalfHeight(OriginalCapsuleHalfHeight/2);
    // Initial upward boost
    OwnerCharacter->LaunchCharacter(FVector(0, 0, InitialUpwardBoost), true, true);
    MovementComponent->SetMovementMode(MOVE_Flying);
    
    if (UCableComponent* Cable = OwnerCharacter->GetCable())
        Cable->SetVisibility(true);
}

void UGrapplingHookComponent::UpdateCableVisuals(float DeltaTime) const
{
    if (UCableComponent* Cable = OwnerCharacter->GetCable())
        Cable->SetWorldLocation(FMath::VInterpTo(Cable->GetComponentLocation(), GrappleLocation, DeltaTime, CableInterpSpeed));
}

void UGrapplingHookComponent::ReleaseGrapple()
{
    if (!bIsGrappling) return;
    if (GrapplePullAudioComponent && GrapplePullAudioComponent->IsPlaying())
        GrapplePullAudioComponent->Stop();
    OwnerCharacter->GetCapsuleComponent()->SetCapsuleHalfHeight(OriginalCapsuleHalfHeight);
    bIsGrappling = false;
    MovementComponent->SetMovementMode(MOVE_Walking);
    
    if (UCableComponent* Cable = OwnerCharacter->GetCable())
        Cable->SetVisibility(false);
}

void UGrapplingHookComponent::ClimbAtEnd() //similar to vault mantling
{
    if(!ValidateLandingSpace(GrappleLocation))
    {
        ReleaseGrapple();
        return;
    }
    
    bIsMantling = true;
    MantleAlpha = 0.0f;
    MantleStartLocation = OwnerCharacter->GetActorLocation();
    
    const FVector ToGrapplePoint = (GrappleLocation - MantleStartLocation).GetSafeNormal();
    const float CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
    
    // Position the target forward from the grapple point
    MantleTargetLocation = GrappleLocation + (ToGrapplePoint * (CapsuleRadius + 40.0f));
    MantleTargetLocation.Z += OriginalCapsuleHalfHeight + 10.0f;
    
    if (MovementComponent)
    {
        MovementComponent->SetMovementMode(MOVE_None);
        MovementComponent->Velocity = FVector::ZeroVector;
    }
    
    ReleaseGrapple();
}

bool UGrapplingHookComponent::ValidateLandingSpace(const FVector& ObstacleTop) const
{
    const float CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
    const FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    const FVector LandingPos = ObstacleTop + ForwardVector * (CapsuleRadius + 20.0f) + FVector(0, 0, OriginalCapsuleHalfHeight);
    
    FCollisionQueryParams TraceParams;
    TraceParams.AddIgnoredActor(OwnerCharacter);
    const bool bHasSpace = !GetWorld()->OverlapAnyTestByChannel(
        LandingPos, 
        FQuat::Identity, 
        ECC_WorldStatic, 
        FCollisionShape::MakeCapsule(CapsuleRadius * 0.85f, OriginalCapsuleHalfHeight * 0.9f), 
        TraceParams
    );
    
    // DrawDebugCapsule(GetWorld(), LandingPos, CapsuleHalfHeight, CapsuleRadius, FQuat::Identity, bHasSpace ? FColor::Green : FColor::Red, false, 1.5f);
    return bHasSpace;
}

void UGrapplingHookComponent::UpdateMantle(const float DeltaTime)
{
    if (!bIsMantling) return;
    
    MantleAlpha += DeltaTime / MantleDuration;
    if (MantleAlpha >= 1.0f)
    {
        bIsMantling = false;
        OwnerCharacter->SetActorLocation(MantleTargetLocation);
        if (MovementComponent)
            MovementComponent->SetMovementMode(MOVE_Walking);
        return;
    }
    
    const FVector CurrentLocation = FMath::Lerp(MantleStartLocation, MantleTargetLocation, MantleAlpha);
    OwnerCharacter->SetActorLocation(CurrentLocation);
}

float UGrapplingHookComponent::CalculatePullStrength(const FVector& ToTarget) const
{
    float PullStrength = BasePullStrength;
    
    if (ToTarget.Z <= 0.0f)
        PullStrength *= DownwardPullMultiplier;
    
    // closer targets get stronger pulls
    const float DistanceMultiplier = FMath::Clamp(Distance / DistanceScaleReference, MinDistanceMultiplier, MaxDistanceMultiplier);
    PullStrength *= DistanceMultiplier;
    
    return PullStrength;
}

bool UGrapplingHookComponent::ShouldApplyAntiGravity(const FVector& ToTarget) const
{
    return FMath::Abs(ToTarget.Z) < HorizontalGrappleThreshold && Distance > MinHorizontalDistanceForAntiGravity;
}
