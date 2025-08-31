// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ParkourComponentBase.h"
#include "GrapplingHookComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VSLICES_API UGrapplingHookComponent : public UParkourComponentBase
{
	GENERATED_BODY()

public:	
	UGrapplingHookComponent();
	void TryShoot();
	void ReleaseGrapple();

	FORCEINLINE bool GetIsGrappling () const {return bIsGrappling;}
	bool ShouldBoost () const;
protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GrapplingGun", meta = (AllowPrivateAccess = "true"))
	float Range = 3000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GrapplingGun", meta = (AllowPrivateAccess = "true"))
	float GrappleSpeed = 800.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GrapplingGun", meta = (AllowPrivateAccess = "true"))
	float SwingForce = 1200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GrapplingGun", meta = (AllowPrivateAccess = "true"))
	float CableInterpSpeed = 15.0f;
	
	bool bIsGrappling;
	bool bShouldBoost;
	FVector GrappleLocation;
	
	void StartGrapple(const FVector& TargetLocation);
	void UpdateGrappleMovement(float const DeltaTime);
	void UpdateCableVisuals(float const DeltaTime) const;
};
