// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VSlicesCharacter.generated.h"

enum class EFoot : uint8;
class USprintComponent;
class USlideComponent;
class USlopeComponent;
class UVaultComponent;
class ULandingComponent;
class UWallRunComponent;
class UGrapplingHookComponent;
class ULedgeComponent;
class UCableComponent;
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="GrapplingGun", meta = (AllowPrivateAccess = "true"))
	UCableComponent* Cable;

	// Jump fields
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float JumpCooldownTime = 1.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float LaunchBoost = 600.f;

	// Speed fields
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
	UFUNCTION(BlueprintCallable, Category="Movement")
	void StartSlide() const;
	UFUNCTION(BlueprintCallable, Category="Movement")
	void StopSlide() const;

	// Sprint
	UFUNCTION(BlueprintCallable, Category="Movement")
	void StartSprinting() const;
	UFUNCTION(BlueprintCallable, Category="Movement")
	void StopSprinting() const;

	// Crouch
	UFUNCTION(BlueprintCallable, Category="Movement")
	void ToggleCrouch();
	UFUNCTION(BlueprintCallable, Category="Movement")
	void StartCrouch();
	UFUNCTION(BlueprintCallable, Category="Movement")
	void StopCrouch();

	// Jump
	virtual bool CanJumpInternal_Implementation() const override;
	virtual void Jump() override;
	void ResetJumpCooldown();
	void LaunchForward();

	// Grappling Hook
	UFUNCTION(BlueprintCallable, Category="Movement")
	void ShootGrapplingHook() const;

	// Footsteps
	void OnFootstep(const EFoot Foot);
	
	UFUNCTION(BlueprintCallable, Category="Movement|Validation")
	void WarnMissingComponent(TSubclassOf<UActorComponent> ComponentClass) const;
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved,
	                       FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) override;

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE UCableComponent* GetCable() const { return Cable; }

	UFUNCTION(BlueprintCallable, Category="Movement")
	FORCEINLINE void SetLedgeGrab(const bool bSet) { bLedgeGrab = bSet; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Movement")
	FORCEINLINE bool GetLedgeGrab() const { return bLedgeGrab; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Movement")
	bool GetIsSprinting() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Movement")
	bool GetIsSliding() const;

	FORCEINLINE float GetMaxJogSpeed() const { return MaxJogSpeed; }
	FORCEINLINE float GetMaxCrouchJogSpeed() const { return MaxCrouchJogSpeed; }
	FORCEINLINE float GetMaxSprintSpeed() const { return MaxSprintSpeed; }
	FORCEINLINE float GetMaxCrouchSprintSpeed() const { return MaxCrouchSprintSpeed; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Movement|Components")
	USprintComponent* GetSprintComponent() const { return SprintComponent; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Movement|Components")
	USlideComponent* GetSlideComponent() const { return SlideComponent; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Movement|Components")
	USlopeComponent* GetSlopeComponent() const { return SlopeComponent; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Movement|Components")
	UVaultComponent* GetVaultComponent() const { return VaultComponent; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Movement|Components")
	ULandingComponent* GetLandingComponent() const { return LandingComponent; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Movement|Components")
	UWallRunComponent* GetWallRunComponent() const { return WallRunComponent; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Movement|Components")
	UGrapplingHookComponent* GetGrapplingHookComponent() const { return GrapplingHookComponent; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Movement|Components")
	ULedgeComponent* GetLedgeComponent() const { return LedgeComponent; }

	struct FSlopeInfo GetSlopeInfo() const;

private:
	bool bLedgeGrab = false;
	bool bCanJump = true;
	bool bInCoyoteTime = true;
	float CoyoteTimeRemaining = 0.f;
	static float GetClampedRelativeYaw(const FRotator& ControlRot, const FRotator& BaseRot, const float YawInput, const float ClampAngle);
	
	UPROPERTY(EditDefaultsOnly, Category="Footsteps")
	class UFootstepData* FootstepData = nullptr;

	FTimerHandle JumpCooldownTimerHandle;
	mutable TSet<FString> AlreadyWarned;
	// Cached at BeginPlay - populated if the component was added in Blueprint
	UPROPERTY()
	USprintComponent* SprintComponent = nullptr;
	UPROPERTY()
	USlideComponent* SlideComponent = nullptr;
	UPROPERTY()
	USlopeComponent* SlopeComponent = nullptr;
	UPROPERTY()
	ULandingComponent* LandingComponent = nullptr;
	UPROPERTY()
	UVaultComponent* VaultComponent = nullptr;
	UPROPERTY()
	UWallRunComponent* WallRunComponent = nullptr;
	UPROPERTY()
	UGrapplingHookComponent* GrapplingHookComponent = nullptr;
	UPROPERTY()
	ULedgeComponent* LedgeComponent = nullptr;
};