// Copyright Epic Games, Inc. All Rights Reserved.

#include "Characters/VSlicesCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "InputActionValue.h"
#include "LoggingMacros.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AVSlicesCharacter

AVSlicesCharacter::AVSlicesCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	
	bUseControllerRotationYaw = true;
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed = MaxJogSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = MaxCrouchJogSpeed;
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh(), TEXT("neck_02"));
	CameraBoom->TargetArmLength = 0.f; 
	CameraBoom->bUsePawnControlRotation = true; 

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); 
	FollowCamera->bUsePawnControlRotation = false; 
}

void AVSlicesCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (bIsSliding)
	{
		HandleSlideTick(DeltaSeconds);
	}
}

void AVSlicesCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		
		const FSlopeInfo& SlopeInfo = GetSlopeInfo();
		ApplySlopeRestrictions(MovementVector, SlopeInfo);
		if(!bIsCrouched || !bIsSliding)
		{
			SprintCheck(MovementVector.Y, MovementVector.X, SlopeInfo);
		}
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AVSlicesCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

#pragma region JUMP
bool AVSlicesCharacter::CanJumpInternal_Implementation() const 
{
	return Super::CanJumpInternal_Implementation() || bIsCrouched;
}

void AVSlicesCharacter::Jump() 
{
	Super::Jump();
	if(bIsSprinting)
	{
		FTimerHandle JumpTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
				JumpTimerHandle, 
				this, 
				&AVSlicesCharacter::LaunchForward, 
				0.1f, 
				false
			);
	}
	if(bIsCrouched)
		UnCrouch();
}

void AVSlicesCharacter::LaunchForward()
{
	UE_LOG(LogTemp, Warning, TEXT("Launched"));
	FVector LaunchDir = GetActorForwardVector() * SlideBoost;
	LaunchCharacter(LaunchDir, true, true);
}
#pragma endregion JUMP

#pragma region SPRINT

void AVSlicesCharacter::StartSprinting()
{
	if(!bCanSprint || bSprintOnCooldown) return;
	bIsSprinting = true;
	if(!bIsCrouched)
	{
		GetCharacterMovement()->MaxWalkSpeed = MaxSprintSpeed;
	}
	else
		GetCharacterMovement()->MaxWalkSpeedCrouched = MaxCrouchSprintSpeed;
}

void AVSlicesCharacter::StopSprinting()
{
	bIsSprinting = false;
	if(!bIsCrouched)
	{
		GetCharacterMovement()->MaxWalkSpeed = MaxJogSpeed;
	}
	else 
		GetCharacterMovement()->MaxWalkSpeedCrouched = MaxCrouchJogSpeed;
}

void AVSlicesCharacter::SprintCheck(float ForwardValue, float RightValue, const FSlopeInfo& SlopeInfo)
{
	const bool bMovingForward = ForwardValue > 0.001f;
	const bool bMovingBackward = ForwardValue < -0.001f;
	const bool bMovingSideways = FMath::Abs(RightValue) > 0.001f;
	
	bCanSprint = !bMovingSideways && !bMovingBackward && bMovingForward && !bSprintOnCooldown;
	if (bIsSprinting && (!bCanSprint || bSprintOnCooldown))
	{
		StopSprinting();
		StartSprintCooldown();
	}
}
void AVSlicesCharacter::StartSprintCooldown()
{
	if (SprintCooldownDuration > 0.0f)
	{
		bSprintOnCooldown = true;
		GetWorld()->GetTimerManager().SetTimer(
			SprintCooldownTimerHandle, 
			this, 
			&AVSlicesCharacter::EndSprintCooldown, 
			SprintCooldownDuration, 
			false
		);
	}
}

void AVSlicesCharacter::EndSprintCooldown() 
{
	bSprintOnCooldown = false;
	GetWorld()->GetTimerManager().ClearTimer(SprintCooldownTimerHandle);
}

#pragma endregion SPRINT

#pragma region SLIDE

void AVSlicesCharacter::StartSlide()
{
	if (bIsSliding || !GetCharacterMovement()->IsMovingOnGround())
		return;

	bIsSliding = true;
	SlideElapsed = 0.f;
	ActualSlideDuration = SlideDuration;

	GetCharacterMovement()->MaxWalkSpeedCrouched = SlideSpeed;
	LaunchForward();
	Crouch();
}

