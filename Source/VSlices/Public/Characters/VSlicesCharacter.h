// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SlideComponent.h"
#include "Components/SprintComponent.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "VSlicesCharacter.generated.h"

struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AVSlicesCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class USprintComponent* SprintComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class USlideComponent* SlideComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class USlopeComponent* SlopeComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class ULandingComponent* LandingComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class UVaultComponent* VaultComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float JumpCooldownTime = 1.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float LaunchBoost = 600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement Speed", meta = (AllowPrivateAccess = "true"))
	float MaxJogSpeed = 600.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement Speed", meta = (AllowPrivateAccess = "true"))
	float MaxCrouchJogSpeed = 300.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement Speed", meta = (AllowPrivateAccess = "true"))
	float MaxSprintSpeed = 1200.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement Speed", meta = (AllowPrivateAccess = "true"))
	float MaxCrouchSprintSpeed = 600.0f;
	
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
	void ResetJumpCooldown();
	void LaunchForward();

protected:
	virtual void Tick(float DeltaSeconds) override;

public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
	FORCEINLINE USprintComponent* GetSprintComponent() const { return SprintComponent; }
	FORCEINLINE USlideComponent* GetSlideComponent() const { return SlideComponent; }
	FORCEINLINE USlopeComponent* GetSlopeComponent() const { return SlopeComponent; }
	FORCEINLINE UVaultComponent* GetVaultComponent() const { return VaultComponent; }
	
	FORCEINLINE FSlopeInfo GetSlopeInfo() const { return SlopeComponent->GetSlopeInfo(); }
	
	FORCEINLINE float GetMaxJogSpeed() const { return MaxJogSpeed; }
	FORCEINLINE float GetMaxCrouchJogSpeed() const { return MaxCrouchJogSpeed; }
	FORCEINLINE float GetMaxSprintSpeed() const { return MaxSprintSpeed; }
	FORCEINLINE float GetMaxCrouchSprintSpeed() const { return MaxCrouchSprintSpeed; }
	
	UFUNCTION(BlueprintCallable, Category = Movement)
	FORCEINLINE bool GetIsSprinting() const { return SprintComponent ? true : false; }
	UFUNCTION(BlueprintCallable, Category = Movement)
	FORCEINLINE bool GetIsSliding() const { return SlideComponent ? SlideComponent->IsSliding() : false; }

private:
	bool bCanJump = true;
	FTimerHandle JumpCooldownTimerHandle;

};