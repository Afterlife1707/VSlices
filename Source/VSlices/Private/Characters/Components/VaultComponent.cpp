#include "Characters/Components/VaultComponent.h"
#include "Characters/VSlicesCharacter.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

UVaultComponent::UVaultComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;
    
    TraceParams.AddIgnoredActor(nullptr); 
    ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
}

void UVaultComponent::BeginPlay()
{
    Super::BeginPlay();
    
    TraceParams.AddIgnoredActor(OwnerCharacter);
    CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
    CapsuleHalfHeight = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

void UVaultComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsVaulting || !OwnerCharacter) 
        return;

    VaultLerpAlpha = FMath::Clamp(VaultLerpAlpha + DeltaTime / VaultLerpTime, 0.f, 1.f);
    
    if(CurrentVaultType==EVaultType::Vault_Short || CurrentVaultType==EVaultType::Vault_Tall)
        UpdateVaultMotion(DeltaTime);
    else if(CurrentVaultType==EVaultType::Climb_Short || CurrentVaultType==EVaultType::Climb_Tall)
        UpdateClimbMotion(DeltaTime);
}

void UVaultComponent::UpdateVaultMotion(const float DeltaTime) const
{
    const FVector HorizontalPos = FMath::Lerp(VaultStartLocation, VaultTargetLocation, VaultLerpAlpha);
    const float ArcHeight = FMath::Lerp(VaultStartLocation.Z, VaultTargetLocation.Z, VaultLerpAlpha) + CalculateArcOffset();
    const FVector TargetLocation(HorizontalPos.X, HorizontalPos.Y, ArcHeight);
    
    // Final position precision
    if (VaultLerpAlpha >= 0.9f)
    {
        // Smoothly move to exact final position instead of using velocity
        const FVector CurrentLocation = OwnerCharacter->GetActorLocation();
        const FVector FinalLocation = FMath::VInterpTo(CurrentLocation, VaultTargetLocation, DeltaTime, 15.f);
        OwnerCharacter->SetActorLocation(FinalLocation);
        MovementComponent->Velocity = FVector::ZeroVector;
    }
    else
    {
        // Use velocity-based movement
        const FVector CurrentLocation = OwnerCharacter->GetActorLocation();
        const FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
        const float Distance = FVector::Dist(CurrentLocation, TargetLocation);
        const float Speed = Distance / (VaultLerpTime * (1.0f - VaultLerpAlpha + 0.01f));
        
        MovementComponent->Velocity = Direction * FMath::Min(Speed, 1000.f);
    }
    
    // Handle rotation
    const FRotator NewRotation = FMath::RInterpTo(OwnerCharacter->GetActorRotation(), VaultTargetRotation, DeltaTime, 15.f);
    OwnerCharacter->SetActorRotation(NewRotation);
}

void UVaultComponent::UpdateClimbMotion(const float DeltaTime) const
{
    MovementComponent->Velocity = FVector::ZeroVector;
    
    // Early termination for final position
    if (VaultLerpAlpha >= 0.9f)
    {
        OwnerCharacter->SetActorLocationAndRotation(VaultTargetLocation, VaultTargetRotation, false, nullptr, ETeleportType::TeleportPhysics);
        return;
    }
    
    // Calculate horizontal movement with accelerated curve near end
    const float HorizontalLerpAmount = (VaultLerpAlpha > 0.6f) ? FMath::Lerp(0.3f, 0.9f, (VaultLerpAlpha - 0.6f) / 0.3f) : VaultLerpAlpha * 0.3f;
    
    const FVector StartToTarget = VaultTargetLocation - VaultStartLocation;
    const FVector HorizontalOffset = FVector(StartToTarget.X, StartToTarget.Y, 0) * HorizontalLerpAmount;
    const FVector CurrentLocation = OwnerCharacter->GetActorLocation();
    const FVector NewLocation = VaultStartLocation + HorizontalOffset + FVector(0, 0, CurrentLocation.Z - VaultStartLocation.Z);
    
    const FRotator NewRotation = FMath::RInterpTo(VaultStartRotation, VaultTargetRotation, DeltaTime, 8.f);
    OwnerCharacter->SetActorLocationAndRotation(NewLocation, NewRotation, false, nullptr, ETeleportType::TeleportPhysics);
}

float UVaultComponent::CalculateArcOffset() const
{
    const float MaxHeight = FMath::Max(VaultStartLocation.Z, VaultTargetLocation.Z);
    const float ArcContribution = VaultArcPeak - MaxHeight;
    return 4.0f * ArcContribution * VaultLerpAlpha * (1.0f - VaultLerpAlpha);
}

