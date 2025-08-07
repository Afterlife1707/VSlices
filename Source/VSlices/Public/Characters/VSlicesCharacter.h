// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SlideComponent.h"
#include "Components/SprintComponent.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "VSlicesCharacter.generated.h"

class ULandingComponent;
class USpringArmComponent;
class UCameraComponent;
class USlopeComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AVSlicesCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	USprintComponent* SprintComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	USlideComponent* SlideComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	USlopeComponent* SlopeComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	ULandingComponent* LandingComponent;

public:
	AVSlicesCharacter();
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	// Slide - delegated to component
	void StartSlide() const;
	void StopSlide() const;

	// Crouch and jump
	void ToggleCrouch();
	void StartCrouch();
	void StopCrouch();
	virtual bool CanJumpInternal_Implementation() const override;
	virtual void Jump() override;
	void LaunchForward();

protected:
	virtual void Tick(float DeltaSeconds) override;

public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
	FORCEINLINE USprintComponent* GetSprintComponent() const { return SprintComponent; }
	FORCEINLINE USlideComponent* GetSlideComponent() const { return SlideComponent; }
	FORCEINLINE USlopeComponent* GetSlopeComponent() const { return SlopeComponent; }
	FORCEINLINE FSlopeInfo GetSlopeInfo() const { return SlopeComponent->GetSlopeInfo(); }
	FORCEINLINE float GetMaxJogSpeed() const { return SprintComponent->GetMaxJogSpeed(); }
	FORCEINLINE float GetMaxCrouchJogSpeed() const { return SprintComponent->GetMaxCrouchJogSpeed(); }
	UFUNCTION(BlueprintCallable, Category = Movement)
	FORCEINLINE bool GetIsSprinting() const { return SprintComponent ? SprintComponent->GetIsSprinting() : false; }
	UFUNCTION(BlueprintCallable, Category = Movement)
	FORCEINLINE bool GetIsSliding() const { return SlideComponent ? SlideComponent->IsSliding() : false; }

};