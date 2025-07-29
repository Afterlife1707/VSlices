// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/VSlicesPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

void AVSlicesPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
	Char = Cast<AVSlicesCharacter>(GetCharacter());
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
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
	}
}

void AVSlicesPlayerController::Move(const FInputActionValue& Value)
{
	Char->Move(Value);
}

void AVSlicesPlayerController::Look(const FInputActionValue& Value)
{
	Char->Look(Value);
}

void AVSlicesPlayerController::JumpPressed()
{
	Char->Jump();
}

void AVSlicesPlayerController::JumpReleased()
{
	Char->StopJumping();
}

void AVSlicesPlayerController::Crouch()
{
	Char->Crouching();
}
