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

    if(MovementComponent->Velocity.Z<0) 
        TryGrab();
}

void ULedgeComponent::TryGrab() const
{
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);
    
    // Start trace from near the top of the character's head
    FVector CharacterTop = OwnerCharacter->GetActorLocation() + FVector(0, 0, CapsuleHalfHeight);
    FVector Start = CharacterTop + OwnerCharacter->GetActorForwardVector() * ForwardTraceDistance;
    FVector End = Start - FVector(0, 0, DownwardTraceDistance);
    FHitResult Hit;
    
    // First trace: Check for ledge edge (downward from in front of character)
    if(bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, QueryParams))
    {
        // Second trace: Check for wall surface (horizontal towards wall)
        Start = CharacterTop;
        End = Hit.Location - FVector(0, 0, LedgeHeightOffset);
        Start.Z = End.Z;
        bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, QueryParams);
        
        if(bHit)
        {
            //hang from ledge
            FVector Location = Hit.Location + Hit.ImpactNormal * WallOffsetDistance;
            Location.Z = Hit.Location.Z - (CapsuleHalfHeight * HangHeightMultiplier);
            FRotator Rotation = UKismetMathLibrary::MakeRotFromX(-Hit.ImpactNormal);
            OwnerCharacter->SetActorLocationAndRotation(Location, Rotation);
            MovementComponent->StopMovementImmediately();
            
            SetGrab(true);
            LOG_INFO("Ledge grab successful!");
        }
        else
        {
            LOG_WARNING("Second trace failed - no wall surface detected");
        }
    }
    else
    {
        LOG_WARNING("First trace failed - no ledge edge detected");
    }
}

void ULedgeComponent::OnJump() const
{
    SetGrab(false);
    MovementComponent->Velocity = FVector(0.0, CapsuleRadius+5.f, MovementComponent->JumpZVelocity * 1.5f);
}

void ULedgeComponent::SetGrab(const bool bGrab) const
{
    MovementComponent->GravityScale = bGrab ? 0.0f:OriginalGravityScale;
    OwnerCharacter->SetGrabLedge(bGrab);
    OwnerCharacter->bUseControllerRotationYaw = !bGrab;
    const auto Type = bGrab ? ECollisionEnabled::Type::NoCollision:ECollisionEnabled::Type::QueryAndPhysics;
    OwnerCharacter->GetCapsuleComponent()->SetCollisionEnabled(Type);
}
