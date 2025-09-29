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
}

void ULedgeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if(MovementComponent->Velocity.Z>=0) return;
    
    // Create collision query params to ignore the character
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    const float CapsuleHalfHeight = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    const float HeadHeight = CapsuleHalfHeight * 0.9f; // Near the top of the head
    
    // Start trace from near the top of the character's head
    FVector CharacterTop = OwnerCharacter->GetActorLocation() + FVector(0, 0, HeadHeight);
    FVector Start = CharacterTop + OwnerCharacter->GetActorForwardVector() * 50.f;
    FVector End = Start - FVector(0, 0, 50.f);
    FHitResult Hit;
    
    // First trace: Check for ledge edge (downward from in front of character)
    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, QueryParams);
    DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Green : FColor::Red, false, 0.1f, 0, 2.f);
    if(bHit)
    {
        OwnerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
        DrawDebugSphere(GetWorld(), Hit.Location, 10.f, 12, FColor::Yellow, false, 0.1f);
        
        // Store the ledge top Z position
        float LedgeTopZ = Hit.Location.Z;
        
        // Second trace: Check for wall surface (horizontal towards wall)
        Start = CharacterTop;
        End = Hit.Location - FVector(0, 0, 10);
        Start.Z = End.Z; // Make the trace horizontal at ledge height
        
        bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, QueryParams);
        DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Green : FColor::Red, false, 0.1f, 0, 2.f);
        
        if(bHit)
        {
            DrawDebugSphere(GetWorld(), Hit.Location, 10.f, 12, FColor::Cyan, false, 0.1f);
            DrawDebugDirectionalArrow(GetWorld(), Hit.Location, Hit.Location + Hit.ImpactNormal * 50.f, 5.f, FColor::Magenta, false, 0.1f, 0, 2.f);
            
            // Position character hanging from the ledge - closer to wall and lower
            FVector Location = Hit.Location + Hit.ImpactNormal * -25; // Minimal offset from wall
            Location.Z = LedgeTopZ - (CapsuleHalfHeight * 1.6f); // Hang lower 
            
            FRotator Rotation = UKismetMathLibrary::MakeRotFromX(-Hit.ImpactNormal);
            
            OwnerCharacter->SetActorLocationAndRotation(Location, Rotation);
            MovementComponent->GravityScale = 0.f;
            MovementComponent->StopMovementImmediately();
            OwnerCharacter->SetGrabLedge(true);
            OwnerCharacter->bUseControllerRotationYaw = false;
            
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

void ULedgeComponent::OnJump()
{
    MovementComponent->GravityScale = OriginalGravityScale;
    OwnerCharacter->SetGrabLedge(false);
    MovementComponent->Velocity = FVector(0.0, 0.0, MovementComponent->JumpZVelocity*1.5f);
    OwnerCharacter->bUseControllerRotationYaw = true;
    OwnerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
}

