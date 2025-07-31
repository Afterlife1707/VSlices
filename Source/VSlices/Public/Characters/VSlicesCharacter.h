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
	void HandleSlideTick(float DeltaSeconds);
	
	void ToggleCrouch();
	void StartCrouch();
	void StopCrouch();
	virtual bool CanJumpInternal_Implementation() const override;
	virtual void Jump() override;
	void LaunchForward();
	
	void StartSprintCooldown();
	UFUNCTION()
	void EndSprintCooldown();
protected:
	virtual void Tick(float DeltaSeconds) override;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="Input")
	float MaxSprintSpeed = 500.f;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	float MaxJogSpeed = 400.f;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	float MaxCrouchSprintSpeed = 500.f;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	float MaxCrouchJogSpeed = 400.f;

	//Slide
	
	UPROPERTY(EditDefaultsOnly, Category = "Slide")
	float SlideDuration = 0.75f;
	UPROPERTY(EditDefaultsOnly, Category = "Slide")
	float SlideSpeed = 1000.f;
	UPROPERTY(EditDefaultsOnly, Category = "Slide")
	float SlideBoost = 1000.f;
	UPROPERTY(EditDefaultsOnly, Category = "Slide")
	float MaxSlideDuration = 3.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Slide")
	float SlideExtensionPerTick = 0.05f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slide")
	float UphillThreshold = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slide") 
	float DownhillThreshold = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slide")
	float MinSlopeAngle = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slide")
	float UphillDurationMultiplier = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slide")
	float DownhillSpeedThreshold = 300.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slide")  
	float MinSlideSpeed = 100.0f;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float SprintCooldownDuration = 0.2f;

	UPROPERTY(BlueprintReadOnly, Category = "Input")
	bool bSprintOnCooldown;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	//UPROPERTY(BlueprintCallable, Category=Movement)
	UFUNCTION(BlueprintCallable, Category = Movement)
	FORCEINLINE bool GetIsSprinting() const { return bIsSprinting; }
	UFUNCTION(BlueprintCallable, Category = Movement)
	FORCEINLINE bool GetIsSliding() const { return bIsSliding; }

private:
	bool bIsSprinting;
	bool bCanSprint;
	bool bIsSliding;
	float SlideElapsed = 0.0f;
	float ActualSlideDuration = 0.0f;
	FTimerHandle SlideTimerHandle;

	FTimerHandle SprintCooldownTimerHandle;
	void SprintCheck(float ForwardValue, float RightValue);
};

