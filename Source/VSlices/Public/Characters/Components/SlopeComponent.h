// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ParkourComponentBase.h"
#include "Components/ActorComponent.h"
#include "SlopeComponent.generated.h"

class AVSlicesCharacter;
class ACharacter;
class UCharacterMovementComponent;

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

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VSLICES_API USlopeComponent : public UParkourComponentBase
{
	GENERATED_BODY()

public:
	USlopeComponent();

	UFUNCTION(BlueprintCallable, Category = "Slope")
	FSlopeInfo GetSlopeInfo();
	UFUNCTION(BlueprintCallable, Category = "Slope")
	void ApplySlopeRestrictions(FVector2D& MovementVector);

protected:
	// Slope settings
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Slope", meta = (ClampMin = "25.0", ClampMax = "50.0"))
	float MinSlopeSpeedDecreaseAngle = 25.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Slope", meta = (ClampMin = "25.0", ClampMax = "50.0"))
	float MaxWalkableUphillAngle = 35.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Slope", meta = (ClampMin = "25.0", ClampMax = "50.0"))
	float MaxWalkableDownhillAngle = 40.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Slope", meta = (ClampMin = "30.0", ClampMax = "60.0"))
	float MaxSlidableDownhillAngle = 50.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope Settings")
	float UphillThreshold = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope") 
	float DownhillThreshold = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope")
	float MinSlopeAngle = 5.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Slope", meta = (ClampMin = "0.01", ClampMax = "0.1"))
	float SlopeUpdateInterval = 0.1f;

private:
	FSlopeInfo CachedSlopeInfo;
	float LastSlopeUpdateTime = 0.0f;
	
	void UpdateSlopeInfo();
};