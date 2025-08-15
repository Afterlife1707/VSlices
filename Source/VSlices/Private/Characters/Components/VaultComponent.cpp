#include "Characters/Components/VaultComponent.h"
#include "LoggingMacros.h"
#include "Characters/VSlicesCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

UVaultComponent::UVaultComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
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

void UVaultComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!bIsVaulting || !OwnerCharacter) return;

    VaultLerpAlpha += DeltaTime / VaultLerpTime;
    VaultLerpAlpha = FMath::Clamp(VaultLerpAlpha, 0.f, 1.f);

    FVector NewLocation;
    
    // Check if this is a climbing action
    if (CurrentVaultType == EVaultType::Climb_Short || CurrentVaultType == EVaultType::Climb_Tall)
    {
        // For climbing: only handle horizontal movement, let animation handle vertical
        const FVector HorizontalStart = FVector(VaultStartLocation.X, VaultStartLocation.Y, 0);
        const FVector HorizontalTarget = FVector(VaultTargetLocation.X, VaultTargetLocation.Y, 0);
        const FVector HorizontalPos = FMath::Lerp(HorizontalStart, HorizontalTarget, VaultLerpAlpha);
        
        // Use current Z from animation, but ensure we don't go below the target at the end
        float FinalZ = OwnerCharacter->GetActorLocation().Z;
        
        // Near the end of the animation, blend toward target height to ensure we reach it
        if (VaultLerpAlpha > 0.8f)
        {
            const float EndBlendAlpha = (VaultLerpAlpha - 0.8f) / 0.2f; // 0 to 1 over last 20%
            const float TargetZ = VaultTargetLocation.Z+20.f;
            FinalZ = FMath::Lerp(FinalZ, TargetZ, EndBlendAlpha * 0.5f); // Gentle blend
        }
        
        NewLocation = FVector(HorizontalPos.X, HorizontalPos.Y, FinalZ);
    }
    else // Vaulting
    {
        // Use arc trajectory to go over the obstacle
        const FVector HorizontalPos = FMath::Lerp(VaultStartLocation, VaultTargetLocation, VaultLerpAlpha);
        const float ArcHeight = FMath::Lerp(VaultStartLocation.Z, VaultTargetLocation.Z, VaultLerpAlpha) + 
                               (4.0f * (VaultArcPeak - FMath::Max(VaultStartLocation.Z, VaultTargetLocation.Z)) * 
                                VaultLerpAlpha * (1.0f - VaultLerpAlpha));
        
        NewLocation = FVector(HorizontalPos.X, HorizontalPos.Y, ArcHeight);
    }

    const FRotator NewRotation = FMath::RInterpTo(VaultStartRotation, VaultTargetRotation, DeltaTime, 15.f);
    OwnerCharacter->SetActorLocationAndRotation(NewLocation, NewRotation, false, nullptr, ETeleportType::TeleportPhysics);
}

bool UVaultComponent::TryVault()
{
    if (!OwnerCharacter || bIsVaulting || !OwnerCharacter->GetCharacterMovement()->IsMovingOnGround())
        return false;

    FTraceResult ForwardHit;
    if (!ForwardTrace(ForwardHit)) return false;
    
    FVector WallTop;
    float WallHeight;
    if (!HeightTrace(ForwardHit, WallTop, WallHeight)) return false;
    
    EVaultType VaultType;
    FVector VaultTarget;
    const float CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
    const float CapsuleHalfHeight = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    
    if (!ThicknessTrace(ForwardHit, WallTop)) // Vault
    {
        VaultType = (WallHeight <= MaxHeightForShortVault) ? EVaultType::Vault_Short : EVaultType::Vault_Tall;
        VaultTarget = WallTop + (-ForwardHit.Normal) * (CapsuleRadius + 80.0f);
    }
    else // Climb
    {
        VaultType = (WallHeight <= MaxHeightForShortVault) ? EVaultType::Climb_Short : EVaultType::Climb_Tall;
        VaultTarget = WallTop + FVector(0, 0, CapsuleHalfHeight + 10.0f);
    }

    // Simple arc peak: wall height + extra clearance
    VaultArcPeak = FMath::Max(WallTop.Z + 50.0f, VaultTarget.Z + 30.0f);

    FVector DirectionToTarget = (VaultTarget - OwnerCharacter->GetActorLocation()).GetSafeNormal();
    DirectionToTarget.Z = 0.f;
    const FRotator VaultRotation = DirectionToTarget.Rotation();

    StartVault(VaultType, VaultTarget, VaultRotation);
    return true;
}

bool UVaultComponent::ForwardTrace(FTraceResult& OutHitResult) const
{
    const FVector Start = OwnerCharacter->GetActorLocation() - FVector(0, 0, 10);
    const FVector End = Start + OwnerCharacter->GetActorForwardVector() * 120.f;

    DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.0f, 0, 2.0f);

    FHitResult Hit;
    if (!PerformTrace(Hit, Start, End))
    {
        LOG_INFO("Forward trace miss");
        DrawDebugSphere(GetWorld(), End, 10.0f, 12, FColor::Orange, false, 2.0f);
        return false;
    }

    OutHitResult.Location = Hit.Location;
    OutHitResult.Normal = Hit.Normal;
    return true;
}

