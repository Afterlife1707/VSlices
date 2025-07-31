// Copyright Epic Games, Inc. All Rights Reserved.

#include "Characters/VSlicesCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "InputActionValue.h"

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
		
		if(!bIsCrouched || !bIsSliding)
		{
			SprintCheck(MovementVector.Y, MovementVector.X);
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

void AVSlicesCharacter::StartSlide()
{
	if (bIsSliding || !GetCharacterMovement()->IsMovingOnGround())
		return;
	bIsSliding = true;
	
	GetCharacterMovement()->MaxWalkSpeedCrouched = SlideSpeed;
	LaunchForward();
	
	GetWorldTimerManager().SetTimer(SlideTimerHandle, this, &AVSlicesCharacter::StopSlide, SlideDuration, false);
}

void AVSlicesCharacter::StopSlide()
{
	bIsSliding = false;
	GetCharacterMovement()->MaxWalkSpeedCrouched = MaxCrouchJogSpeed;
}

void AVSlicesCharacter::ToggleCrouch()
{
	if (bIsCrouched)
	{
		StopCrouch();
	}
	else
	{
		StartCrouch();
	}
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

bool AVSlicesCharacter::CanJumpInternal_Implementation() const 
{
	return Super::CanJumpInternal_Implementation() || bIsCrouched;
}

void AVSlicesCharacter::Jump() 
{
	Super::Jump();
	if(bIsCrouched)
		UnCrouch();
}

void AVSlicesCharacter::LaunchForward()
{
	FVector LaunchDir = GetActorForwardVector() * SlideBoost;
	LaunchCharacter(LaunchDir, true, true);
}

void AVSlicesCharacter::SprintCheck(float ForwardValue, float RightValue )
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
			SprintCooldownTimer, 
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
	GetWorld()->GetTimerManager().ClearTimer(SprintCooldownTimer);
}