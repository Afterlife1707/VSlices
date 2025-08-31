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

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
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

protected:
	virtual void BeginPlay() override;

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
};