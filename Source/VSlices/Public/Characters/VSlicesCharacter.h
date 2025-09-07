// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class UWallRunComponent* WallRunComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class UGrapplingHookComponent* GrapplingHookComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="GrapplingGun", meta = (AllowPrivateAccess = "true"))
	class UCableComponent* Cable;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class ULedgeSwingComponent* LedgeSwingComponent;
	
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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float CoyoteTimeDuration = 0.15f;
public:
	AVSlicesCharacter();
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	// Slide
	void StartSlide() const;
	void StopSlide() const;
	//sprint
	void StartSprinting() const;
	void StopSprinting() const;
	// Crouch and jump
	void ToggleCrouch();
	void StartCrouch();
	void StopCrouch();
	virtual bool CanJumpInternal_Implementation() const override;
	virtual void Jump() override;
	void ResetJumpCooldown();
	void LaunchForward();
	//Grappling Hook
	void ShootGrapplingHook() const;

protected:
	virtual void Tick(float DeltaSeconds) override;
	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved,
	                       FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) override;
public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
	USprintComponent* GetSprintComponent() const;
	USlideComponent* GetSlideComponent() const;
	USlopeComponent* GetSlopeComponent() const;
	UVaultComponent* GetVaultComponent() const;
	ULandingComponent* GetLandingComponent() const;
	UWallRunComponent* GetWallRunComponent() const;

	struct FSlopeInfo GetSlopeInfo() const;

	FORCEINLINE float GetMaxJogSpeed() const { return MaxJogSpeed; }
	FORCEINLINE float GetMaxCrouchJogSpeed() const { return MaxCrouchJogSpeed; }
	FORCEINLINE float GetMaxSprintSpeed() const { return MaxSprintSpeed; }
	FORCEINLINE float GetMaxCrouchSprintSpeed() const { return MaxCrouchSprintSpeed; }
	
	FORCEINLINE UCableComponent* GetCable() const { return Cable; }
	
	UFUNCTION(BlueprintCallable, Category = Movement)
	bool GetIsSprinting() const;
	UFUNCTION(BlueprintCallable, Category = Movement)
	bool GetIsSliding() const;

private:
	bool bCanJump = true;
	bool bInCoyoteTime = true;
    float CoyoteTimeRemaining;
	
	FTimerHandle JumpCooldownTimerHandle;
	//FTimerHandle LedgeDetectionTimerHandle;
};