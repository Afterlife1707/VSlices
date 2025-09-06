#include "Characters/Components/LedgeSwingComponent.h"
#include "Characters/VSlicesCharacter.h"
#include "Components/CapsuleComponent.h"

ULedgeSwingComponent::ULedgeSwingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void ULedgeSwingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsHanging) return;
    
    if (CurrentHangType == EHangType::Pole)
        UpdateSwing(DeltaTime);
    
    UpdateHangPosition();
}

bool ULedgeSwingComponent::TryGrab()
{
    if (bIsHanging) 
    {
        LOG_INFO("Already hanging, cannot grab");
        return false;
    }
    
    FVector GrabLocation;
    FVector GrabNormal;
    
    // Try ledge detection first
    if (DetectLedge(GrabLocation, GrabNormal))
    {
        LOG_INFO("Ledge detected at: %s", *GrabLocation.ToString());
        StartHang(GrabLocation, GrabNormal, EHangType::Ledge);
        return true;
    }
    
    // Try pole detection
    if (DetectPole(GrabLocation, GrabNormal))
    {
        LOG_INFO("Pole detected at: %s", *GrabLocation.ToString());
        StartHang(GrabLocation, GrabNormal, EHangType::Pole);
        return true;
    }
    
    LOG_INFO("No grabbable surface found");
    return false;
}

void ULedgeSwingComponent::Jump()
{
    if (!bIsHanging) return;
    
    if (CurrentHangType == EHangType::Pole)
        SwingJump();
    else if (CurrentHangType == EHangType::Ledge)
        MantleUp();
}

void ULedgeSwingComponent::Drop()
{
    if (!bIsHanging) return;
    ReleaseHang();
}

void ULedgeSwingComponent::StartHang(const FVector& Location, const FVector& Normal, EHangType HangType)
{
    bIsHanging = true;
    HangLocation = Location;
    HangNormal = Normal;
    CurrentHangType = HangType;
    
    // Store initial momentum for poles
    if (HangType == EHangType::Pole)
    {
        const FVector CurrentVelocity = OwnerCharacter->GetVelocity();
        const FVector ForwardVelocity = FVector::DotProduct(CurrentVelocity, OwnerCharacter->GetActorForwardVector()) * OwnerCharacter->GetActorForwardVector();
        InitialMomentum = ForwardVelocity.Size();
        SwingVelocity = FMath::Clamp(InitialMomentum * 0.01f, -MaxSwingAngle, MaxSwingAngle);
    }
    
    // Set movement mode and position
    MovementComponent->SetMovementMode(MOVE_None);
    MovementComponent->Velocity = FVector::ZeroVector;
    
    UpdateHangPosition();
    
    LOG_INFO("Started hanging on %s at %s", 
        HangType == EHangType::Pole ? TEXT("Pole") : TEXT("Ledge"),
        *Location.ToString());
}

void ULedgeSwingComponent::UpdateSwing(float DeltaTime)
{
    if (CurrentHangType != EHangType::Pole) return;
    
    // Apply swing physics
    const float GravityEffect = FMath::Sin(FMath::DegreesToRadians(SwingAngle)) * 20.0f;
    SwingVelocity += GravityEffect * DeltaTime;
    SwingAngle += SwingVelocity * DeltaTime;
    
    // Apply decay
    SwingVelocity *= SwingDecay;
    
    // Clamp swing angle
    SwingAngle = FMath::Clamp(SwingAngle, -MaxSwingAngle, MaxSwingAngle);
    
    // Reverse at extremes
    if (FMath::Abs(SwingAngle) >= MaxSwingAngle)
    {
        SwingVelocity *= -0.8f; // Energy loss on swing reversal
    }
}

void ULedgeSwingComponent::UpdateHangPosition()
{
    if (!bIsHanging) return;
    
    FVector TargetLocation = HangLocation;
    
    if (CurrentHangType == EHangType::Pole)
    {
        // Swing position for poles
        const float SwingRadius = 80.0f;
        const FVector SwingOffset = FVector(
            FMath::Sin(FMath::DegreesToRadians(SwingAngle)) * SwingRadius,
            0,
            -SwingRadius
        );
        TargetLocation += SwingOffset;
    }
    else
    {
        // Static hang position for ledges
        const float CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
        TargetLocation -= HangNormal * (CapsuleRadius + 30.0f);
        TargetLocation.Z -= 70.0f; // Hang below ledge
    }
    
    OwnerCharacter->SetActorLocation(TargetLocation);
}

void ULedgeSwingComponent::ReleaseHang()
{
    bIsHanging = false;
    CurrentHangType = EHangType::None;
    SwingAngle = 0.0f;
    SwingVelocity = 0.0f;
    InitialMomentum = 0.0f;
    
    MovementComponent->SetMovementMode(MOVE_Walking);
    
    LOG_INFO("Released hang");
}