void AVSlicesCharacter::HandleSlideTick(float DeltaSeconds)
{
	SlideElapsed += DeltaSeconds;

	const float CurrentSpeed = GetVelocity().Size();
	if (CurrentSpeed < MinSlideSpeed || SlideElapsed >= ActualSlideDuration)
	{
		StopSlide();
		return;
	}

	const FSlopeInfo& SlopeInfo = GetSlopeInfo();
	
	if (!SlopeInfo.bIsOnSlope) return;
	if (SlopeInfo.bIsUphill && SlideElapsed > SlideDuration * UphillDurationMultiplier)
		StopSlide();
	else if (SlopeInfo.bIsDownhill && CurrentSpeed > DownhillSpeedThreshold)
		ActualSlideDuration = FMath::Min(ActualSlideDuration + SlideExtensionPerTick, MaxSlideDuration);
}

void AVSlicesCharacter::StopSlide()
{
	if (!bIsSliding)
		return;

	bIsSliding = false;
	SlideElapsed = 0.f;
	ActualSlideDuration = 0.f;

	GetCharacterMovement()->MaxWalkSpeedCrouched = MaxCrouchJogSpeed;
}
#pragma endregion SLIDE

#pragma region CROUCH
void AVSlicesCharacter::ToggleCrouch()
{
	if (bIsCrouched)
		StopCrouch();
	else
		StartCrouch();
}

void AVSlicesCharacter::StartCrouch()
{
	Crouch();
	if (bIsSprinting)
		StartSlide(); 
}

void AVSlicesCharacter::StopCrouch()
{
	UnCrouch();
}
#pragma endregion CROUCH

#pragma region SLOPE

void AVSlicesCharacter::ApplySlopeRestrictions(FVector2D& MovementVector, const FSlopeInfo& SlopeInfo) const
{
	if (!SlopeInfo.bIsOnSlope) return;
    
	const bool bMovingForward = MovementVector.Y > 0.001f;
	const bool bMovingBackward = MovementVector.Y < -0.001f;
    
	// Restrict uphill movement on steep slopes
	if (bMovingForward && SlopeInfo.bIsUphill)
	{
		if(SlopeInfo.SlopeAngle > MinSlopeSpeedDecreaseAngle)
		{
			if(bIsSprinting) MovementVector.Y *= 0.75f;
			else MovementVector.Y *= 0.5f;
		}
		else if(SlopeInfo.SlopeAngle > MaxWalkableUphillAngle)
		{
			MovementVector.Y = 0.0f; 
			LOG_WARNING("Cannot walk up steep slope: %.1f degrees", SlopeInfo.SlopeAngle);
		}
	}
	if (bMovingBackward && SlopeInfo.bIsDownhill && SlopeInfo.SlopeAngle > MaxWalkableDownhillAngle)
	{
		MovementVector.Y = 0.0f; // Block backward movement
		LOG_WARNING("Cannot walk up steep downhill slope: %.1f degrees", SlopeInfo.SlopeAngle);
	}
}

FSlopeInfo AVSlicesCharacter::GetSlopeInfo() 
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastSlopeUpdateTime >= SlopeUpdateInterval)
	{
		UpdateSlopeInfo();
		LastSlopeUpdateTime = CurrentTime;
	}
	return CachedSlopeInfo;
}

void AVSlicesCharacter::UpdateSlopeInfo()
{
	CachedSlopeInfo = FSlopeInfo();
    
	const FHitResult& FloorHit = GetCharacterMovement()->CurrentFloor.HitResult;
	if (!FloorHit.IsValidBlockingHit()) 
		return;
    
	// slope angle
	const FVector FloorNormal = FloorHit.ImpactNormal;
	const float FloorDotUp = FVector::DotProduct(FloorNormal, FVector::UpVector);
	CachedSlopeInfo.SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FloorDotUp, 0.0f, 1.0f)));
    
	if (CachedSlopeInfo.SlopeAngle <= MinSlopeAngle) 
		return;
    
	CachedSlopeInfo.bIsOnSlope = true;
    
	// slope direction and character alignment
	const FVector SlopeDirection = FVector::CrossProduct(FloorNormal, 
		FVector::CrossProduct(FloorNormal, FVector::UpVector)).GetSafeNormal();
	CachedSlopeInfo.FacingAlignment = FVector::DotProduct(GetActorForwardVector(), SlopeDirection);
    
	// uphill/downhill
	CachedSlopeInfo.bIsUphill = CachedSlopeInfo.FacingAlignment < -UphillThreshold;
	CachedSlopeInfo.bIsDownhill = CachedSlopeInfo.FacingAlignment > DownhillThreshold;
}

void AVSlicesCharacter::InvalidateSlopeCache() //use when needed
{
	LastSlopeUpdateTime = 0.0f;
}

#pragma endregion SLOPE