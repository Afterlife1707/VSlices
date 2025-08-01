// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "VSlicesCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
struct FInputActionValue;

USTRUCT(BlueprintType)
struct FSlopeInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bIsOnSlope = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsUphill = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsDownhill = false;

	UPROPERTY(BlueprintReadOnly)
	float SlopeAngle = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float FacingAlignment = 0.0f; // -1 = uphill, +1 = downhill, 0 = sideways
};

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AVSlicesCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

public:
	AVSlicesCharacter();
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	//sprint
	void StartSprinting();
	void StopSprinting();
	void StartSprintCooldown();
	UFUNCTION()
	void EndSprintCooldown();

	//slide
	void StartSlide();
	void StopSlide();
	void HandleSlideTick(float DeltaSeconds);

	//crouch and jump
	void ToggleCrouch();
	void StartCrouch();
	void StopCrouch();
	virtual bool CanJumpInternal_Implementation() const override;
	virtual void Jump() override;
	void LaunchForward();

	//slope
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Slope", meta = (ClampMin = "25.0", ClampMax = "50.0"))
	float MinSlopeSpeedDecreaseAngle = 25.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Slope", meta = (ClampMin = "25.0", ClampMax = "50.0"))
	float MaxWalkableUphillAngle = 35.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Slope", meta = (ClampMin = "25.0", ClampMax = "50.0"))
	float MaxWalkableDownhillAngle = 40.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Slope", meta = (ClampMin = "30.0", ClampMax = "60.0"))
	float MaxSlidableDownhillAngle = 50.0f;
	
protected:
	virtual void Tick(float DeltaSeconds) override;
	
protected:
	//speed variables
	
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

	//sprint
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float SprintCooldownDuration = 0.2f;
	UPROPERTY(BlueprintReadOnly, Category = "Input")
	bool bSprintOnCooldown;

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	UFUNCTION(BlueprintCallable, Category = Movement)
	FORCEINLINE bool GetIsSprinting() const { return bIsSprinting; }
	UFUNCTION(BlueprintCallable, Category = Movement)
	FORCEINLINE bool GetIsSliding() const { return bIsSliding; }

private:
	//sprint
	bool bIsSprinting;
	bool bCanSprint;
	FTimerHandle SprintCooldownTimerHandle;
	void SprintCheck(float ForwardValue, float RightValue, const FSlopeInfo& SlopeInfo);
	
	//slide
	bool bIsSliding;
	float SlideElapsed = 0.0f;
	float ActualSlideDuration = 0.0f;
	FTimerHandle SlideTimerHandle;

	//slope
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Slope", meta = (ClampMin = "0.01", ClampMax = "0.1"), meta=(AllowPrivateAccess))
	float SlopeUpdateInterval = 0.1f; 
	FSlopeInfo CachedSlopeInfo;
	float LastSlopeUpdateTime = 0.0f;
	FSlopeInfo GetSlopeInfo();
	void UpdateSlopeInfo();
	void InvalidateSlopeCache();
	void ApplySlopeRestrictions(FVector2D& MovementVector, const FSlopeInfo& SlopeInfo) const;
};

