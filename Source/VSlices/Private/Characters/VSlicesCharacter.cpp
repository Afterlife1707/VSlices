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

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
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
	if(!bIsCrouched)
	{
		bIsSprinting = true;
		GetCharacterMovement()->MaxWalkSpeed = MaxSprintSpeed;
	}
	else
		GetCharacterMovement()->MaxWalkSpeedCrouched = MaxCrouchSprintSpeed;
}

void AVSlicesCharacter::StopSprinting()
{
	if(!bIsCrouched)
	{
		bIsSprinting = false;
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

bool AVSlicesCharacter::CanJumpInternal_Implementation() const override
{
	return Super::CanJumpInternal_Implementation() || bIsCrouched;
}

void AVSlicesCharacter::Jump() override
{
	Super::Jump();
	if(bIsCrouched)
		UnCrouch();
	// if (bIsSliding) 
	// 	LaunchForward(); //NOT WORKING
}

void AVSlicesCharacter::LaunchForward()
{
	UE_LOG(LogTemp, Warning, TEXT("Launched!"));
	FVector LaunchDir = GetActorForwardVector() * SlideBoost;
	LaunchCharacter(LaunchDir, true, true);
}
