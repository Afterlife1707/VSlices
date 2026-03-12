// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ParkourComponentBase.h"
#include "Components/ActorComponent.h"
#include "SprintComponent.generated.h"

class AVSlicesCharacter;
class ACharacter;
class UCharacterMovementComponent;
struct FSlopeInfo;

UCLASS(ClassGroup=(Parkour), meta=(BlueprintSpawnableComponent))
class VSLICES_API USprintComponent : public UParkourComponentBase
{
	GENERATED_BODY()

public:
	USprintComponent();

	UFUNCTION(BlueprintCallable, Category = "Sprint")
	void StartSprinting();
	UFUNCTION(BlueprintCallable, Category = "Sprint")
	void StopSprinting();
	
	UFUNCTION(BlueprintCallable, Category = "Sprint")
	bool CanSprint() const { return bCanSprint && !bSprintOnCooldown; }
	UFUNCTION(BlueprintCallable, Category = "Sprint")
	bool GetIsSprinting() const { return bIsSprinting; }
	
	// Called by character's move function
	void SprintCheck(float ForwardValue, float RightValue);
	
	// Stamina
	UPROPERTY(EditDefaultsOnly, Category="Stamina")
	float MaxStamina = 100.f;
	UPROPERTY(EditDefaultsOnly, Category="Stamina")
	float StaminaDrainRate = 10.f; 
	UPROPERTY(EditDefaultsOnly, Category="Stamina")
	float StaminaRegenRate = 15.f; 
	UPROPERTY(EditDefaultsOnly, Category="Stamina")
	float StaminaRegenDelay = 2.f;
	UPROPERTY(EditDefaultsOnly, Category="Stamina")
	float StaminaRegenThreshold = 30.f; 
	UPROPERTY(EditDefaultsOnly, Category="Stamina")
	float StaminaBreathingThreshold = 70.f; 

	UPROPERTY(EditDefaultsOnly, Category="Audio")
	USoundBase* BreathingSound = nullptr;

	
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
	float SprintCooldownDuration = 0.2f;

private:
	bool bIsSprinting = false;
	bool bCanSprint = false;
	UPROPERTY(BlueprintReadOnly, Category = "Sprint", meta = (AllowPrivateAccess = "true"))
	bool bSprintOnCooldown = false;
	
	FTimerHandle SprintCooldownTimerHandle;

	void StartSprintCooldown();
	UFUNCTION()
	void EndSprintCooldown();
	
	float CurrentStamina = 100.f;
	float RegenDelayRemaining = 0.f;
	bool bExhausted = false;
	UPROPERTY()
	UAudioComponent* BreathingAudioComponent = nullptr;
	void UpdateBreathingAudio() const;
	void UpdateStamina(float DeltaTime);
};