// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/VSlicesPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"

void AVSlicesPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
	PlayerCharacter = Cast<AVSlicesCharacter>(GetCharacter());
	if(PlayerCharacter)
	{
		PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = MaxJogSpeed;
		PlayerCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched = MaxCrouchJogSpeed;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerCharacter is null!"));
	}
}

void AVSlicesPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AVSlicesPlayerController::Move);
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AVSlicesPlayerController::Look);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &AVSlicesPlayerController::JumpPressed);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &AVSlicesPlayerController::JumpReleased);
		EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Started, this, &AVSlicesPlayerController::Crouch);
		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &AVSlicesPlayerController::Sprint);
		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &AVSlicesPlayerController::UnSprint);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
	}
}

void AVSlicesPlayerController::Move(const FInputActionValue& Value)
{
	PlayerCharacter->Move(Value);
}

void AVSlicesPlayerController::Look(const FInputActionValue& Value)
{
	PlayerCharacter->Look(Value);
}

void AVSlicesPlayerController::JumpPressed()
{
	PlayerCharacter->Jump();
}

void AVSlicesPlayerController::JumpReleased()
{
	PlayerCharacter->StopJumping();
}

void AVSlicesPlayerController::Crouch()
{
	PlayerCharacter->Crouching();
}

void AVSlicesPlayerController::Sprint()
{
	if(!PlayerCharacter->bIsCrouched)
		PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = MaxSprintSpeed;
	else
		PlayerCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched = MaxCrouchSprintSpeed;
}

void AVSlicesPlayerController::UnSprint()
{
	if(!PlayerCharacter->bIsCrouched)
		PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = MaxJogSpeed;
	else 
		PlayerCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched = MaxCrouchJogSpeed;
}
