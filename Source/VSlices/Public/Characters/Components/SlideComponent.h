// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SlopeComponent.h" // For FSlopeInfo
#include "SlideComponent.generated.h"

class AVSlicesCharacter;
class ACharacter;
class UCharacterMovementComponent;
class USlopeComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VSLICES_API USlideComponent : public UParkourComponentBase
{
	GENERATED_BODY()

public:
	USlideComponent();

	UFUNCTION(BlueprintCallable, Category = "Slide")
	void StartSlide();
	
	UFUNCTION(BlueprintCallable, Category = "Slide")
	void StopSlide();
	
	UFUNCTION(BlueprintCallable, Category = "Slide")
	bool IsSliding() const { return bIsSliding; }
	
	UFUNCTION(BlueprintCallable, Category = "Slide")
	void HandleSlideTick(float DeltaSeconds);

protected:
	// Slide settings
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Slide")
	float SlideDuration = 0.75f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Slide")
	float SlideSpeed = 1000.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Slide")
	float SlideBoost = 1000.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Slide")
	float MaxSlideDuration = 3.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Slide")
	float SlideExtensionPerTick = 0.05f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slide")
	float UphillDurationMultiplier = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slide")
	float DownhillSpeedThreshold = 300.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slide")  
	float MinSlideSpeed = 100.0f;

private:
	bool bIsSliding = false;
	float SlideElapsed = 0.0f;
	float ActualSlideDuration = 0.0f;
	FTimerHandle SlideTimerHandle;
	
	// Cached speed for restoration
	float CachedCrouchSpeed = 0.0f;
};