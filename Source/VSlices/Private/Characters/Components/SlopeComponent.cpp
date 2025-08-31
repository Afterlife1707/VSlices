// Copyright Epic Games, Inc. All Rights Reserved.

#include "Characters/Components/SlopeComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Characters/VSlicesCharacter.h" 
#include "Logging/LogMacros.h"

USlopeComponent::USlopeComponent()
{
}

void USlopeComponent::ApplySlopeRestrictions(FVector2D& MovementVector)
{
    // Get fresh slope info
    const FSlopeInfo CurrentSlopeInfo = GetSlopeInfo();
    
    if (!CurrentSlopeInfo.bIsOnSlope) 
    {
        //LOG_INFO("Not on slope - no restrictions");
        return;
    }
    
    //LOG_INFO("On slope: %.1f degrees, Uphill: %s, Downhill: %s", CurrentSlopeInfo.SlopeAngle, CurrentSlopeInfo.bIsUphill ? TEXT("Yes") : TEXT("No"), CurrentSlopeInfo.bIsDownhill ? TEXT("Yes") : TEXT("No"));
    
    const bool bMovingForward = MovementVector.Y > 0.001f;
    const bool bMovingBackward = MovementVector.Y < -0.001f;
    
    if (bMovingForward && CurrentSlopeInfo.bIsUphill)
    {
        if (CurrentSlopeInfo.SlopeAngle > MaxWalkableUphillAngle)
        {
            LOG_WARNING("Blocking uphill movement - too steep: %.1f degrees", CurrentSlopeInfo.SlopeAngle);
            MovementVector.Y = 0.0f; 
        }
        else if (CurrentSlopeInfo.SlopeAngle > MinSlopeSpeedDecreaseAngle)
        {
            const float SpeedMultiplier = OwnerCharacter->GetIsSprinting() ? 0.75f : 0.5f;
            //LOG_INFO("Reducing uphill speed by %.0f%% (was sprinting: %s)", (1.0f - SpeedMultiplier) * 100.0f,OwnerCharacter->GetIsSprinting() ? TEXT("Yes") : TEXT("No"));
            MovementVector.Y *= SpeedMultiplier;
        }
    }
    
    if (bMovingBackward && CurrentSlopeInfo.bIsDownhill && CurrentSlopeInfo.SlopeAngle > MaxWalkableDownhillAngle)
        MovementVector.Y = 0.0f;
}

FSlopeInfo USlopeComponent::GetSlopeInfo() 
{
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastSlopeUpdateTime >= SlopeUpdateInterval)
    {
        UpdateSlopeInfo();
        LastSlopeUpdateTime = CurrentTime;
    }
    return CachedSlopeInfo;
}

void USlopeComponent::UpdateSlopeInfo()
{
    // Reset slope info
    CachedSlopeInfo = FSlopeInfo();
    
    if (!MovementComponent || !OwnerCharacter)
    {
        LOG_WARNING("Missing MovementComponent or OwnerCharacter");
        return;
    }
    
    const FHitResult& FloorHit = MovementComponent->CurrentFloor.HitResult;
    if (!FloorHit.IsValidBlockingHit()) 
    {
       // LOG_INFO("No valid floor hit");
        return;
    }
    
    const FVector FloorNormal = FloorHit.ImpactNormal;
    const float FloorDotUp = FVector::DotProduct(FloorNormal, FVector::UpVector);
    CachedSlopeInfo.SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FloorDotUp, 0.0f, 1.0f)));
    
    //LOG_INFO("Floor angle: %.1f degrees", CachedSlopeInfo.SlopeAngle);
    
    if (CachedSlopeInfo.SlopeAngle <= MinSlopeAngle) 
    {
       // LOG_INFO("Slope too shallow (%.1f <= %.1f)", CachedSlopeInfo.SlopeAngle, MinSlopeAngle);
        return;
    }
    
    CachedSlopeInfo.bIsOnSlope = true;
    
    const FVector SlopeDirection = FVector::CrossProduct(FloorNormal, 
        FVector::CrossProduct(FloorNormal, FVector::UpVector)).GetSafeNormal();
    CachedSlopeInfo.FacingAlignment = FVector::DotProduct(OwnerCharacter->GetActorForwardVector(), SlopeDirection);
    
    // Determine uphill/downhill
    CachedSlopeInfo.bIsUphill = CachedSlopeInfo.FacingAlignment < -UphillThreshold;
    CachedSlopeInfo.bIsDownhill = CachedSlopeInfo.FacingAlignment > DownhillThreshold;
}