bool UVaultComponent::HeightTrace(const FTraceResult& ForwardHit, FVector& OutWallTop, float& OutWallHeight) const
{
    const FVector Start2 = ForwardHit.Location + FVector(0, 0, 200.0f) - ForwardHit.Normal * 10.0f;
    const FVector End2 = Start2 - FVector(0, 0, 400.0f);

    DrawDebugLine(GetWorld(), Start2, End2, FColor::Yellow, false, 2.0f, 0, 2.0f);

    FHitResult HeightHit;
    if (!PerformTrace(HeightHit, Start2, End2))
    {
        LOG_INFO("Height trace miss");
        return false;
    }

    OutWallTop = HeightHit.Location;
    OutWallHeight = OutWallTop.Z - OwnerCharacter->GetActorLocation().Z;

    DrawDebugSphere(GetWorld(), OutWallTop, 12.0f, 12, FColor::Purple, false, 2.0f);
    LOG_INFO("Wall height: %.2f", OutWallHeight);

    return true;
}

bool UVaultComponent::ThicknessTrace(const FTraceResult& ForwardHit, const FVector& WallTop) const
{
    const FVector Start = WallTop + FVector(0, 0, 50.0f) - ForwardHit.Normal * ThicknessForClimb;
    const FVector End = Start - FVector(0, 0, 100.0f);

    DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 2.0f, 0, 2.0f);

    FHitResult ThicknessHit;
    if (PerformTrace(ThicknessHit, Start, End))
    {
        DrawDebugSphere(GetWorld(), ThicknessHit.Location, 12.0f, 12, FColor::Green, false, 2.0f);
        LOG_INFO("Wall is thick - will climb");
        return true; 
    }
    LOG_INFO("Wall is thin - will vault");
    return false;
}

void UVaultComponent::StartVault(const EVaultType VaultType, const FVector& TargetLocation, const FRotator& TargetRotation)
{
    if (!OwnerCharacter) return;

    bIsVaulting = true;
    CurrentVaultType = VaultType;
    VaultStartLocation = OwnerCharacter->GetActorLocation();
    VaultTargetLocation = TargetLocation;
    VaultStartRotation = OwnerCharacter->GetActorRotation();
    VaultTargetRotation = TargetRotation;

    OwnerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MovementComp->SetMovementMode(MOVE_Flying);
    MovementComp->Velocity = FVector::ZeroVector;
    
    // Play animation
    UAnimMontage* MontageToPlay = nullptr;
    switch (VaultType)
    {
        case EVaultType::Vault_Short: MontageToPlay = VaultShortMontage; break;
        case EVaultType::Vault_Tall:  MontageToPlay = VaultTallMontage; break;
        case EVaultType::Climb_Short: MontageToPlay = ClimbShortMontage; break;
        case EVaultType::Climb_Tall:  MontageToPlay = ClimbTallMontage; break;
    }

    if (MontageToPlay)
    {
        VaultLerpTime = OwnerCharacter->PlayAnimMontage(MontageToPlay);
        if (VaultLerpTime <= 0.f) VaultLerpTime = 1.f;
    }

    VaultLerpAlpha = 0.f;

    
    LOG_INFO("Starting vault from %s to %s", 
        *VaultStartLocation.ToString(), 
        *VaultTargetLocation.ToString());
}

void UVaultComponent::FinishVault()
{
    if (!OwnerCharacter) return;

    bIsVaulting = false;
    
    OwnerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MovementComp->SetMovementMode(MOVE_Walking);
    MovementComp->Velocity = FVector::ZeroVector;
    
    VaultLerpAlpha = 0.f;
    
    OwnerCharacter->SetActorLocationAndRotation(VaultTargetLocation, VaultTargetRotation, false, nullptr, ETeleportType::TeleportPhysics);
    
    // Optional: Do a ground check to ensure proper landing for climbing
    if (CurrentVaultType == EVaultType::Climb_Short || CurrentVaultType == EVaultType::Climb_Tall)
    {
        FVector FloorLocation = VaultTargetLocation;
        FHitResult FloorHit;
        if (GetWorld()->LineTraceSingleByChannel(FloorHit, FloorLocation, FloorLocation - FVector(0, 0, 200), ECC_WorldStatic))
        {
            // Make sure we're standing on the surface we climbed onto
            FloorLocation.Z = FMath::Max(FloorHit.Location.Z + OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), 
                                       VaultTargetLocation.Z);
            OwnerCharacter->SetActorLocation(FloorLocation);
        }
    }
}

bool UVaultComponent::PerformTrace(FHitResult& OutHit, const FVector& Start, const FVector& End) const
{
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    FCollisionObjectQueryParams ObjParams;
    ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);

    return GetWorld()->LineTraceSingleByObjectType(OutHit, Start, End, ObjParams, Params);
}
