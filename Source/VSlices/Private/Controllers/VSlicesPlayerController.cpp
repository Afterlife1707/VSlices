// Fill out your copyright notice in the Description page of Project Settings.

#include "Controllers/VSlicesPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Characters/VSlicesCharacter.h"
#include "LoggingMacros.h"

void AVSlicesPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
    
	PlayerCharacter = Cast<AVSlicesCharacter>(InPawn);
	if (!PlayerCharacter)
	{
		LOG_ERROR("PlayerCharacter is null!");
	}
}

void AVSlicesPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
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
		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AVSlicesPlayerController::Sprint);
		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &AVSlicesPlayerController::UnSprint);
		EnhancedInput->BindAction(GrappleAction, ETriggerEvent::Completed, this, &AVSlicesPlayerController::ShootGrapplingHook);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
	}
}

void AVSlicesPlayerController::Move(const FInputActionValue& Value)
{
	if (PlayerCharacter) PlayerCharacter->Move(Value);
}

void AVSlicesPlayerController::Look(const FInputActionValue& Value)
{
	if (PlayerCharacter) PlayerCharacter->Look(Value);
}

void AVSlicesPlayerController::JumpPressed()
{
	if (PlayerCharacter) PlayerCharacter->Jump();
}

void AVSlicesPlayerController::JumpReleased()
{
	if (PlayerCharacter) PlayerCharacter->StopJumping();
}

void AVSlicesPlayerController::Crouch()
{
	if (PlayerCharacter) PlayerCharacter->ToggleCrouch();
}

void AVSlicesPlayerController::Sprint()
{
	if (PlayerCharacter) PlayerCharacter->StartSprinting();
}

void AVSlicesPlayerController::UnSprint()
{
	if (PlayerCharacter) PlayerCharacter->StopSprinting();
}

void AVSlicesPlayerController::ShootGrapplingHook()
{
	if (PlayerCharacter) PlayerCharacter->ShootGrapplingHook();
}