bool UVaultComponent::TryVault(const bool bWasSprinting)
{
    if (!OwnerCharacter || bIsVaulting || !MovementComponent->IsMovingOnGround())
        return false;
        
    FVaultableObstacle Obstacle;
    return FindVaultableObstacle(Obstacle, bWasSprinting) && ExecuteVault(Obstacle);
}

bool UVaultComponent::FindVaultableObstacle(FVaultableObstacle& OutObstacle, const bool bWasSprinting) const
{
    const FVector PlayerLocation = OwnerCharacter->GetActorLocation();
    const FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    const float CurrentTraceDistance = TraceDistance * (bWasSprinting ? 3.0f : 1.0f);
    
    // Find closest obstacle from multi-height traces
    FHitResult BestHit;
    float ClosestDistance = MAX_FLT;
    
    constexpr int32 NumTraces = 5;
    for (int32 i = 0; i < NumTraces; i++)
    {
        const float HeightOffset = FMath::Lerp(MinTraceHeight, MaxTraceHeight, i / static_cast<float>(NumTraces - 1));
        const FVector Start = PlayerLocation + FVector(0, 0, HeightOffset);
        const FVector End = Start + ForwardVector * CurrentTraceDistance;
        
        FHitResult Hit;
        if (PerformTrace(Hit, Start, End))
        {
            if (const float Distance = FVector::DistSquared(PlayerLocation, Hit.Location); Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                BestHit = Hit;
            }
        }
        //DrawTraceDebug(Start, End, Hit.bBlockingHit, Hit.Location);
    }
    return BestHit.bBlockingHit && AnalyzeObstacle(BestHit, OutObstacle);
}

bool UVaultComponent::AnalyzeObstacle(const FHitResult& Hit, FVaultableObstacle& OutObstacle) const
{
    const bool bIsWall = Hit.Normal.Z < 0.5f;
    FVector ObstacleTop;
    float ObstacleHeight;
    
    if (bIsWall)
    {
        if (!FindWallTop(Hit, ObstacleTop, ObstacleHeight))
            return false;
    }
    else
    {
        ObstacleTop = Hit.Location;
        ObstacleHeight = ObstacleTop.Z - OwnerCharacter->GetActorLocation().Z;
    }
    ObstacleHeight = FMath::Abs(ObstacleHeight);
    if (ObstacleHeight < MinHeightForShortVault || ObstacleHeight > MaxHeightForTraverse)
    {
        LOG_INFO("Obstacle height %.2f outside valid range [%.2f - %.2f]", ObstacleHeight, MinHeightForShortVault, MaxHeightForTraverse);
        return false;
    }
    
    LOG_INFO("Obstacle height %.2f ", ObstacleHeight);
    if (!ValidateLandingSpace(ObstacleTop))
    {
        LOG_INFO("Failed landing space validation");
        return false;
    }
    
    OutObstacle = FVaultableObstacle{
        .TopLocation = ObstacleTop,
        .Normal = Hit.Normal,
        .Height = ObstacleHeight,
        .bIsWall = bIsWall,
        .bIsThick = bIsWall ? IsObstacleThick(Hit, ObstacleTop) : false
    };
    
   // DrawDebugSphere(GetWorld(), ObstacleTop, 12.0f, 12, FColor::Purple, false, 2.0f);
    LOG_INFO("Found %s obstacle - Height: %.2f, Thick: %s", bIsWall ? TEXT("Wall") : TEXT("Platform"), ObstacleHeight, OutObstacle.bIsThick ? TEXT("Yes") : TEXT("No"));
    
    return true;
}

bool UVaultComponent::FindWallTop(const FHitResult& WallHit, FVector& OutWallTop, float& OutHeight) const
{
    const FVector Start = WallHit.Location + FVector(0, 0, 200.0f) - WallHit.Normal * 10.0f;
    const FVector End = Start - FVector(0, 0, 300.0f);
    
    FHitResult TopHit;
    if (!PerformTrace(TopHit, Start, End))
        return false;
    
    OutWallTop = TopHit.Location;
    OutHeight = OutWallTop.Z - OwnerCharacter->GetActorLocation().Z;
    return true;
}

