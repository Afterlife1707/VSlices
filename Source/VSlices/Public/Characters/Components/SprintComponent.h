// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SprintComponent.generated.h"

class AVSlicesCharacter;
class ACharacter;
class UCharacterMovementComponent;
struct FSlopeInfo;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VSLICES_API USprintComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USprintComponent();

	// Sprint control functions
	UFUNCTION(BlueprintCallable, Category = "Sprint")
	void StartSprinting();
	UFUNCTION(BlueprintCallable, Category = "Sprint")
	void StopSprinting();
	UFUNCTION(BlueprintCallable, Category = "Sprint")
	bool CanSprint() const { return bCanSprint && !bSprintOnCooldown; }
	UFUNCTION(BlueprintCallable, Category = "Sprint")
	bool GetIsSprinting() const { return bIsSprinting; }
	UFUNCTION(BlueprintCallable, Category = "Sprint")
	float GetMaxJogSpeed() const { return MaxJogSpeed; }
	UFUNCTION(BlueprintCallable, Category = "Sprint")
	float GetMaxCrouchJogSpeed() const { return MaxCrouchJogSpeed; }

	// Called by character's move function to check sprint conditions
	void SprintCheck(float ForwardValue, float RightValue);

protected:
	virtual void BeginPlay() override;

	// Sprint settings
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sprint")
	float MaxSprintSpeed = 500.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sprint")
	float MaxJogSpeed = 400.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sprint")
	float MaxCrouchSprintSpeed = 500.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sprint")
	float MaxCrouchJogSpeed = 400.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
	float SprintCooldownDuration = 0.2f;

private:
	// Sprint state
	bool bIsSprinting = false;
	bool bCanSprint = false;
	UPROPERTY(BlueprintReadOnly, Category = "Sprint", meta = (AllowPrivateAccess = "true"))
	bool bSprintOnCooldown = false;
	
	// Timer handle for cooldown
	FTimerHandle SprintCooldownTimerHandle;
	
	// Cached components
	UPROPERTY()
	AVSlicesCharacter* OwnerCharacter = nullptr;
	UPROPERTY()
	UCharacterMovementComponent* MovementComponent = nullptr;

	// Internal functions
	void StartSprintCooldown();
	UFUNCTION()
	void EndSprintCooldown();
};