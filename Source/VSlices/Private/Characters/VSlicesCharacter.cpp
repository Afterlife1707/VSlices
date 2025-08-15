//Copyright Epic Games, Inc. All Rights Reserved.

#include "Characters/VSlicesCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "InputActionValue.h"
#include "LoggingMacros.h"
#include "Characters/Components/LandingComponent.h"
#include "Characters/Components/SprintComponent.h"
#include "Characters/Components/SlideComponent.h"
#include "Characters/Components/SlopeComponent.h"
#include "Characters/Components/VaultComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AVSlicesCharacter

AVSlicesCharacter::AVSlicesCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	
	bUseControllerRotationYaw = true;
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh(), TEXT("neck_02"));
	CameraBoom->TargetArmLength = 0.f; 
	CameraBoom->bUsePawnControlRotation = true; 

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); 
	FollowCamera->bUsePawnControlRotation = false; 
	
	// Create movement components
	SprintComponent = CreateDefaultSubobject<USprintComponent>(TEXT("Sprint"));
	SlideComponent = CreateDefaultSubobject<USlideComponent>(TEXT("Slide"));
	SlopeComponent = CreateDefaultSubobject<USlopeComponent>(TEXT("Slope"));
	LandingComponent = CreateDefaultSubobject<ULandingComponent>(TEXT("Landing"));
	VaultComponent = CreateDefaultSubobject<UVaultComponent>(TEXT("Vault"));
}

void AVSlicesCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (SlideComponent)
		SlideComponent->HandleSlideTick(DeltaSeconds);
	
	if (LandingComponent)
		LandingComponent->HandleFallDetection();
}

void AVSlicesCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller && MovementVector.SizeSquared() > 0.0f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        
		if (SlopeComponent)
		{
			SlopeComponent->ApplySlopeRestrictions(MovementVector);
		}
        
		if (!bIsCrouched && !GetIsSliding() && SprintComponent)
		{
			SprintComponent->SprintCheck(MovementVector.Y, MovementVector.X);
		}
        
		// Apply movement input
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
	if (VaultComponent && !VaultComponent->IsVaulting() && VaultComponent->TryVault())
		return;
	if (!bCanJump)
		return;
	Super::Jump();
	if (GetIsSprinting())
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
	bCanJump = false;
	if(GetVelocity().Length()<150.f) return;
	
	float CurrentJumpCooldown = JumpCooldownTime;
	if(GetIsSprinting()) CurrentJumpCooldown *= 1.5f;
	GetWorldTimerManager().SetTimer(
		JumpCooldownTimerHandle,
		this,
		&AVSlicesCharacter::ResetJumpCooldown,
		CurrentJumpCooldown,
		false
	);
}

void AVSlicesCharacter::ResetJumpCooldown()
{
	bCanJump = true;
}

void AVSlicesCharacter::LaunchForward()
{
	const float CurrentSpeed = GetVelocity().Length();
	const float SpeedRatio = FMath::Clamp(CurrentSpeed / GetMaxSprintSpeed(), 0.0f, 2.0f); // Cap at 200%
    
	const float SpeedMultiplier = 1.0f + (SpeedRatio * 0.5f); // 1.0 to 2.0 multiplier
    
	const FVector LaunchDir = GetActorForwardVector() * LaunchBoost * SpeedMultiplier;
	LaunchCharacter(LaunchDir, true, false);
}
#pragma endregion JUMP

void AVSlicesCharacter::StartSlide() const
{
	SlideComponent->StartSlide();
}

void AVSlicesCharacter::StopSlide() const
{
	SlideComponent->StopSlide();
}

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
	if (GetIsSprinting())
		StartSlide(); 
}

void AVSlicesCharacter::StopCrouch()
{
	UnCrouch();
}