bool UVaultComponent::ValidateLandingSpace(const FVector& ObstacleTop) const
{
    const FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    const FVector LandingPos = ObstacleTop + ForwardVector * (CapsuleRadius + 20.0f) + FVector(0, 0, CapsuleHalfHeight);
    
    const bool bHasSpace = !GetWorld()->OverlapAnyTestByChannel(
        LandingPos, 
        FQuat::Identity, 
        ECC_WorldStatic, 
        FCollisionShape::MakeCapsule(CapsuleRadius * 0.85f, CapsuleHalfHeight * 0.9f), 
        TraceParams
    );
    
   // DrawDebugCapsule(GetWorld(), LandingPos, CapsuleHalfHeight, CapsuleRadius, FQuat::Identity, bHasSpace ? FColor::Green : FColor::Red, false, 1.5f);
    return bHasSpace;
}

bool UVaultComponent::ExecuteVault(const FVaultableObstacle& Obstacle)
{
    EVaultType VaultType;
    FVector VaultTarget;
    
    if (Obstacle.bIsWall && !Obstacle.bIsThick)
    {
        // Traditional vault over wall
        VaultType = (Obstacle.Height <= MaxHeightForShortVault) ? EVaultType::Vault_Short : EVaultType::Vault_Tall;
        VaultTarget = Obstacle.TopLocation - Obstacle.Normal * (CapsuleRadius + 80.0f);
        VaultArcPeak = FMath::Max(Obstacle.TopLocation.Z + 50.0f, VaultTarget.Z + 30.0f);
    }
    else
    {
        // Climb wall or mantle platform
        VaultType = (Obstacle.Height <= MaxHeightForShortVault) ? EVaultType::Climb_Short : EVaultType::Climb_Tall;
        
        const FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
        VaultTarget = Obstacle.TopLocation + ForwardVector * (CapsuleRadius + 40.0f);
        VaultTarget.Z = Obstacle.TopLocation.Z + CapsuleHalfHeight + 5.0f;
        VaultArcPeak = Obstacle.TopLocation.Z + 30.0f;
    }
    
    const FVector DirectionToTarget = (VaultTarget - OwnerCharacter->GetActorLocation()).GetSafeNormal2D();
    StartVault(VaultType, VaultTarget, DirectionToTarget.Rotation());
    return true;
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
    VaultLerpAlpha = 0.f;

    OwnerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MovementComponent->SetMovementMode(MOVE_Flying);
    MovementComponent->Velocity = FVector::ZeroVector;
    
    // Get animation montage and set lerp time
    if (UAnimMontage* MontageToPlay = GetVaultMontage(VaultType))
    {
        VaultLerpTime = OwnerCharacter->PlayAnimMontage(MontageToPlay);
        if (VaultLerpTime <= 0.f) VaultLerpTime = 1.f;
    }
}

void UVaultComponent::FinishVault()
{
    if (!OwnerCharacter) return;
    
    bIsVaulting = false;
    VaultLerpAlpha = 0.f;
    
    OwnerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MovementComponent->SetMovementMode(MOVE_Walking);
}

bool UVaultComponent::IsObstacleThick(const FHitResult& Hit, const FVector& WallTop) const
{
    const FVector Start = WallTop + FVector(0, 0, 50.0f) - Hit.Normal * ThicknessForClimb;
    const FVector End = Start - FVector(0, 0, 100.0f);
    
    FHitResult ThicknessHit;
    const bool bIsThick = PerformTrace(ThicknessHit, Start, End);
    
   // DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 2.0f, 0, 2.0f);
   // if (bIsThick) DrawDebugSphere(GetWorld(), ThicknessHit.Location, 12.0f, 12, FColor::Green, false, 2.0f);
    
    return bIsThick;
}

UAnimMontage* UVaultComponent::GetVaultMontage(const EVaultType VaultType) const
{
    switch (VaultType)
    {
        case EVaultType::Vault_Short: return VaultShortMontage;
        case EVaultType::Vault_Tall:  return VaultTallMontage;
        case EVaultType::Climb_Short: return ClimbShortMontage;
        case EVaultType::Climb_Tall:  return ClimbTallMontage;
        default: return nullptr;
    }
}

void UVaultComponent::DrawTraceDebug(const FVector& Start, const FVector& End, const bool bHit, const FVector& HitLocation) const
{
    DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Red : FColor::Green, false, 5.0f);
    if (bHit) DrawDebugSphere(GetWorld(), HitLocation, 5.0f, 8, FColor::Blue, false, 1.0f);
}

bool UVaultComponent::PerformTrace(FHitResult& OutHit, const FVector& Start, const FVector& End) const
{
    return GetWorld()->LineTraceSingleByObjectType(OutHit, Start, End, ObjectParams, TraceParams);
}