bool ULedgeSwingComponent::DetectLedge(FVector& OutLocation, FVector& OutNormal)
{
    const FVector PlayerLocation = OwnerCharacter->GetActorLocation();
    const FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    
    // Forward trace to detect wall
    const FVector ForwardStart = PlayerLocation + FVector(0, 0, 50); // Chest height
    const FVector ForwardEnd = ForwardStart + ForwardVector * ForwardReachDistance;
    
    FCollisionQueryParams TraceParams;
    TraceParams.AddIgnoredActor(OwnerCharacter);
    
    FHitResult ForwardHit;
    if (!GetWorld()->LineTraceSingleByChannel(ForwardHit, ForwardStart, ForwardEnd, ECC_WorldStatic, TraceParams))
        return false;
    
    // Downward trace from above hit point to find ledge top
    const FVector DownStart = ForwardHit.Location + FVector::UpVector * UpwardReachDistance;
    const FVector DownEnd = DownStart + FVector::DownVector * DownwardSearchDistance;
    
    FHitResult DownHit;
    if (!GetWorld()->LineTraceSingleByChannel(DownHit, DownStart, DownEnd, ECC_WorldStatic, TraceParams))
        return false;
    
    // Validate it's a proper ledge (horizontal surface)
    if (DownHit.Normal.Z < 0.7f) return false;
    
    // Check height requirement
    if ((DownHit.Location.Z - PlayerLocation.Z) < MinGrabHeight) return false;
    
    OutLocation = DownHit.Location;
    OutNormal = DownHit.Normal;
    return true;
}

bool ULedgeSwingComponent::DetectPole(FVector& OutLocation, FVector& OutNormal)
{
    const FVector PlayerLocation = OwnerCharacter->GetActorLocation();
    const FVector UpVector = FVector::UpVector;
    
    // Upward trace to detect overhead pole
    const FVector UpStart = PlayerLocation;
    const FVector UpEnd = UpStart + UpVector * UpwardReachDistance;
    
    FCollisionQueryParams TraceParams;
    TraceParams.AddIgnoredActor(OwnerCharacter);
    
    FHitResult UpHit;
    if (!GetWorld()->LineTraceSingleByChannel(UpHit, UpStart, UpEnd, ECC_WorldStatic, TraceParams))
        return false;
    
    // Check if it's a cylindrical surface (side hit, not top/bottom)
    if (FMath::Abs(UpHit.Normal.Z) > 0.3f) return false;
    
    // Check height requirement
    if ((UpHit.Location.Z - PlayerLocation.Z) < MinGrabHeight) return false;
    
    OutLocation = UpHit.Location;
    OutNormal = UpHit.Normal;
    return true;
}

void ULedgeSwingComponent::SwingJump()
{
    const float CurrentMomentum = CalculateSwingMomentum();
    const FVector JumpDirection = OwnerCharacter->GetActorForwardVector();
    const FVector JumpVelocity = JumpDirection * CurrentMomentum * SwingJumpMultiplier + FVector(0, 0, 400);
    
    ReleaseHang();
    OwnerCharacter->LaunchCharacter(JumpVelocity, false, true);
    
    LOG_INFO("Swing jump with momentum: %f", CurrentMomentum);
}

void ULedgeSwingComponent::MantleUp()
{
    const FVector MantleTarget = HangLocation + HangNormal * MantleForwardDistance + FVector(0, 0, MantleHeight);
    
    ReleaseHang();
    OwnerCharacter->LaunchCharacter((MantleTarget - OwnerCharacter->GetActorLocation()).GetSafeNormal() * 600, false, true);
    
    LOG_INFO("Mantling up to: %s", *MantleTarget.ToString());
}

float ULedgeSwingComponent::CalculateSwingMomentum() const
{
    return FMath::Abs(SwingVelocity) + (InitialMomentum * 0.5f);
}

void ULedgeSwingComponent::DebugDrawGrabAttempt()
{
    if (!OwnerCharacter) return;
    
    const FVector PlayerLocation = OwnerCharacter->GetActorLocation();
    const FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    
    // Draw detection ranges
    DrawDebugSphere(GetWorld(), PlayerLocation, ForwardReachDistance, 16, FColor::White, false, 1.0f);
    DrawDebugLine(GetWorld(), PlayerLocation, PlayerLocation + FVector::UpVector * UpwardReachDistance, FColor::Purple, false, 1.0f, 0, 3.0f);
    DrawDebugLine(GetWorld(), PlayerLocation, PlayerLocation + ForwardVector * ForwardReachDistance, FColor::Orange, false, 1.0f, 0, 3.0f);
}