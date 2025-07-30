// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "VSlicesCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AVSlicesCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

public:
	AVSlicesCharacter();
	
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	
	void StartSprinting();
	void StopSprinting();
	
	void StartSlide();
	void StopSlide();
	
	void ToggleCrouch();
	void StartCrouch();
	void StopCrouch();
	virtual bool CanJumpInternal_Implementation() const override;
	virtual void Jump() override;
	void LaunchForward();

protected:
	UPROPERTY(EditDefaultsOnly, Category=Input)
	float MaxSprintSpeed = 500.f;
	UPROPERTY(EditDefaultsOnly, Category=Input)
	float MaxJogSpeed = 400.f;
	UPROPERTY(EditDefaultsOnly, Category=Input)
	float MaxCrouchSprintSpeed = 500.f;
	UPROPERTY(EditDefaultsOnly, Category=Input)
	float MaxCrouchJogSpeed = 400.f;

	UPROPERTY(EditDefaultsOnly, Category = Slide)
	float SlideDuration = 0.75f;
	UPROPERTY(EditDefaultsOnly, Category = Slide)
	float SlideSpeed = 1000.f;
	UPROPERTY(EditDefaultsOnly, Category = Slide)
	float SlideBoost = 1000.f;
	
public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

private:
	bool bIsSprinting;
	
	bool bIsSliding;
	FTimerHandle SlideTimerHandle;
};

