#include "Characters/Components/LedgeComponent.h"
#include "Characters/VSlicesCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"

ULedgeComponent::ULedgeComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
}

void ULedgeComponent::BeginPlay()
{
    Super::BeginPlay();

    OriginalGravityScale = MovementComponent->GravityScale;
    CapsuleHalfHeight = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
}

void ULedgeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if(!OwnerCharacter->GetLedgeGrab() && MovementComponent->Velocity.Z<0) 
        TryGrab();
}

void ULedgeComponent::TryGrab() const
{
    FCollisionQueryParams QueryParams;
    FCollisionObjectQueryParams ObjectQueryParams;
    ObjectQueryParams.AddObjectTypesToQuery(ECC_GameTraceChannel1); //ledge channel, BP_Ledge's object type
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    // Start trace from near the top of the character's head and slightly above
    const FVector CharacterTop = OwnerCharacter->GetActorLocation() + FVector(0, 0, CapsuleHalfHeight * 1.5f);
    FVector Start = CharacterTop + OwnerCharacter->GetActorForwardVector() * ForwardTraceDistance;
    FVector End = Start - FVector(0, 0, DownwardTraceDistance);
    FHitResult Hit;
    // First trace: Check for ledge edge (downward from in front of character)
    if(bool bHit = GetWorld()->LineTraceSingleByObjectType(Hit, Start, End, ObjectQueryParams, QueryParams))
    {
        // Second trace: Check for wall surface (horizontal towards wall)
        Start = CharacterTop;
        End = Hit.Location - FVector(0, 0, LedgeHeightOffset);
        Start.Z = End.Z;
        bHit = GetWorld()->LineTraceSingleByObjectType(Hit, Start, End, ObjectQueryParams, QueryParams);
        
        if(bHit)
        {
            //hang from ledge
            FVector Location = Hit.Location + Hit.ImpactNormal * WallOffsetDistance;
            Location.Z = Hit.Location.Z - (CapsuleHalfHeight * HangHeightMultiplier);
            const FRotator Rotation = UKismetMathLibrary::MakeRotFromX(-Hit.ImpactNormal);
            OwnerCharacter->SetActorLocationAndRotation(Location, Rotation);
            MovementComponent->StopMovementImmediately();
            
            SetGrab(true);
        }
    }
}

void ULedgeComponent::OnJump()
{
    MovementComponent->Velocity = FVector(0.0, 0.0, MovementComponent->JumpZVelocity/2);
    OwnerCharacter->SetLedgeGrab(false);
    GetWorld()->GetTimerManager().SetTimer(JumpTimer, this, &ULedgeComponent::DisableGrab, 0.5f, false);
}

void ULedgeComponent::DisableGrab() const
{
    SetGrab(false);
}

void ULedgeComponent::SetGrab(const bool bGrab) const
{
    OwnerCharacter->SetActorEnableCollision(!bGrab);
    OwnerCharacter->GetCapsuleComponent()->SetCapsuleRadius(bGrab ? CapsuleRadius/2:CapsuleRadius);
    MovementComponent->GravityScale = bGrab ? 0.0f:OriginalGravityScale;
    OwnerCharacter->SetLedgeGrab(bGrab);
    OwnerCharacter->bUseControllerRotationYaw = !bGrab;
    const auto Type = bGrab ? ECollisionEnabled::Type::NoCollision:ECollisionEnabled::Type::QueryAndPhysics;
    OwnerCharacter->GetCapsuleComponent()->SetCollisionEnabled(Type);
}
